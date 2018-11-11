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
	testmode_ioctl.c
*/
#if defined(COMPOS_WIN)
#include "MtConfig.h"
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#include "hdev/hdev.h"
#endif
static INT EthGetParamAndShiftBuff(BOOLEAN convert, UINT size, UCHAR **buf, 
									   IN UCHAR *out)
{
	if (!(*buf)) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("*buf NULL pointer with size:%u\n", size));
		return -1;
	}

	if (!out) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("out NULL pointer with size:%u\n", size));
		return -1;
	}

	NdisMoveMemory(out, *buf, size);
	*buf = *buf + size;
	if (!convert) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s::size %u, ",__FUNCTION__, size));
		return 0;
	}

	if(size == sizeof(UINT32)) {
		UINT32 *tmp = (UINT32 *)out;
		*tmp = PKTL_TRAN_TO_HOST(*tmp);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s::size %u, val: %u\n",__FUNCTION__, size, *tmp));
	}else if(size == sizeof(UINT16)) {
		UINT16 *tmp = (UINT16 *)out;
		*tmp = PKTS_TRAN_TO_HOST(*tmp);
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
			("%s::size %u, val: %u\n",__FUNCTION__, size, *tmp));
	}else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s::size %u not supported\n",__FUNCTION__, size));
		return -1;
	}
	return 0;
}

// to seperate Windows ndis and Linux/Windows WDM
#if (defined(SUPPORT_QA_TOOL)&&(SUPPORT_QA_TOOL ==1)) || !defined(COMPOS_WIN)
//1to do removed later
//to seperate Windows ndis/WDM and Linux
#if defined(COMPOS_TESTMODE_WIN) || defined(COMPOS_WIN)
UINT32 mt_dft_mac_cr_range[] = {
	0x50022000, 0x50022000, 0xc84,	 /* USB Controller */
	0x50029000, 0x50029000, 0x210,	 /* USB DMA */
	0x800c006c, 0x800c006c, 0x100,	/* PSE Client */
	0x60000000, 0x20000, 0x200, /* WF_CFG */
	0x60100000, 0x21000, 0x200, /* WF_TRB */
	0x60110000, 0x21200, 0x200, /* WF_AGG */
	0x60120000, 0x21400, 0x200, /* WF_ARB */
	0x60130000, 0x21600, 0x200, /* WF_TMAC */
	0x60140000, 0x21800, 0x200, /* WF_RMAC */
	0x60150000, 0x21A00, 0x200, /* WF_SEC */
	0x60160000, 0x21C00, 0x200, /* WF_DMA */
	0x60170000, 0x21E00, 0x200, /* WF_CFGOFF */
	0x60180000, 0x22000, 0x1000, /* WF_PF */
	0x60190000, 0x23000, 0x200, /* WF_WTBLOFF */
	0x601A0000, 0x23200, 0x200, /* WF_ETBF */
	
	0x60300000, 0x24000, 0x400, /* WF_LPON */
	0x60310000, 0x24400, 0x200, /* WF_INT */
	0x60320000, 0x28000, 0x4000, /* WF_WTBLON */
	0x60330000, 0x2C000, 0x200, /* WF_MIB */
	0x60400000, 0x2D000, 0x200, /* WF_AON */

	0x80020000, 0x00000, 0x2000, /* TOP_CFG */	
	0x80000000, 0x02000, 0x2000, /* MCU_CFG */
	0x50000000, 0x04000, 0x4000, /* PDMA_CFG */
	0xA0000000, 0x08000, 0x8000, /* PSE_CFG */
	0x60200000, 0x10000, 0x10000, /* WF_PHY */
	
	0x0, 0x0, 0x0,
};
BOOLEAN mt_mac_cr_range_mapping(RTMP_ADAPTER *pAd, UINT32 *mac_addr)
{
	UINT32 mac_addr_hif = *mac_addr;
	INT idx = 0;
	BOOLEAN IsFound = 0;
	UINT32 *mac_cr_range = NULL;

	if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT76x6(pAd) || IS_MT7637(pAd))
		mac_cr_range = &mt_dft_mac_cr_range[0];

	if (!mac_cr_range)
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): NotSupported Chip for this function!\n", __FUNCTION__));
		return IsFound;
	}

	if (mac_addr_hif >= 0x40000)
	{
		do
		{
			if (mac_addr_hif >= mac_cr_range[idx] && 
				mac_addr_hif < (mac_cr_range[idx] + mac_cr_range[idx + 2]))
			{
				mac_addr_hif -= mac_cr_range[idx];
				mac_addr_hif += mac_cr_range[idx + 1];
				IsFound = 1;
				break;
			}
			idx += 3;
		} while (mac_cr_range[idx] != 0);
	} 
	else
	{
		IsFound = 1;
	}

	*mac_addr = mac_addr_hif;

	return IsFound;
}

UINT32 mt_physical_addr_map(UINT32 addr)
{	
	UINT32 global_addr = 0x0, idx = 1;	
	extern UINT32 mt_mac_cr_range[];	
	if (addr < 0x2000)		
		global_addr = 0x80020000 + addr;	
	else if ((addr >= 0x2000) && (addr < 0x4000))		
		global_addr = 0x80000000 + addr - 0x2000;	
	else if ((addr >= 0x4000) && (addr < 0x8000))		
		global_addr = 0x50000000 + addr - 0x4000;	
	else if ((addr >= 0x8000) && (addr < 0x10000))		
		global_addr = 0xa0000000 + addr - 0x8000;	
	else if ((addr >= 0x10000) && (addr < 0x20000))		
		global_addr = 0x60200000 + addr - 0x10000;	
	else if ((addr >= 0x20000) && (addr < 0x40000))	
	{		
		do 
		{			
			if ((addr >= mt_mac_cr_range[idx]) && (addr < (mt_mac_cr_range[idx]+mt_mac_cr_range[idx+1]))) 
			{				
				global_addr = mt_mac_cr_range[idx-1]+(addr-mt_mac_cr_range[idx]);				
				break;			
			}			
			idx += 3;		
		}while (mt_mac_cr_range[idx] != 0);		
		if (mt_mac_cr_range[idx] == 0)		
		{			
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s unknow addr range = %x !!!\n",__FUNCTION__, addr));					
		}	
	}	
	else		
		global_addr = addr;	

	return global_addr;
}

static INT32 ResponseToQA(
	HQA_CMD_FRAME *HqaCmdFrame,
	RTMP_IOCTL_INPUT_STRUCT	*WRQ,
	UINT32 Length,
	INT32 Status)
{
	NdisMoveMemory(HqaCmdFrame->Data, &Status, 2);
	HqaCmdFrame->Length = (UINT16)Length;
	*WRQ->BytesRet = sizeof((HqaCmdFrame)->MagicNo) + sizeof((HqaCmdFrame)->Type)
							+ sizeof((HqaCmdFrame)->Id) + sizeof((HqaCmdFrame)->Length)
							+ sizeof((HqaCmdFrame)->Sequence) + Length;
	if(*WRQ->BytesRet >WRQ->OutBufLen)
	{
		*WRQ->BytesRet = WRQ->OutBufLen;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  OutBufLen = 0x%x\n", __FUNCTION__, WRQ->OutBufLen));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:  BytesRet = 0x%x\n", __FUNCTION__, *WRQ->BytesRet));
	RTMPMoveMemory(WRQ->OutBuf, WRQ->InBuf, *WRQ->BytesRet);
	return Status;
}

#else  /*defined(COMPOS_TESTMODE_WIN) || defined(COMPOS_WIN)*/
static INT ResponseToQA(
	HQA_CMD_FRAME *HqaCmdFrame,
	RTMP_IOCTL_INPUT_STRUCT	*WRQ,
	INT32 Length,
	INT32 Status)
{
	HqaCmdFrame->Length = PKTS_TRAN_TO_NET((Length));

	Status = PKTS_TRAN_TO_NET((Status));
	NdisCopyMemory(HqaCmdFrame->Data, &Status, 2);

	WRQ->u.data.length = sizeof((HqaCmdFrame)->MagicNo) + sizeof((HqaCmdFrame)->Type)
							+ sizeof((HqaCmdFrame)->Id) + sizeof((HqaCmdFrame)->Length)
							+ sizeof((HqaCmdFrame)->Sequence) + PKTS_TRAN_TO_HOST((HqaCmdFrame)->Length);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("WRQ->u.data.length = %d, usr_addr:%p, hqa_addr:%p\n"
			, WRQ->u.data.length, WRQ->u.data.pointer, HqaCmdFrame));

	if (copy_to_user(WRQ->u.data.pointer, (UCHAR *)(HqaCmdFrame), WRQ->u.data.length))
	{

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("copy_to_user() fail in %s\n", __FUNCTION__));
		return (-EFAULT);
	}
	else
	{
	}
	return 0;
}
#endif /* TODO: Add lack of functions temporarily, and delete after merge */

static INT32 HQA_OpenAdapter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Ret = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQAEnabled = TRUE;

	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	if(ATEOp->ATEStart)
		Ret = ATEOp->ATEStart(pAd);
	else
		Ret = TM_STATUS_NOTSUPPORT;
	return Ret;
}

static INT32 HQA_CloseAdapter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQAEnabled = FALSE;

	/* Prepare feedback as soon as we can to avoid QA timeout. */
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_StartTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 TxCount;
	UINT16 TxLength;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy(&TxCount, HqaCmdFrame->Data, 4);
	memcpy(&TxLength, HqaCmdFrame->Data + 4, 2);

	TxCount = PKTL_TRAN_TO_HOST(TxCount);
	TxLength = PKTS_TRAN_TO_HOST(TxLength);

	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxCount, TxCount);
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxLength, TxLength);
	ATECtrl->bQATxStart = TRUE;
	if (ATEOp->StartTx) {
		Ret = ATEOp->StartTx(pAd, band_idx);
	} else
		Ret = TM_STATUS_NOTSUPPORT;

	if (ATECtrl->bQATxStart == TRUE)
		ATECtrl->TxStatus = 1;

	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_StartTxExt(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	return Ret;
}

//1 todo not support yet
static INT32 HQA_StartTxContiTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_StartTxCarrier(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StartRx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ATECtrl->bQARxStart = TRUE;
	if (ATEOp->StartRx)
		Ret = ATEOp->StartRx(pAd, band_idx);
	else
		Ret = TM_STATUS_NOTSUPPORT;

	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Mode;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	Mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode);
	Mode &= ATE_TXSTOP;
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, Mode);

	ATECtrl->bQATxStart = FALSE;
	if(ATEOp->StopTx)
		Ret = ATEOp->StopTx(pAd, band_idx);
	else
		Ret = TM_STATUS_NOTSUPPORT;
	ATECtrl->TxStatus = 0;
	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopContiTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	/* TODO Get Correct TxfdMode*/
	UINT32 TxfdMode = 1;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	{
		if(ATEOp->StopContinousTx)
			ATEOp->StopContinousTx(pAd, TxfdMode, TESTMODE_BAND0);
		else
			Ret = TM_STATUS_NOTSUPPORT;		
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_StopTxCarrier(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_StopRx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Mode;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	Mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode);
	Mode &= ATE_RXSTOP;
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, Mode);

	ATECtrl->bQARxStart = FALSE;
	if(ATEOp->StopRx)
		Ret = ATEOp->StopRx(pAd, band_idx);
	else
		Ret = TM_STATUS_NOTSUPPORT;
	
	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxPath(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;	
	INT16 Value = 0;
	INT32 ant_sel = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	ant_sel = PKTS_TRAN_TO_HOST(Value);
	if (ant_sel&0x8000) {
		ant_sel &= 0x7FFF;
		ant_sel |= 0x80000000;
	}
	if (ATEOp->SetTxAntenna)
		Ret = ATEOp->SetTxAntenna(pAd, ant_sel, band_idx);
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	
	err0:
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetRxPath(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;	

	INT16 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	if (ATEOp->SetRxAntenna)
		Ret = ATEOp->SetRxAntenna(pAd, (CHAR)Value, band_idx);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTxIPG(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, (" %s, val:%d,0x%04x\n", __FUNCTION__,Value,Value));
	if(ATEOp->SetAIFS)
		ATEOp->SetAIFS(pAd, (UINT32)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;		

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;

}


static INT32 HQA_SetTxPower0(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Value;
	TxPower.Dbdc_idx = band_idx;
	if(ATEOp->SetTxPower0)
		Ret = ATEOp->SetTxPower0(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HAQ_SetTxPower1(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT16 Value = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx == -1) {
		Ret = -1;
		goto err0;
	}
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Value;
	TxPower.Dbdc_idx = band_idx;
	if(ATEOp->SetTxPower1)
		Ret = ATEOp->SetTxPower1(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;		

err0:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetTxPowerExt(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data; 
	ATE_TXPOWER TxPower;
	UINT32 power = 0;
	UINT32 Channel = 0;
	UINT32 dbdc_idx = 0;
	UINT32 band_idx = 0;
	UINT32 ant_idx = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&power);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&dbdc_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Channel);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ant_idx);
	
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Ant_idx = ant_idx;
	TxPower.Power = power;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = dbdc_idx;
	TxPower.Band_idx = band_idx;
	
	if(ATEOp->SetTxPowerX)
		Ret = ATEOp->SetTxPowerX(pAd, TxPower);
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_AntennaSel(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	// todo wait FW confirm
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}
static INT32 HQA_SetOnOFF(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	//MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	return Ret;
}


static INT32 HQA_FWPacketCMD_ClockSwitchDisable(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 isDisable = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));


	EthGetParamAndShiftBuff(TRUE, sizeof(isDisable), &data, (UCHAR *)&isDisable);

	ATEOp->ClockSwitchDisable(pAd, isDisable);
	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_SetTxPowerEval(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	
	//MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	return Ret;
}

static INT32 HQA_AntennaSelExt (PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 RfModeMask = 0;
	UINT32 RfPortMask = 0;
	UINT32 AntPortMask = 0;
	UINT32 BandIdx = 0;
	
	UCHAR *data = HqaCmdFrame->Data;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&BandIdx, data, sizeof(BandIdx));
	data += sizeof(BandIdx);
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);
	
	NdisMoveMemory((PUCHAR)&RfModeMask, data, sizeof(RfModeMask));
	data += sizeof(RfModeMask);
	RfModeMask = PKTL_TRAN_TO_HOST(RfModeMask);

	NdisMoveMemory((PUCHAR)&RfPortMask, data, sizeof(RfPortMask));
	data += sizeof(RfPortMask);
	RfPortMask = PKTL_TRAN_TO_HOST(RfPortMask);

	NdisMoveMemory((PUCHAR)&AntPortMask, data, sizeof(AntPortMask));
	data += sizeof(AntPortMask);
	AntPortMask = PKTL_TRAN_TO_HOST(AntPortMask);

	Ret = ATEOp->SetAntennaPort(pAd, RfModeMask, RfPortMask, AntPortMask);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, BandIdx:%x, RfModeMask:%x, RfPortMask:%x, AntPortMask:%x\n"
			, __FUNCTION__, BandIdx, RfModeMask, RfPortMask, AntPortMask));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

#ifdef ABSOLUTE_POWER_TEST
static INT32 HQA_SetAbsoluteTxPower (PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 BandIdx = 0;
	UINT32 Power = 0;
	UINT32 PhyMode = 0;
	UINT32 TxRate = 0;
	UINT32 BW = 0;
	
	UCHAR *data = HqaCmdFrame->Data;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	NdisMoveMemory((PUCHAR)&BandIdx, data, sizeof(BandIdx));
	data += sizeof(BandIdx);
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);
	
	NdisMoveMemory((PUCHAR)&Power, data, sizeof(Power));
	data += sizeof(Power);
	Power = PKTL_TRAN_TO_HOST(Power);

	NdisMoveMemory((PUCHAR)&PhyMode, data, sizeof(PhyMode));
	data += sizeof(PhyMode);
	PhyMode = PKTL_TRAN_TO_HOST(PhyMode);

	NdisMoveMemory((PUCHAR)&TxRate, data, sizeof(TxRate));
	data += sizeof(TxRate);
	TxRate = PKTL_TRAN_TO_HOST(TxRate);

	NdisMoveMemory((PUCHAR)&BW, data, sizeof(BW));
	data += sizeof(BW);
	BW = PKTL_TRAN_TO_HOST(BW);

	Ret = ATEOp->SetTxForceTxPower(pAd, BandIdx, Power, PhyMode, TxRate, BW);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, BandIdx:%x, Power:%x, PhyMode:%x, TxRate:%x, BW:%x\n"
			, __FUNCTION__, BandIdx, Power, PhyMode, TxRate, BW));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif /* ABSOLUTE_POWER_TEST */

static HQA_CMD_HANDLER HQA_CMD_SET0[] =
{
	/* cmd id start from 0x1000 */
	HQA_OpenAdapter,	/* 0x1000 */
	HQA_CloseAdapter,	/* 0x1001 */
	HQA_StartTx,		/* 0x1002 */
	HQA_StartTxExt,		/* 0x1003 */
	HQA_StartTxContiTx,	/* 0x1004 */
	HQA_StartTxCarrier,	/* 0x1005 */
	HQA_StartRx,		/* 0x1006 */
	HQA_StopTx,			/* 0x1007 */
	HQA_StopContiTx,	/* 0x1008 */
	HQA_StopTxCarrier,	/* 0x1009 */
	HQA_StopRx,			/* 0x100A */
	HQA_SetTxPath,		/* 0x100B */
	HQA_SetRxPath,		/* 0x100C */
	HQA_SetTxIPG,		/* 0x100D */
	HQA_SetTxPower0,	/* 0x100E */
	HAQ_SetTxPower1,	/* 0x100F */
	HQA_SetTxPowerEval,	/* 0x1010 */
	HQA_SetTxPowerExt,	/* 0x1011 */
	HQA_SetOnOFF,	    /* 0x1012 */
	HQA_AntennaSel,     /* 0x1013 */
	HQA_FWPacketCMD_ClockSwitchDisable, /* 0x1014 */
	HQA_AntennaSelExt,  /* 0x1015 */
#ifdef ABSOLUTE_POWER_TEST	
	HQA_SetAbsoluteTxPower,  /* 0x1016 */
#endif /* #ifdef ABSOLUTE_POWER_TEST */	
};

static INT32 HQA_SetChannel(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Ret = 0;

	UINT32 Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((UINT8 *)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Channel = %d, BW = %d\n", __FUNCTION__, ATECtrl->Channel, ATECtrl->BW));

	ATECtrl->Channel = (UINT8)Value;

	if(ATEOp->SetChannel)
		Ret = ATEOp->SetChannel(pAd, (UINT16)Value, TESTMODE_BAND0, 0, 0 ,0);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_SetPreamble(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);
	/* 000: Legacy CCK
	 * 001: Legacy OFDM
	 * 010: HT Mixed mode
	 * 011: HT Green field mode
	 * 100: VHT mode
	 */
    ATECtrl->PhyMode = (UCHAR)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetRate(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	ATECtrl->Mcs = (UCHAR)Value;

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetNss(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetSystemBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{	
	INT32 Ret = 0;
	UINT32 Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	/* 0: BW_20, 1:BW_40, 2:BW_80, 3:BW_160*/
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: BW = %d\n", __FUNCTION__, Value));
	if(ATEOp->SetBW)
		Ret = ATEOp->SetBW(pAd, (INT16)Value, TESTMODE_BAND0);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetPerPktBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, %u\n", __FUNCTION__, Value));
	if(Value > ATECtrl->BW)
		Value = ATECtrl->BW;
	ATECtrl->PerPktBW = Value;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetPrimaryBW(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetFreqOffset(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	if(ATEOp->SetTxFreqOffset)
		ATEOp->SetTxFreqOffset(pAd, (UINT32)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_SetAutoResponder(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetTssiOnOff(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{	
	INT32 Ret = 0;

	INT32 Value = 0, WFSel;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    /* ON/OFF:4 WF Sel:4 */
	memcpy((PUCHAR)&Value, HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	memcpy((PUCHAR)&WFSel, HqaCmdFrame->Data + 4, 4);
	WFSel = PKTL_TRAN_TO_HOST(WFSel);
	if(ATEOp->SetTSSI)
		ATEOp->SetTSSI(pAd, (CHAR)WFSel, (CHAR)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

//1 todo not support yet
static INT32 HQA_SetRxHighLowTemperatureCompensation(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_LowPower(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{	
	INT32 Ret = 0;

	INT32 Control = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(Control), &data, (UCHAR *)&Control);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Control:%d\n", __FUNCTION__, Control));

	if(ATEOp->LowPower)
		ATEOp->LowPower(pAd, Control);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET1[] =
{
	/* cmd id start from 0x1100 */
	HQA_SetChannel,				/* 0x1100 */
	HQA_SetPreamble,			/* 0x1101 */
	HQA_SetRate,				/* 0x1102 */
	HQA_SetNss,					/* 0x1103 */
	HQA_SetSystemBW,			/* 0x1104 */
	HQA_SetPerPktBW,			/* 0x1105 */
	HQA_SetPrimaryBW,			/* 0x1106 */
	HQA_SetFreqOffset,			/* 0x1107 */
	HQA_SetAutoResponder,		/* 0x1108 */
	HQA_SetTssiOnOff,			/* 0x1109 */
	HQA_SetRxHighLowTemperatureCompensation,	/* 0x110A */
	HQA_LowPower,				/* 0x110B */
};

static INT32 HQA_ResetTxRxCounter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	
	MtAsicGetRxStat(pAd, HQA_RX_RESET_PHY_COUNT); 
	MtAsicGetRxStat(pAd, HQA_RX_RESET_MAC_COUNT); 
#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND0);
	MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND0);
	if (IS_ATE_DBDC(pAd)){
		MtCmdSetPhyCounter(pAd, 0, TESTMODE_BAND1);
		MtCmdSetPhyCounter(pAd, 1, TESTMODE_BAND1);
	}
#endif
	MT_ATEUpdateRxStatistic(pAd, TESTMODE_RESET_CNT, NULL);
	ATECtrl->TxDoneCount = 0;
	if (IS_ATE_DBDC(pAd))
		TESTMODE_SET_PARAM(ATECtrl, TESTMODE_BAND1, TxDoneCount, 0);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32  	HQA_GetChipID(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 ChipId;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, TOP_HCR, &ChipId);
	ChipId = OS_NTOHL(ChipId);
	memcpy(HqaCmdFrame->Data + 2, &ChipId, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

static INT32 HQA_GetStatistics(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
//1 todo not support yet
static INT32 HQA_GetRxOKData(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_GetRxOKOther(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_GetRxAllPktCount(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_GetTxTransmitted(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

    Value = ATECtrl->TxDoneCount;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: TxDoneCount = %d\n", __FUNCTION__, ATECtrl->TxDoneCount));

	Value = PKTL_TRAN_TO_NET(Value);

	memcpy(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_GetHwCounter(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_CalibrationOperation(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_CalibrationBypassExt(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 item = 0;
	UINT32 band_idx = 0;
	UCHAR *data = HqaCmdFrame->Data;
	
	NdisMoveMemory((UCHAR *)&item, data, sizeof(item));
	data += sizeof(item);
	item = PKTL_TRAN_TO_HOST(item);
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	MtCmdDoCalibration(pAd, CALIBRATION_BYPASS, item, band_idx);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, item:%x, band_idx:%x\n"
			, __FUNCTION__, item, band_idx));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetRXVectorIdx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 Group_1 = 0, Group_2 = 0;
	UCHAR *data = HqaCmdFrame->Data;
	
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	NdisMoveMemory((UCHAR *)&Group_1, data, sizeof(Group_1));
	data += sizeof(Group_1);
	Group_1 = PKTL_TRAN_TO_HOST(Group_1);
	NdisMoveMemory((UCHAR *)&Group_2, data, sizeof(Group_2));
	data += sizeof(Group_2);
	Group_2 = PKTL_TRAN_TO_HOST(Group_2);

#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetRxvIndex (pAd, Group_1, Group_2, band_idx);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, band_idx:%d, G1:%d, G2:%d\n"
			, __FUNCTION__, band_idx, Group_1, Group_2));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetFAGCRssiPath(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 FAGC_Path = 0;
	UCHAR *data = HqaCmdFrame->Data;
	ATE_CTRL *ate_ctrl = &pAd->ATECtrl;
	
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	NdisMoveMemory((UCHAR *)&FAGC_Path, data, sizeof(FAGC_Path));
	data += sizeof(FAGC_Path);
	FAGC_Path = PKTL_TRAN_TO_HOST(FAGC_Path);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, FAGC_Path, FAGC_Path);

#ifdef CONFIG_HW_HAL_OFFLOAD
	MtCmdSetFAGCPath(pAd, FAGC_Path, band_idx);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, band_idx:%d, FAGC_Path%d\n"
			, __FUNCTION__, band_idx, FAGC_Path));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET2[] =
{
	/* cmd id start from 0x1200 */
	HQA_ResetTxRxCounter,	/* 0x1200 */	
	HQA_GetStatistics,		/* 0x1201 */
	HQA_GetRxOKData,		/* 0x1202 */
	HQA_GetRxOKOther,		/* 0x1203 */
	HQA_GetRxAllPktCount,	/* 0x1204 */
	HQA_GetTxTransmitted,	/* 0x1205 */
	HQA_GetHwCounter,		/* 0x1206 */
	HQA_CalibrationOperation,	/* 0x1207 */
 	HQA_CalibrationBypassExt,	/* 0x1208 */
 	HQA_SetRXVectorIdx,	/* 0x1209 */
 	HQA_SetFAGCRssiPath,	/* 0x120A */
};
#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
static VOID memcpy_exs(PRTMP_ADAPTER pAd, UCHAR *dst, UCHAR *src, ULONG len)
{
	ULONG i;
	{
		USHORT *pDst, *pSrc;

		pDst = (USHORT *) dst;
		pSrc = (USHORT *) src;

		for (i =0; i < (len >> 1); i++)
		{
			*pDst = PKTS_TRAN_TO_HOST(*pSrc);
			pDst++;
			pSrc++;
		}

		if ((len % 2) != 0)
		{
			memcpy(pDst, pSrc, (len % 2));
			*pDst = PKTS_TRAN_TO_HOST(*pDst);
		}
	}
	return;
}
#endif
static INT32 HQA_MacBbpRegRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;
	BOOLEAN IsFound;

	memcpy(&Offset, HqaCmdFrame->Data, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);

	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);
#if defined (COMPOS_TESTMODE_WIN) ||defined (COMPOS_WIN)
	Offset = mt_physical_addr_map(Offset);
	IsFound = TRUE;
#endif

	if (!IsFound)
	{
		UINT32 RemapBase, RemapOffset;
		UINT32 RestoreValue;
		HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(Offset) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(Offset);
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		HW_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	}
	else
	{
		HW_IO_READ32(pAd, Offset, &Value);
	}


	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);

	return Ret;
}


static INT32 HQA_MacBbpRegWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset, Value;
	BOOLEAN IsFound;

	memcpy(&Offset, HqaCmdFrame->Data, 4);
	memcpy(&Value, HqaCmdFrame->Data + 4, 4);

	Offset = PKTL_TRAN_TO_HOST(Offset);
	Value = PKTL_TRAN_TO_HOST(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);
#if defined (COMPOS_TESTMODE_WIN) ||defined (COMPOS_WIN)
	Offset = mt_physical_addr_map(Offset);
	IsFound = TRUE;
#endif
	if (!IsFound)
	{
		UINT32 RemapBase, RemapOffset;
		UINT32 RestoreValue;
		HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
		RemapBase = GET_REMAP_2_BASE(Offset) << 19;
		RemapOffset = GET_REMAP_2_OFFSET(Offset);
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
		HW_IO_WRITE32(pAd, 0x80000 + RemapOffset, Value);
		HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
	}
	else
	{
		HW_IO_WRITE32(pAd, Offset, Value);
	}

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
#define REG_SIZE 128
RTMP_REG_PAIR RegPair[REG_SIZE];

VOID RTMP_IO_MCU_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *Dst, UINT32 Offset, UINT32 Len)
{
	UINT32 Index, Value = 0;
	UCHAR *pDst;
	UINT32 NumOfReg = (Len >> 2);
	UINT32 Reg_idx = 0; /* Rate is 0 to REG_SIZE-1 */
	UINT32 i;
	UCHAR OffsetByte = 0x4;
	UINT32 Ret_idx = 0;
	for (Index = 0 ; Index < NumOfReg; Index++) {
		RegPair[Reg_idx].Register = Offset + OffsetByte * Index;
		/* Read CR per REG_SIZE or lastest CR */
		if (Reg_idx == REG_SIZE - 1 || Index == NumOfReg -1) {
			MtCmdMultipleMacRegAccessRead(pAd, RegPair, Reg_idx + 1);
			for (i = 0; i <= Reg_idx; i++) {
				pDst = (Dst + ((Ret_idx) << 2));
				Value = RegPair[i].Value;
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: Offset = %x, Value = %x\n", __FUNCTION__, RegPair[i].Register, Value));
				Value = PKTL_TRAN_TO_NET(Value);
				memmove(pDst, &Value, 4);
				Ret_idx++;
			}
			Reg_idx = 0;
		}
		else
			Reg_idx++;
	}
	return;
}
#endif /* !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) */

VOID RTMP_IO_READ_BULK(PRTMP_ADAPTER pAd, UCHAR *Dst, UINT32 Offset, UINT32 Len)
{
	UINT32 Index, Value = 0;
	UCHAR *pDst;
	UINT32 NumOfReg = (Len >> 2);
	BOOLEAN IsFound;
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n\n"));

#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
	IsFound = mt_mac_cr_range_mapping(pAd, &Offset);
	if (!IsFound && NumOfReg > 1 && Offset && !(ATECtrl->Mode & fATE_IN_ICAPOVERLAP)){
		RTMP_IO_MCU_READ_BULK(pAd, Dst, Offset, Len);
		return;
	}
#endif /* !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) */

	for (Index = 0 ; Index < NumOfReg; Index++)
	{
		pDst = (Dst + (Index << 2));
		IsFound = mt_mac_cr_range_mapping(pAd, &Offset);
#if defined (COMPOS_TESTMODE_WIN) ||defined (COMPOS_WIN)
		Offset = mt_physical_addr_map(Offset);
		IsFound = TRUE;
#endif
		if (!IsFound)
		{
			UINT32 RemapBase, RemapOffset;
			UINT32 RestoreValue;
			HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &RestoreValue);
			RemapBase = GET_REMAP_2_BASE(Offset) << 19;
			RemapOffset = GET_REMAP_2_OFFSET(Offset);
			HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RemapBase);
			HW_IO_READ32(pAd, 0x80000 + RemapOffset, &Value);
			HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, RestoreValue);
		}
		else
		{
			HW_IO_READ32(pAd, Offset, &Value);
		}

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s: Offset = %x, Value = %x\n", __FUNCTION__, Offset, Value));

		Value = PKTL_TRAN_TO_NET(Value);
		memmove(pDst, &Value, 4);
		Offset += 4;
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN, ("\n\n"));

	return;
}

static INT32 HQA_MACBbpRegBulkRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Offset;
	UINT16 Len, Tmp;
	INT debug_lvl = DebugLevel;
	DebugLevel = DBG_LVL_OFF;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy(&Offset, HqaCmdFrame->Data, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);

	memcpy(&Len, HqaCmdFrame->Data + 4, 2);
	Len = PKTS_TRAN_TO_HOST(Len);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Len(unit: 4bytes) = %d\n", __FUNCTION__, Offset, Len));

	if (Len > 371)
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : length requested is too large, make it smaller\n", __FUNCTION__));
		HqaCmdFrame->Length = PKTS_TRAN_TO_NET(2);
		Tmp = PKTS_TRAN_TO_NET(1);
		memcpy(HqaCmdFrame->Data, &Tmp, 2);

		return -EFAULT;
	}

	RTMP_IO_READ_BULK(pAd, HqaCmdFrame->Data + 2, Offset, (Len << 2));/* unit in four bytes*/

	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Len << 2), Ret);
	DebugLevel = debug_lvl;

	return Ret;
}


static INT32 HQA_RfRegBulkRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	UINT32 Index, WfSel, Offset, Length, Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = PKTL_TRAN_TO_HOST(WfSel);

	memcpy(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);

	memcpy(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = PKTL_TRAN_TO_HOST(Length);
	if(ATEOp->RfRegRead)	
	{
		for (Index = 0; Index < Length; Index++)
		{
			Ret = ATEOp->RfRegRead(pAd, WfSel, Offset + Index * 4, &Value);

			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Wfsel = %d, Offset = %x, Value = %x\n", __FUNCTION__, WfSel,
																	Offset + Index * 4, Value));

			if (Ret)
			{
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wfsel = %d, Offset = %x, Value = %x fail\n", WfSel,
																	Offset + Index * 4, Value));
				break;
			}

			Value = PKTL_TRAN_TO_NET(Value);
			memcpy(HqaCmdFrame->Data + 2 + (Index * 4), &Value, 4);
		}
	}
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	ResponseToQA(HqaCmdFrame, WRQ, 2 + (Length * 4), Ret);

	return Ret;
}


static INT32 HQA_RfRegBulkWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	UINT32 Index, WfSel, Offset, Length, Value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy(&WfSel, HqaCmdFrame->Data, 4);
	WfSel = PKTL_TRAN_TO_HOST(WfSel);

	memcpy(&Offset, HqaCmdFrame->Data + 4, 4);
	Offset = PKTL_TRAN_TO_HOST(Offset);

	memcpy(&Length,  HqaCmdFrame->Data + 8, 4);
	Length = PKTL_TRAN_TO_HOST(Length);
	if(ATEOp->RfRegWrite)
	{
		for (Index = 0; Index < Length; Index++)
		{
			memcpy(&Value, HqaCmdFrame->Data + 12 + (Index * 4), 4);
			Value = PKTL_TRAN_TO_HOST(Value);
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Wfsel = %d, Offset = %x, Value = %x\n", __FUNCTION__,
													WfSel,Offset + Index * 4, Value));

			Ret = ATEOp->RfRegWrite(pAd, WfSel, Offset + Index * 4, Value);

			if (Ret)
			{
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Wfsel = %d, Offset = %x, Value = %x fail\n", WfSel,
																	Offset + Index * 4, Value));
				break;
			}
		}
	}
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_ReadEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)//1Todo	 RT28xx_EEPROM_READ16	
	UINT16 Offset=0, Value=0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);

	RT28xx_EEPROM_READ16(pAd, Offset, Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p r %02Xh = 0x%02X\n"
		, (Offset & 0x00FF), (Value & 0x00FF)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p r %02Xh = 0x%02X\n"
		, (Offset & 0x00FF)+1, (Value & 0xFF00)>>8));

	Value = PKTS_TRAN_TO_NET(Value);

	memcpy(HqaCmdFrame->Data + 2, &Value, 2);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 4, Ret);

	return Ret;
}


static INT32 HQA_WriteEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)//1Todo	 RT28xx_EEPROM_WRITE16	
	UINT16 Offset=0, Value=0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);

	memcpy(&Value, HqaCmdFrame->Data + 2, 2);
	Value = PKTS_TRAN_TO_HOST(Value);
	RT28xx_EEPROM_WRITE16(pAd, Offset, Value);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w 0x%04X = 0x%04X\n", Offset, Value));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w %02Xh = 0x%02X\n"
		, (Offset & 0x00FF), (Value & 0x00FF)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("e2p w %02Xh = 0x%02X\n"
		, (Offset & 0x00FF) + 1, (Value & 0xFF00)>>8));
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}


static INT32 HQA_ReadBulkEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	 EEReadAll
	UINT16 Offset;
	UINT16 Len;
	UINT16 *Buffer = NULL;

	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);

	memcpy(&Len, HqaCmdFrame->Data + 2, 2);
	Len = PKTS_TRAN_TO_HOST(Len);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Offset = %x, Length = %x\n", __FUNCTION__, Offset, Len));

    RTMP_OS_NETDEV_STOP_QUEUE(pAd->net_dev);

	Ret = os_alloc_mem(pAd,(PUCHAR *)&Buffer, EEPROM_SIZE);	//TODO verify
	if(Ret == NDIS_STATUS_FAILURE){
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : allocate memory for read EEPROM fail\n", __FUNCTION__));
		Len = 0;
		goto HQA_ReadBulkEEPROM_RET;
	}
	EEReadAll(pAd, (UINT16 *)Buffer);

	if (Offset + Len <= EEPROM_SIZE)
		memcpy_exs(pAd, HqaCmdFrame->Data + 2, (UCHAR *)Buffer+Offset, Len);
	else
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : exceed EEPROM size\n", __FUNCTION__));
		Len = 0;
		Ret = -1;
	}

	os_free_mem(Buffer);

    RTMP_OS_NETDEV_START_QUEUE(pAd->net_dev);
HQA_ReadBulkEEPROM_RET:
	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);
#endif
	return Ret;
}


static VOID EEWriteBulk(PRTMP_ADAPTER pAd, UINT16 *Data, UINT16 Offset, UINT16 Length)
{
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	ATE_CTRL RT28xx_EEPROM_WRITE16
	UINT16 Pos;
	UINT16 Value;
	UINT16 Len = Length;

	for (Pos = 0; Pos < (Len >> 1);)
	{
		Value = Data[Pos];
		RT28xx_EEPROM_WRITE16(pAd, Offset + (Pos * 2), Value);
		Pos++;
	}
#endif
	return;
}


static INT32 HQA_WriteBulkEEPROM(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	 RT28xx_EEPROM_WRITE16	
	USHORT Offset;
	USHORT Len;
	UINT16 *Buffer = NULL;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s\n", __FUNCTION__));

	memcpy(&Offset, HqaCmdFrame->Data, 2);
	Offset = PKTS_TRAN_TO_HOST(Offset);

	memcpy(&Len, HqaCmdFrame->Data + 2, 2);
	Len = PKTS_TRAN_TO_HOST(Len);

	Ret = os_alloc_mem(pAd,(PUCHAR *)&Buffer, EEPROM_SIZE);	//TODO verify
	if(Ret == NDIS_STATUS_FAILURE){
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : allocate memory for read EEPROM fail\n", __FUNCTION__));
		goto HQA_WriteBulkEEPROM_RET;
	}

	memcpy_exs(pAd, (UCHAR *)Buffer + Offset, (UCHAR *)HqaCmdFrame->Data + 4, Len);

#if defined(RTMP_RBUS_SUPPORT) || defined(RTMP_FLASH_SUPPORT)
    if (Len == 16) {
        memcpy(pAd->eebuf + Offset, (UCHAR *)Buffer + Offset, Len);
    }
    if ((Offset + Len) == EEPROM_SIZE)
        rtmp_ee_flash_write_all(pAd, (PUSHORT)pAd->eebuf);

    if (Len != 16)
#endif
    {
        if ((Offset + Len) <= EEPROM_SIZE)
        {
            EEWriteBulk(pAd,(UINT16 *)(((UCHAR *)Buffer) + Offset), Offset, Len);
        }
        else
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : exceed EEPROM size(%d)\n", __FUNCTION__, EEPROM_SIZE));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Offset = %u\n", Offset));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Length = %u\n", Len));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("Offset + Length=%u\n", (Offset + Len)));
        }
    }

HQA_WriteBulkEEPROM_RET:
	ResponseToQA(HqaCmdFrame, WRQ, 2 + Len, Ret);
#endif
	return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
static INT32 HQA_CheckEfuseMode(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	eFuseGetFreeBlockCount		
	UINT32 Value;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	if (pAd->bUseEfuse)
		Value = 1;
	else
		Value = 0;


	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}


static INT32 HQA_GetFreeEfuseBlock(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	eFuseGetFreeBlockCount	
	// remove this block when command RSP function ready
	UINT32 Value;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	eFuseGetFreeBlockCount(pAd, &Value);

	Value = PKTL_TRAN_TO_NET(Value);
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);
	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
#else
	INT32 GetFreeBlock = 0;
	UINT32 Result = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s GetFreeBlock:%d \n", __FUNCTION__, GetFreeBlock));

	if(ATEOp->EfuseGetFreeBlock)
	{
		ATEOp->EfuseGetFreeBlock(pAd, GetFreeBlock, &Result);
	   	Result = PKTL_TRAN_TO_HOST(Result);
		memcpy(HqaCmdFrame->Data + 2, &Result, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	}
	else 
	{
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}
#endif
	
	return Ret;
}

//1 todo not support yet
static INT32 HQA_GetEfuseBlockNr(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_WriteEFuseFromBuffer(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif /* RTMP_EFUSE_SUPPORT */

static INT32 HQA_GetTxPower(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Channel = 0, Band = 0, Ch_Band = 0, EfuseAddr = 0, Power = 0;
	INT32 offset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Channel, (PUCHAR)&HqaCmdFrame->Data, sizeof(Channel));
	Channel = PKTL_TRAN_TO_HOST(Channel);
	offset += sizeof(Channel);

	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);

	memcpy((PUCHAR)&Ch_Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Ch_Band));
	Ch_Band = PKTL_TRAN_TO_HOST(Ch_Band);
	offset += sizeof(Ch_Band);

	if(ATEOp->GetTxPower){
		ATEOp->GetTxPower(pAd, Channel, Band, Ch_Band, &EfuseAddr, &Power);
		os_msec_delay(30);
	   	EfuseAddr = PKTL_TRAN_TO_HOST(EfuseAddr);
	   	Power = PKTL_TRAN_TO_HOST(Power);
		memcpy(HqaCmdFrame->Data + 2, &EfuseAddr, 4);
		memcpy(HqaCmdFrame->Data + 2 + 4, &Power, 4);
	}
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);
	return Ret;
}

static INT32 HQA_SetCfgOnOff(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{	
	INT32 Ret = 0;

	INT32 Type, Enable, Band;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Type, HqaCmdFrame->Data, 4);
	Type = PKTL_TRAN_TO_HOST(Type);

	memcpy((PUCHAR)&Enable, HqaCmdFrame->Data + 4, 4);
	Enable = PKTL_TRAN_TO_HOST(Enable);

	memcpy((PUCHAR)&Band, HqaCmdFrame->Data + 8, 4);
	Band = PKTL_TRAN_TO_HOST(Band);

	if(ATEOp->SetCfgOnOff)
		ATEOp->SetCfgOnOff(pAd, Type, Enable, Band);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_GetFreqOffset(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 FreqOffset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	if(ATEOp->GetTxFreqOffset){
		ATEOp->GetTxFreqOffset(pAd, &FreqOffset);
   		FreqOffset = PKTL_TRAN_TO_HOST(FreqOffset);
		memcpy(HqaCmdFrame->Data + 2, &FreqOffset, 4);
		ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	}
	else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}

	return Ret;
}

static INT32 HQA_DBDCTXTone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 BandIdx= 0, Control = 0, AntIndex = 0, ToneType = 0, ToneFreq = 0, DcOffset_I = 0, DcOffset_Q = 0, Band = 0;
	INT32 RF_Power = 0, Digital_Power = 0;
	INT32 offset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	/* BandIdx:4 Control:4 AntIndex:4 ToneType:4 ToneFreq:4 DcOffset_I:4 DcOffset_Q:4 Band:4 */
	memcpy((PUCHAR)&BandIdx, (PUCHAR)&HqaCmdFrame->Data, sizeof(BandIdx));
	BandIdx = PKTL_TRAN_TO_HOST(BandIdx);
	offset += sizeof(BandIdx);

	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);

	memcpy((PUCHAR)&AntIndex, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(AntIndex));
	AntIndex = PKTL_TRAN_TO_HOST(AntIndex);
	offset += sizeof(AntIndex);

	memcpy((PUCHAR)&ToneType, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(ToneType));
	ToneType = PKTL_TRAN_TO_HOST(ToneType);
	offset += sizeof(ToneType);

	memcpy((PUCHAR)&ToneFreq, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(ToneFreq));
	ToneFreq = PKTL_TRAN_TO_HOST(ToneFreq);
	offset += sizeof(ToneFreq);

	memcpy((PUCHAR)&DcOffset_I, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(DcOffset_I));
	DcOffset_I = PKTL_TRAN_TO_HOST(DcOffset_I);
	offset += sizeof(DcOffset_I);

	memcpy((PUCHAR)&DcOffset_Q, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(DcOffset_Q));
	DcOffset_Q = PKTL_TRAN_TO_HOST(DcOffset_Q);
	offset += sizeof(DcOffset_Q);

	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);

	memcpy((PUCHAR)&RF_Power, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(RF_Power));
	RF_Power = PKTL_TRAN_TO_HOST(RF_Power);
	offset += sizeof(RF_Power);

	memcpy((PUCHAR)&Digital_Power, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Digital_Power));
	Digital_Power = PKTL_TRAN_TO_HOST(Digital_Power);
	offset += sizeof(Digital_Power);

	if(ATEOp->SetDBDCTxTonePower)
		ATEOp->SetDBDCTxTonePower(pAd, RF_Power, Digital_Power, AntIndex, BandIdx);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	if(ATEOp->DBDCTxTone)
		ATEOp->DBDCTxTone(pAd, BandIdx, Control, AntIndex, ToneType, ToneFreq, DcOffset_I, DcOffset_Q, Band);
	else
		Ret = TM_STATUS_NOTSUPPORT;		
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_DBDCContinuousTX(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Band = 0, Control = 0, AntMask = 0, Phymode = 0, BW = 0;
	INT32 Pri_Ch = 0, Rate = 0, Central_Ch = 0, TxfdMode;
	INT32 offset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	/* Band:4 Control:4 AntIndex:4 Modulation:4 BW:4 Pri_Ch:4 Rate:4 */
	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);

	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);

	memcpy((PUCHAR)&AntMask, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(AntMask));
	AntMask = PKTL_TRAN_TO_HOST(AntMask);
	offset += sizeof(AntMask);

	memcpy((PUCHAR)&Phymode, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Phymode));
	Phymode = PKTL_TRAN_TO_HOST(Phymode);
	offset += sizeof(Phymode);

	memcpy((PUCHAR)&BW, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(BW));
	BW = PKTL_TRAN_TO_HOST(BW);
	offset += sizeof(BW);

	memcpy((PUCHAR)&Pri_Ch, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Pri_Ch));
	Pri_Ch = PKTL_TRAN_TO_HOST(Pri_Ch);
	offset += sizeof(Pri_Ch);

	memcpy((PUCHAR)&Rate, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Rate));
	Rate = PKTL_TRAN_TO_HOST(Rate);
	offset += sizeof(Rate);

	memcpy((PUCHAR)&Central_Ch, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Central_Ch));
	Central_Ch = PKTL_TRAN_TO_HOST(Central_Ch);
	offset += sizeof(Central_Ch);

	memcpy((PUCHAR)&TxfdMode, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(TxfdMode));
	TxfdMode = PKTL_TRAN_TO_HOST(TxfdMode);
	offset += sizeof(TxfdMode);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s : Band = %d, Control = %d, AntIndex = %d, Phymode = %d, BW = %d, CH = %d, Rate = %d, Central_Ch = %d, TxfdMode = %d\n",
			__FUNCTION__, Band, Control, AntMask, Phymode, BW, Pri_Ch, Rate, Central_Ch, TxfdMode));

	if (Control){
		if(ATEOp->StartContinousTx) {
			TESTMODE_SET_PARAM(ATECtrl, Band, PhyMode, Phymode);
			TESTMODE_SET_PARAM(ATECtrl, Band, BW, BW);
			TESTMODE_SET_PARAM(ATECtrl, Band, ControlChl, Pri_Ch);
			TESTMODE_SET_PARAM(ATECtrl, Band, Channel, Central_Ch);
			TESTMODE_SET_PARAM(ATECtrl, Band, Mcs, Rate);
			TESTMODE_SET_PARAM(ATECtrl, Band, TxAntennaSel, AntMask);
			ATEOp->StartContinousTx(pAd, AntMask, TxfdMode, Band);
		}
		else
			Ret = TM_STATUS_NOTSUPPORT;		
	}
	else{
		if(ATEOp->StopContinousTx)
			ATEOp->StopContinousTx(pAd, TxfdMode, Band);
		else
			Ret = TM_STATUS_NOTSUPPORT;		
	}
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetRXFilterPktLen(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Band = 0, Control = 0, RxPktlen = 0;
	INT32 offset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);

	memcpy((PUCHAR)&Control, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Control));
	Control = PKTL_TRAN_TO_HOST(Control);
	offset += sizeof(Control);

	memcpy((PUCHAR)&RxPktlen, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(RxPktlen));
	RxPktlen = PKTL_TRAN_TO_HOST(RxPktlen);
	offset += sizeof(RxPktlen);

	if(ATEOp->SetRXFilterPktLen)
		ATEOp->SetRXFilterPktLen(pAd, Control, Band, RxPktlen);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GetTXInfo(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UINT32 txed_band0 = 0;
	UINT32 txed_band1 = 0;
	UCHAR *data = HqaCmdFrame->Data + 2;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	txed_band0 = TESTMODE_GET_PARAM(ate_ctrl, 0, TxDoneCount);
	txed_band1 = TESTMODE_GET_PARAM(ate_ctrl, 1, TxDoneCount);
	txed_band0 = PKTL_TRAN_TO_NET(txed_band0);
	txed_band1 = PKTL_TRAN_TO_NET(txed_band1);
	NdisMoveMemory(data,(UCHAR *)&txed_band0, sizeof(txed_band0));
	data += sizeof(txed_band0);
	NdisMoveMemory(data,(UCHAR *)&txed_band1, sizeof(txed_band1));
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(txed_band0)+sizeof(txed_band1), Ret);
	return Ret;
}

static INT32 HQA_GetCfgOnOff(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Type = 0, Band = 0;
	UINT32 Result = 0;
	INT32 offset = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Type, (PUCHAR)&HqaCmdFrame->Data, sizeof(Type));
	Type = PKTL_TRAN_TO_HOST(Type);
	offset += sizeof(Type);

	memcpy((PUCHAR)&Band, (PUCHAR)&HqaCmdFrame->Data + offset, sizeof(Band));
	Band = PKTL_TRAN_TO_HOST(Band);
	offset += sizeof(Band);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s Type:%d Band:%d\n", __FUNCTION__, Type, Band));

	if(ATEOp->GetCfgOnOff){
#if defined(MT7615) || defined(MT7622)
		/*FW not support Get Rate power (Type=2)*/
		if((IS_MT7615(pAd) || IS_MT7622(pAd)) && Type != 2)
#endif
		{
			ATEOp->GetCfgOnOff(pAd, Type, Band, &Result);
		}
			Result = PKTL_TRAN_TO_HOST(Result);
			memcpy(HqaCmdFrame->Data + 2, &Result, 4);
			ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	}
	else {
		Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	}
	return Ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET3[] =
{	
	/* cmd id start from 0x1300 */
	HQA_MacBbpRegRead,		/* 0x1300 */
	HQA_MacBbpRegWrite,		/* 0x1301 */
	HQA_MACBbpRegBulkRead,	/* 0x1302 */
	HQA_RfRegBulkRead,		/* 0x1303 */
	HQA_RfRegBulkWrite,		/* 0x1304 */
	HQA_ReadEEPROM,			/* 0x1305 */
	HQA_WriteEEPROM,		/* 0x1306 */
	HQA_ReadBulkEEPROM,		/* 0x1307 */
	HQA_WriteBulkEEPROM,	/* 0x1308 */
#ifdef RTMP_EFUSE_SUPPORT
	HQA_CheckEfuseMode,		/* 0x1309 */
	HQA_GetFreeEfuseBlock,	/* 0x130A */
	HQA_GetEfuseBlockNr,	/* 0x130B */
	HQA_WriteEFuseFromBuffer, /* 0x130C */
#endif /* RTMP_EFUSE_SUPPORT */
	HQA_GetTxPower,			/* 0x130D */
	HQA_SetCfgOnOff,		/* 0x130E */
	HQA_GetFreqOffset,		/* 0x130F */
	HQA_DBDCTXTone,			/* 0x1310 */
	HQA_DBDCContinuousTX,	/* 0x1311 */
	HQA_SetRXFilterPktLen,	/* 0x1312 */
	HQA_GetTXInfo,			/* 0x1313 */
	HQA_GetCfgOnOff,		/* 0x1314 */
};

//1 todo not support yet
static INT32 HQA_ReadTempReferenceValue(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

VOID HQA_GetThermalValue_CB(struct cmd_msg *msg, char *Data, UINT16 Len)
{
	struct _EXT_EVENT_GET_SENSOR_RESULT_T *EventExtCmdResult = (struct _EXT_EVENT_GET_SENSOR_RESULT_T *)Data;
	RTMP_ADAPTER *pAd = (RTMP_ADAPTER *)msg->priv;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	EventExtCmdResult->u4SensorResult = le2cpu32(EventExtCmdResult->u4SensorResult);
    
	ATECtrl->thermal_val = EventExtCmdResult->u4SensorResult;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, value: 0x%x\n", __FUNCTION__, EventExtCmdResult->u4SensorResult));
#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)//1 todo windows no need RTMP_OS_COMPLETE
	RTMP_OS_COMPLETE(&ATECtrl->cmd_done);
#endif
}

static INT32 HQA_GetThermalValue(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	//CmdGetThemalSensorResult(pAd, 0); /* 0: get temperature; 1: get adc */
	MtCmdGetThermalSensorResult(pAd, 0, &ATECtrl->thermal_val); /* 0: get temperature; 1: get adc */
    	Value = PKTL_TRAN_TO_HOST(ATECtrl->thermal_val);	
	memcpy(HqaCmdFrame->Data + 2, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 6, Ret);
	return Ret;
}

//1 todo not support yet
static INT32 HQA_SetSideBandOption(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET4[] =
{
	/* cmd id start from 0x1400 */
	HQA_ReadTempReferenceValue, /* 0x1400 */
	HQA_GetThermalValue,		/* 0x1401 */
	HQA_SetSideBandOption,		/* 0x1402 */
};

static INT32 HQA_GetFWInfo(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
						
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
	if(ATEOp->GetFWInfo)
		Ret = ATEOp->GetFWInfo(pAd, HqaCmdFrame->Data + 2);
	else
		Ret = TM_STATUS_NOTSUPPORT;	

	ResponseToQA(HqaCmdFrame, WRQ, 38, Ret);
	return Ret;
}

static INT32 HQA_StartContinousTx(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0 , WFSel = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	/* TODO Get Correct TxfdMode*/
	UINT32 TxfdMode = 1;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    /* Modulation:4 BW:4 PRI_CH:4 RATE:4 WFSel:4 */
    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);
    ATECtrl->PhyMode = (UCHAR)Value;

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
    Value = PKTL_TRAN_TO_HOST(Value);
    ATECtrl->BW = (UCHAR)Value;

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 8, 4);
    Value = PKTL_TRAN_TO_HOST(Value);
    ATECtrl->ControlChl = (UCHAR)Value;

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 12, 4);
    Value = PKTL_TRAN_TO_HOST(Value);
    ATECtrl->Mcs = (UCHAR)Value;

    memcpy((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 16, 4);
    WFSel = PKTL_TRAN_TO_HOST(WFSel);
	if(ATEOp->StartContinousTx)
		ATEOp->StartContinousTx(pAd, (CHAR)WFSel, TxfdMode, TESTMODE_BAND0);
	else
		Ret = TM_STATUS_NOTSUPPORT;		

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetSTBC(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    INT32 Ret = 0;

	INT32 Value = 0;		
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    memcpy((UINT8 *)&Value, (UINT8 *)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);

    ATECtrl->Stbc = (UINT8)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_SetShortGI(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);

    ATECtrl->Sgi= (UCHAR)Value;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetDPD(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{

	INT32 Ret = 0;

	INT32 Value = 0, WFSel;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    /* ON/OFF:4 WF Sel:4 */
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);

	memcpy((PUCHAR)&WFSel, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	WFSel = PKTL_TRAN_TO_HOST(WFSel);
	if(ATEOp->SetDPD)
		ATEOp->SetDPD(pAd, (CHAR)WFSel, (CHAR)Value);
	else
		Ret = TM_STATUS_NOTSUPPORT;		

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#if defined(MT7615) || defined(MT7622)
static VOID HQA_ParseRxRssiCR(PRTMP_ADAPTER pAd, HQA_RX_STAT *HqaRxStat, INT type, UINT32 value, UINT32 Ch_Band)
{
	UINT32 IBRssi0, IBRssi1, WBRssi0, WBRssi1;
	/*[31:24]WBRSSI1 [23:16]IBRSSI1 [15:8]WBRSSI0 [7:0]IBRSSI0*/
    /*[31:24]WBRSSI3 [23:16]IBRSSI3 [15:8]WBRSSI2 [7:0]IBRSSI2*/	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Value : %02x\n", __FUNCTION__, value));

	IBRssi0 = (value&0xFF000000) >> 24;
	if (IBRssi0 >= 128)
		IBRssi0 -=256;
	WBRssi0 = (value&0x00FF0000) >> 16;
	if (WBRssi0 >= 128)
		WBRssi0 -=256;
	IBRssi1 = (value&0x0000FF00) >> 8;
	if (IBRssi1 >= 128)
		IBRssi1 -=256;
	WBRssi1 = (value&0x000000FF);
	if (WBRssi1 >= 128)
		WBRssi1 -=256;
	if(type == HQA_RX_STAT_RSSI){
		IBRssi0 = MtATERSSIOffset(pAd, IBRssi0, 0, Ch_Band);
		IBRssi1 = MtATERSSIOffset(pAd, IBRssi1, 1, Ch_Band);
		WBRssi0 = MtATERSSIOffset(pAd, WBRssi0, 0, Ch_Band);
		WBRssi1 = MtATERSSIOffset(pAd, WBRssi1, 1, Ch_Band);
		HqaRxStat->IB_RSSSI0 = PKTL_TRAN_TO_HOST(IBRssi0);
		HqaRxStat->WB_RSSSI0 = PKTL_TRAN_TO_HOST(WBRssi0);
		HqaRxStat->IB_RSSSI1 = PKTL_TRAN_TO_HOST(IBRssi1);
		HqaRxStat->WB_RSSSI1 = PKTL_TRAN_TO_HOST(WBRssi1);
		HqaRxStat->Inst_IB_RSSSI[0] = PKTL_TRAN_TO_HOST(IBRssi0);
		HqaRxStat->Inst_WB_RSSSI[0] = PKTL_TRAN_TO_HOST(WBRssi0);
		HqaRxStat->Inst_IB_RSSSI[1] = PKTL_TRAN_TO_HOST(IBRssi1);
		HqaRxStat->Inst_WB_RSSSI[1] = PKTL_TRAN_TO_HOST(WBRssi1);
	}else{
		IBRssi0 = MtATERSSIOffset(pAd, IBRssi0, 2, Ch_Band);
		IBRssi1 = MtATERSSIOffset(pAd, IBRssi1, 3, Ch_Band);
		WBRssi0 = MtATERSSIOffset(pAd, WBRssi0, 2, Ch_Band);
		WBRssi1 = MtATERSSIOffset(pAd, WBRssi1, 3, Ch_Band);
		HqaRxStat->Inst_IB_RSSSI[2] = PKTL_TRAN_TO_HOST(IBRssi0);
		HqaRxStat->Inst_WB_RSSSI[2] = PKTL_TRAN_TO_HOST(WBRssi0);
		HqaRxStat->Inst_IB_RSSSI[3] = PKTL_TRAN_TO_HOST(IBRssi1);
		HqaRxStat->Inst_WB_RSSSI[3] = PKTL_TRAN_TO_HOST(WBRssi1);
	}
}
#else
static VOID HQA_ParseRxRssiCR(PRTMP_ADAPTER pAd, HQA_RX_STAT *HqaRxStat, INT type, UINT32 value, UINT32 Ch_Band)
{
	UINT32 IBRssi0, IBRssi1, WBRssi0, WBRssi1;
	/*[31:24]IBRSSI0 [23:16]WBRSSI0 [15:8]IBRSSI1 [7:0]WBRSSI1*/
	IBRssi0 = (value&0xFF000000) >> 24;
	if (IBRssi0 > 128)
		IBRssi0 -=256;
	WBRssi0 = (value&0x00FF0000) >> 16;
	if (WBRssi0 > 128)
		WBRssi0 -=256;
	IBRssi1 = (value&0x0000FF00) >> 8;
	if (IBRssi1 > 128)
		IBRssi1 -=256;
	WBRssi1 = (value&0x000000FF);
	if (WBRssi1 > 128)
		WBRssi1 -=256;
	HqaRxStat->IB_RSSSI0 = PKTL_TRAN_TO_HOST(IBRssi0);
	HqaRxStat->WB_RSSSI0 = PKTL_TRAN_TO_HOST(WBRssi0);
	HqaRxStat->IB_RSSSI1 = PKTL_TRAN_TO_HOST(IBRssi1);
	HqaRxStat->WB_RSSSI1 = PKTL_TRAN_TO_HOST(WBRssi1);
}
#endif
static INT32 HQA_GetRxStatisticsAll(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;	
	UINT32 value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	HQA_RX_STAT HqaRxStat;
	UINT32 rx_driver_cnt = 0;
	INT i = 0;
	UINT32 Ch_Band_0 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND0, Ch_Band);
	UINT32 Ch_Band_1 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND0, Ch_Band);
	UCHAR FAGC_Path_0 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND0, FAGC_Path);
	UCHAR FAGC_Path_1 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND0, FAGC_Path);
	if (IS_ATE_DBDC(pAd)) {
		Ch_Band_1 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND1, Ch_Band);
		FAGC_Path_1 = TESTMODE_GET_PARAM(ATECtrl, TESTMODE_BAND1, FAGC_Path);
	}

	memset(&HqaRxStat, 0, sizeof(HQA_RX_STAT));
	/* MAC COUNT */
#if defined(MT7615) || defined(MT7637) || defined(MT7622)
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MACFCSERRCNT);
	ATECtrl->rx_stat.RxMacFCSErrCount = value;
	HqaRxStat.MacFCSErr = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacFCSErrCount);
#else
	/* Get latest FCSErr. sync iwpriv & HQA same FCSErr result */
	NICUpdateRawCounters(pAd);
	HqaRxStat.MacFCSErr = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacFCSErrCount);
#endif
 	value = MtAsicGetRxStat(pAd, HQA_RX_FIFO_FULL_COUNT);
	HqaRxStat.rx_fifo_full = PKTL_TRAN_TO_HOST(value);
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MAC_MDRDYCNT);
	ATECtrl->rx_stat.RxMacMdrdyCount = value;
	HqaRxStat.MacMdrdy = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacMdrdyCount);
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_FCSERRCNT);
	HqaRxStat.FCSErr_OFDM = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.FCSErr_CCK = PKTL_TRAN_TO_HOST(value & 0xFFFF);
#if defined(MT7615) || defined(MT7622)
 	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MAC_RXLENMISMATCH);
	HqaRxStat.RxLenMismatch = PKTL_TRAN_TO_HOST(value);
	if (IS_ATE_DBDC(pAd)){
		value = MtAsicGetRxStat(pAd, HQA_RX_FIFO_FULL_COUNT_BAND1);
		HqaRxStat.rx_fifo_full_band1 = PKTL_TRAN_TO_HOST(value);
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MACFCSERRCNT_BAND1);
		ATECtrl->rx_stat.RxMacFCSErrCount_band1 = value;
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MAC_MDRDYCNT_BAND1);
		ATECtrl->rx_stat.RxMacMdrdyCount_band1 = value;
		value = MtAsicGetRxStat(pAd, HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1);
		HqaRxStat.RxLenMismatch_band1 = PKTL_TRAN_TO_HOST(value);
		HqaRxStat.MacFCSErr_band1 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacFCSErrCount_band1);
		HqaRxStat.MacMdrdy_band1 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacMdrdyCount_band1);
	}
#endif
	
	/* PHY COUNT */
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD);
	HqaRxStat.OFDM_PD = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.CCK_PD = PKTL_TRAN_TO_HOST(value & 0xFFFF);
	
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD);
	HqaRxStat.CCK_SIG_Err = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.CCK_SFD_Err = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG);
	HqaRxStat.OFDM_SIG_Err = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.OFDM_TAG_Err = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI);
	HQA_ParseRxRssiCR(pAd, &HqaRxStat, HQA_RX_STAT_RSSI, value, Ch_Band_0);
#if defined(MT7615) || defined(MT7622)
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_RSSI_RX23);
	HQA_ParseRxRssiCR(pAd, &HqaRxStat, HQA_RX_STAT_RSSI_RX23, value, Ch_Band_1);
	
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITL);
	HqaRxStat.ACIHitLow = PKTL_TRAN_TO_HOST((value >> 18) & 0x1);
	
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_ACI_HITH);
	HqaRxStat.ACIHitHigh = PKTL_TRAN_TO_HOST((value >> 18) & 0x1);
#endif
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT);
	HqaRxStat.PhyMdrdyOFDM = PKTL_TRAN_TO_HOST(value >>16);
	HqaRxStat.PhyMdrdyCCK = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PHY_MDRDYCNT_BAND1);
	HqaRxStat.PhyMdrdyOFDM_band1 = PKTL_TRAN_TO_HOST(value >>16);
	HqaRxStat.PhyMdrdyCCK_band1 = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_PD_BAND1);
	HqaRxStat.OFDM_PD_band1 = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.CCK_PD_band1 = PKTL_TRAN_TO_HOST(value & 0xFFFF);
	
	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_CCK_SIG_SFD_BAND1);
	HqaRxStat.CCK_SIG_Err_band1 = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.CCK_SFD_Err_band1 = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	value = MtAsicGetRxStat(pAd, HQA_RX_STAT_OFDM_SIG_TAG_BAND1);
	HqaRxStat.OFDM_SIG_Err_band1 = PKTL_TRAN_TO_HOST(value >> 16);
	HqaRxStat.OFDM_TAG_Err_band1 = PKTL_TRAN_TO_HOST(value & 0xFFFF);

	/* DRIVER COUNT */
	rx_driver_cnt = ATECtrl->rx_stat.RxTotalCnt[0];
	HqaRxStat.DriverRxCount = PKTL_TRAN_TO_HOST(rx_driver_cnt);
#ifdef MT7615
#ifdef CFG_SUPPORT_MU_MIMO
	HqaRxStat.MuPktCount = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RxMacMuPktCount);
#endif
#endif
#ifdef MT7615
	rx_driver_cnt = 0;
	if (IS_ATE_DBDC(pAd))
		rx_driver_cnt = ATECtrl->rx_stat.RxTotalCnt[1];
	HqaRxStat.DriverRxCount1 = PKTL_TRAN_TO_HOST(rx_driver_cnt);
#endif
	/* RXV COUNT */
	for (i=0;i<4;i++) {
		if (i < 2)
			ATECtrl->rx_stat.FAGC_RSSI_IB[i] = MtATERSSIOffset(pAd, ATECtrl->rx_stat.FAGC_RSSI_IB[i], FAGC_Path_0, Ch_Band_0);
		else
			ATECtrl->rx_stat.FAGC_RSSI_IB[i] = MtATERSSIOffset(pAd, ATECtrl->rx_stat.FAGC_RSSI_IB[i], FAGC_Path_1, Ch_Band_1);
		HqaRxStat.FAGC_IB_RSSSI[i] = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.FAGC_RSSI_IB[i]);
	}
	for (i=0;i<4;i++) {
		if (i < 2)
			ATECtrl->rx_stat.FAGC_RSSI_WB[i] = MtATERSSIOffset(pAd, ATECtrl->rx_stat.FAGC_RSSI_WB[i], FAGC_Path_0, Ch_Band_0);
		else
			ATECtrl->rx_stat.FAGC_RSSI_WB[i] = MtATERSSIOffset(pAd, ATECtrl->rx_stat.FAGC_RSSI_WB[i], FAGC_Path_1, Ch_Band_1);
		HqaRxStat.FAGC_WB_RSSSI[i] = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.FAGC_RSSI_WB[i]);
	}
	HqaRxStat.RCPI0 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RCPI[0]);
	HqaRxStat.RCPI1 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RCPI[1]);
	HqaRxStat.FreqOffsetFromRX = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.FreqOffsetFromRx);
	HqaRxStat.RSSI0 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RSSI[0]);
	HqaRxStat.RSSI1 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RSSI[1]);
#ifdef MT7615
	HqaRxStat.RCPI2 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RCPI[2]);
	HqaRxStat.RCPI3 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RCPI[3]);
	HqaRxStat.RSSI2 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RSSI[2]);
	HqaRxStat.RSSI3 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RSSI[3]);
	HqaRxStat.SNR0 = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SNR[0]);
	HqaRxStat.SIG_MCS = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SIG_MCS);
	HqaRxStat.SINR = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SINR);
	HqaRxStat.RXVRSSI = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.RXVRSSI);
#endif
	{
			for (i=0;i<4;i++)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("Report[0]: RCPI[%d]:%x\n",
					i, ATECtrl->rx_stat.RCPI[i]));
			for (i=0;i<4;i++)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("Report[1]: FAGC_RSSI_IB[%d]:%x\n",
					i, ATECtrl->rx_stat.FAGC_RSSI_IB[i]));
			for (i=0;i<4;i++)
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
					("Report[2]: FAGC_RSSI_WB[%d]:%x\n",
					i, ATECtrl->rx_stat.FAGC_RSSI_WB[i]));
	}
	memcpy(HqaCmdFrame->Data + 2, &(HqaRxStat), sizeof(HQA_RX_STAT));
	ResponseToQA(HqaCmdFrame, WRQ, (2+sizeof(HQA_RX_STAT)), Ret);

	return Ret;
}


static INT32 HQA_StartContiTxTone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);

	if(ATEOp->StartTxTone)
		ATEOp->StartTxTone(pAd, Value); 
	else
		Ret = TM_STATUS_NOTSUPPORT;	

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_StopContiTxTone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	//MtAsicSetTxToneTest(pAd, 0, 0);
	if(ATEOp->StopTxTone)
		ATEOp->StopTxTone(pAd); 
	else
		Ret = TM_STATUS_NOTSUPPORT;	

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_CalibrationTestMode(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Value = 0;
	INT32 Ret = 0;
	UINT8  Mode = 0;
	INT32 ICaplen = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	Value = PKTL_TRAN_TO_HOST(Value);
	NdisMoveMemory((PUCHAR)&ICaplen, (PUCHAR)&HqaCmdFrame->Data+4, 4);
	ICaplen = PKTL_TRAN_TO_HOST(ICaplen);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s mode = %X ICapLen= %X\n", __FUNCTION__, Value, ICaplen));
	if (Value == 0) {
		if (ATECtrl->Mode & ATE_FFT)
			ATECtrl->Mode &= ~ATE_FFT;
		ATECtrl->Mode &= ~fATE_IN_RFTEST;
		Mode = OPERATION_NORMAL_MODE;
	} else if (Value == 1) {
		ATECtrl->Mode |= fATE_IN_RFTEST;
		Mode = OPERATION_RFTEST_MODE;
	} else if (Value == 2) {
		ATECtrl->Mode |= fATE_IN_RFTEST;
		Mode = OPERATION_ICAP_MODE;
	} else if (Value == 3) {
		ATECtrl->Mode |= fATE_IN_RFTEST;
		ATECtrl->Mode |= fATE_IN_ICAPOVERLAP;
		Mode = OPERATION_ICAP_OVERLAP;
	} else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Mode = %d error!!!\n",__FUNCTION__, Value));

	MtCmdRfTestSwitchMode(pAd, Mode, ICaplen, RF_TEST_DEFAULT_RESP_LEN);
	msleep(100);
	RcUpdateBandCtrl((HD_CFG*)pAd->pHdevCfg);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_DoCalibrationTestItem(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 item = 0;
	UINT32 band_idx = MT_ATEGetBandIdxByIf(pAd);
	UCHAR *data = HqaCmdFrame->Data;
	
	NdisMoveMemory((UCHAR *)&item, data, sizeof(item));
	data += sizeof(item);
	item = PKTL_TRAN_TO_HOST(item);
	NdisMoveMemory((UCHAR *)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
#if defined (COMPOS_TESTMODE_WIN)
	CreateThread(pAd);
#endif
	MtCmdDoCalibration(pAd, RE_CALIBRATION, item, band_idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, item:%x, band_idx:%x\n"
			, __FUNCTION__, item, band_idx));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);

	return Ret;
}

static INT32 HQA_eFusePhysicalWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFusePhysicalRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFuseLogicalRead(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_eFuseLogicalWrite(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));
    /* ToDo */
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;
}

static INT32 HQA_TMRSetting(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 value = 0, version = 0;
	CHAR TMR_Value[8];
    CHAR TMR_HW_Version[8];

	EthGetParamAndShiftBuff(TRUE, sizeof(value), &data, (UCHAR *)&value);
    EthGetParamAndShiftBuff(TRUE, sizeof(version), &data, (UCHAR *)&version);
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, TMR setting: %u, TMR version: %u\n", __FUNCTION__, value, version));

    if (version == TMR_HW_VER_100)
        version = TMR_VER_1_0;
    else if (version == TMR_HW_VER_150)
        version = TMR_VER_1_5;
    else if (version == TMR_HW_VER_200)
        version = TMR_VER_2_0;
    else
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: wrong version %d!!\n", __FUNCTION__, version));
        return FALSE;
    }
	sprintf(TMR_Value, "%d", value);
    sprintf(TMR_HW_Version, "%d", version);

    setTmrVerProc(pAd, TMR_HW_Version);
	setTmrEnableProc(pAd, TMR_Value);
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GetRxSNR(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;

	INT32 Value = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

   	Value = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SNR[0]);
    memcpy(HqaCmdFrame->Data + 2, &Value, 4);

    Value = PKTL_TRAN_TO_HOST(ATECtrl->rx_stat.SNR[1]);
    memcpy(HqaCmdFrame->Data + 6, &Value, 4);

	ResponseToQA(HqaCmdFrame, WRQ, 10, Ret);
    return Ret;
}


static INT32 HQA_WriteBufferDone(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);	
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN) //1Todo	Set_EepromBufferWriteBack_Proc
	switch (Value)
	{
		case E2P_EFUSE_MODE:
            /* update status of Effuse write back */
            pAd->fgQAEffuseWriteBack = TRUE;
			Set_EepromBufferWriteBack_Proc(pAd, "1");
			break;
		case E2P_FLASH_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "2");
			break;
		case E2P_EEPROM_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "3");
			break;
		case E2P_BIN_MODE:
			Set_EepromBufferWriteBack_Proc(pAd, "4");
			break;
		default:
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Unknow write back mode(%d)\n", __FUNCTION__, Value));
	}
#endif	

    /* update status of Effuse write back */
    pAd->fgQAEffuseWriteBack = FALSE;

    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
    return Ret;

}


static INT32 HQA_FFT(
				PRTMP_ADAPTER pAd,
				RTMP_IOCTL_INPUT_STRUCT *WRQ,
				HQA_CMD_FRAME *HqaCmdFrame)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Value = 0;
	INT32 Ret = 0;

    NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
    Value = PKTL_TRAN_TO_HOST(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: %d\n", __FUNCTION__, Value));
	Ret = ATEOp->SetFFTMode(pAd, Value);

	return Ret;
}

static INT32 HQA_SetTxTonePower(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;
	INT32 pwr1 = 0;
	INT32 pwr2 = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	pwr1 = PKTL_TRAN_TO_HOST(Value);

	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	pwr2 = PKTL_TRAN_TO_HOST(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, pwr1:%d, pwr2:%d \n", __FUNCTION__,pwr1,pwr2));
	if(ATEOp->SetTxTonePower)
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_SetAIFS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Value = 0;	
	UINT32 SlotTime = 0;
	UINT32 SifsTime = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data, 4);
	SlotTime = PKTL_TRAN_TO_HOST(Value);
	memcpy((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	SifsTime = PKTL_TRAN_TO_HOST(Value);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, SlotTime:%d, SifsTime:%d \n", __FUNCTION__,SlotTime,SifsTime));
	if(ATEOp->SetSlotTime)
		ATEOp->SetSlotTime(pAd, SlotTime, SifsTime, 0);
	else
		Ret = TM_STATUS_NOTSUPPORT;	
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetSeqData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length)/sizeof(UINT32)- 1;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>512) || (len == 0))
		goto MPS_SEQ_DATA_RET;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_SEQ_DATA_RET;
	
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_SEQDATA, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

MPS_SEQ_DATA_RET:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPayloadLength(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;

    len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_PKT_LEN_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_LEN_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_PAYLOAD_LEN, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

MPS_PKT_LEN_RET:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPacketCount(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;

    len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length)/4 - 1;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_PKT_CNT_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_PKT_CNT_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_TX_COUNT, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

MPS_PKT_CNT_RET:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPowerGain(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;

	len = PKTS_TRAN_TO_NET(HqaCmdFrame->Length)/4 - 1;
		
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>1024) || (len == 0))
		goto MPS_SET_PWR_RET;
	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto MPS_SET_PWR_RET;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	
	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
    		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}
	
	ATEOp->MPSSetParm(pAd, MPS_PWR_GAIN, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

MPS_SET_PWR_RET:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSStart(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT32 band_idx = 0;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::\n", __FUNCTION__));

	Ret = ATEOp->MPSTxStart(pAd, band_idx);
	if(WRQ)
		ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSStop(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 band_idx = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s\n", __FUNCTION__));
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	Ret = ATEOp->MPSTxStop(pAd, band_idx);
	if(WRQ)
	    ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetNss(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 offset = 0;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length)/sizeof(UINT32)- 1;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>512) || (len == 0))
		goto out;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto out;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);
		mps_setting[i] = PKTL_TRAN_TO_HOST(Value);
	}

	ATEOp->MPSSetParm(pAd, MPS_NSS, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

out:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MPSSetPerpacketBW(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Value = 0;
	UINT32 len = 0;
	INT32 Ret = 0;
	UINT32 i = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 *mps_setting = NULL;
	UINT32 band_idx = 0;
	UINT32 per_pkt_bw = 0;
	UINT32 offset = 0;

	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length)/sizeof(UINT32)- 1;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s::len:%u\n", __FUNCTION__, len));
	if((len>512) || (len == 0))
		goto out;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(len));
	if(Ret == NDIS_STATUS_FAILURE)
		goto out;

	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	band_idx = OS_NTOHL(band_idx);

	for(i=0;i<len;i++){
		offset = 4 + 4 * i;
		if(offset + 4 > sizeof(HqaCmdFrame->Data)) /* Reserved at least 4 byte availbale data */
			break;
		NdisMoveMemory((PUCHAR)&Value, (PUCHAR)&HqaCmdFrame->Data + 4 + 4*i, 4);

		per_pkt_bw = PKTL_TRAN_TO_HOST(Value);

		switch(per_pkt_bw){
		case ATE_BAND_WIDTH_20:
			per_pkt_bw = BAND_WIDTH_20;
			break;
		case ATE_BAND_WIDTH_40:
			per_pkt_bw = BAND_WIDTH_40;
			break;
		case ATE_BAND_WIDTH_80:
			per_pkt_bw = BAND_WIDTH_80;
			break;
		case ATE_BAND_WIDTH_10:
			per_pkt_bw = BAND_WIDTH_10;
			break;
		case ATE_BAND_WIDTH_5:
			per_pkt_bw = BAND_WIDTH_5;
			break;
		case ATE_BAND_WIDTH_160:
		case ATE_BAND_WIDTH_8080:
			per_pkt_bw = BAND_WIDTH_160;
			break;
		default:
			per_pkt_bw = BAND_WIDTH_20;
			break;
		}

		mps_setting[i] = per_pkt_bw;
	}

	ATEOp->MPSSetParm(pAd, MPS_PKT_BW, band_idx, len, mps_setting);
	os_free_mem(mps_setting);

out:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, len:%u, MPS_CNT:%u\n", __FUNCTION__,len, ATECtrl->mps_cb.mps_cnt));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_CheckEfuseModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, %x\n", __FUNCTION__, pAd->E2pCtrl.e2pCurMode));
	val = PKTL_TRAN_TO_NET(pAd->E2pCtrl.e2pCurMode); 	//Fix me::pAd unify
#endif
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(val), Ret);
	return Ret;
}

static INT32 HQA_CheckEfuseNativeModeType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, %x\n", __FUNCTION__, pAd->E2pAccessMode));
	val = PKTL_TRAN_TO_NET(pAd->E2pAccessMode);	//Fix me::pAd unify
#endif
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(val), Ret);
	return Ret;
}

static INT32 HQA_SetBandMode(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 band_mode = 0;
	UINT32 band_type = 0;
	
	NdisMoveMemory((UCHAR *)&band_mode, data, sizeof(band_mode));
	data += sizeof(band_mode);
	band_mode = PKTL_TRAN_TO_HOST(band_mode);
	NdisMoveMemory((UCHAR *)&band_type, data, sizeof(band_type));
	data += sizeof(band_type);
	band_type = PKTL_TRAN_TO_HOST(band_type);

#ifndef COMPOS_TESTMODE_WIN
	// todo: windows do not have Set_WirelessMode_Proc function
	if (band_mode == ATE_SINGLE_BAND) {
		if (band_type == ATE_ABAND_TYPE)
			Set_WirelessMode_Proc(pAd, "14");
		else if (band_type == ATE_GBAND_TYPE)
			Set_WirelessMode_Proc(pAd, "9");
		else
			Ret = -1;
	}
#endif


	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, band_mode:%x, band_type:%x\n"
			, __FUNCTION__, band_mode, band_type));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_GetBandMode(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{

	INT32 Ret = 0;
	UINT32 band_mode = 0;
#ifndef COMPOS_TESTMODE_WIN
	UINT32 band_idx = 0;
	UINT32 is_dbdc = IS_ATE_DBDC(pAd);
	struct wifi_dev *wdev = NULL;
	UINT8 wdev_idx = 0;
	
	NdisMoveMemory((UCHAR *)&band_idx, HqaCmdFrame->Data, sizeof(band_idx));	
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	wdev_idx = MT_ATEGetWDevIdxByBand(pAd, band_idx);
	if (wdev_idx != -1)
		wdev = pAd->wdev_list[wdev_idx];
	if (!wdev)
		goto resp;

	if (is_dbdc) {
		band_mode = wdev->PhyMode;
		band_mode = WMODE_CAP_5G(band_mode)?2:1;
	}
	else
		band_mode = 3; /* Always report 2.4 + 5G*/
	band_mode = PKTL_TRAN_TO_HOST(band_mode);
	resp:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, %x,band_mode:%x, band_idx:%x\n"
			, __FUNCTION__, is_dbdc, band_mode, band_idx));
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(band_mode), sizeof(band_mode));
#endif
	ResponseToQA(HqaCmdFrame, WRQ, 2+sizeof(band_mode), Ret);

	return Ret;
}


static INT32 HQA_RDDStartExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 rdd_num = 0;
	UINT32 rdd_in_sel = 0;
	INT32 Ret = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((PUCHAR)&rdd_num, data, sizeof(rdd_num));
	rdd_num = PKTL_TRAN_TO_HOST(rdd_num);
	data += sizeof(rdd_num);
	NdisMoveMemory((PUCHAR)&rdd_in_sel, data, sizeof(rdd_in_sel));
	rdd_in_sel = PKTL_TRAN_TO_HOST(rdd_in_sel);

	Ret = ate_ops->onOffRDD(pAd, rdd_num, rdd_in_sel, 1);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, %x\n", __FUNCTION__, rdd_num));
	return Ret;
}

static INT32 HQA_RDDStopExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 rdd_num = 0;
    UINT32 rdd_in_sel = 0;
	INT32 Ret = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	NdisMoveMemory((PUCHAR)&rdd_num, data, sizeof(rdd_num));
	rdd_num = PKTL_TRAN_TO_HOST(rdd_num);
	data += sizeof(rdd_num);
	NdisMoveMemory((PUCHAR)&rdd_in_sel, data, sizeof(rdd_in_sel));
	rdd_in_sel = PKTL_TRAN_TO_HOST(rdd_in_sel);

	Ret = ate_ops->onOffRDD(pAd, rdd_num, rdd_in_sel, 0);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, %x\n", __FUNCTION__, rdd_num));
	return Ret;
}

static INT32 HQA_BssInfoUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 OwnMacIdx = 0, BssIdx = 0;
	UCHAR Bssid[MAC_ADDR_LEN];
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(OwnMacIdx), &data, (UCHAR *)&OwnMacIdx);
	EthGetParamAndShiftBuff(TRUE, sizeof(BssIdx), &data, (UCHAR *)&BssIdx);
	EthGetParamAndShiftBuff(FALSE, sizeof(Bssid), &data, Bssid);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, BssIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n", 
		 __FUNCTION__, BssIdx, PRINT_MAC(Bssid)));

	Ret = ATEOp->BssInfoUpdate(pAd, OwnMacIdx, BssIdx, Bssid);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_DevInfoUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Band = 0, OwnMacIdx = 0;
	UCHAR Bssid[MAC_ADDR_LEN];
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(Band), &data, (UCHAR *)&Band);
	EthGetParamAndShiftBuff(TRUE, sizeof(OwnMacIdx), &data, (UCHAR *)&OwnMacIdx);
	EthGetParamAndShiftBuff(FALSE, sizeof(Bssid), &data, Bssid);
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Band:%d OwnMacIdx:%d Bssid:%02x:%02x:%02x:%02x:%02x:%02x\n", 
		 __FUNCTION__, Band, OwnMacIdx, PRINT_MAC(Bssid)));

	Ret = ATEOp->DevInfoUpdate(pAd, Band, OwnMacIdx, Bssid);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_LogOnOff(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 band_idx = 0;
	UINT32 log_type = 0;
	UINT32 log_ctrl = 0;
	UINT32 log_size = 200;
	UCHAR *data = HqaCmdFrame->Data;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	
	NdisMoveMemory((PUCHAR)&band_idx, data, sizeof(band_idx));
	data += sizeof(band_idx);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);

	NdisMoveMemory((PUCHAR)&log_type, data, sizeof(log_type));
	data += sizeof(log_type);
	log_type = PKTL_TRAN_TO_HOST(log_type);

	NdisMoveMemory((PUCHAR)&log_ctrl, data, sizeof(log_ctrl));
	data += sizeof(log_ctrl);
	log_ctrl = PKTL_TRAN_TO_HOST(log_ctrl);
	
	NdisMoveMemory((PUCHAR)&log_size, data, sizeof(log_size));
	data += sizeof(log_size);
	log_size = PKTL_TRAN_TO_HOST(log_size);

	Ret = ATEOp->LogOnOff(pAd, log_type, log_ctrl, log_size);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, band_idx:%x, log_type:%x, log_ctrl:%x, en_log:%x, log_size:%u\n"
			, __FUNCTION__, band_idx, log_type, log_ctrl, ATECtrl->en_log, log_size));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static INT32 HQA_HIFTestSetStartLoopback(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 len = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	HQA_HIFTestParam tmp, *param;
	LOOPBACK_SETTING set;
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((UCHAR *)&tmp, HqaCmdFrame->Data, sizeof(tmp));
	param = PKTLA_TRAN_TO_HOST(len/4, &tmp);
	set.StartLen = param->start_len;
	set.StopLen = param->stop_len;
	set.RepeatTimes = param->repeat_time;
	set.IsDefaultPattern = param->is_def_pattern;
	set.BulkOutNumber = param->bulkout_num;
	set.BulkInNumber = param->bulkin_num;
	set.TxAggNumber = param->txagg_num;
	set.RxAggPktLmt = param->rxagg_limit;
	set.RxAggLmt = param->rxagg_lm;
	set.RxAggTO = param->rxagg_to;
	set.RxAggEnable = param->enable_rxagg;
	ate_ctrl->verify_mode = ATE_LOOPBACK;
	if(!pAd->LbCtrl.LoopBackRunning)
		LoopBack_Start(pAd, &set);
	else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s, LB is running\n", __FUNCTION__));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s: start_len:%u, stop_len:%u, repeat:%u, is_default:%u, bulkout_num:%u, bulkin_num:%u, txagg_num:%u, rxagg_limit:%u, rxagg_lm:%u, rxagg_to:%u, enable_rxagg:%u\n", __FUNCTION__, param->start_len,param->stop_len,param->repeat_time,param->is_def_pattern,param->bulkout_num,param->bulkin_num,param->txagg_num,param->rxagg_limit,param->rxagg_lm,param->rxagg_to,param->enable_rxagg));
	return Ret;
}

static INT32 HQA_HIFTestSetStopLoopback(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:\n", __FUNCTION__));
	if(pAd->LbCtrl.LoopBackRunning)
		LoopBack_Stop(pAd);
	pAd->LbCtrl.DebugMode = FALSE;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_HIFTestGetStatus(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	LOOPBACK_RESULT tmp, *resp;
	LoopBack_Status(pAd, &tmp);
	resp = PKTLA_TRAN_TO_NET(sizeof(tmp)/4, &tmp);
	NdisMoveMemory(HqaCmdFrame->Data + 2,(UCHAR *)resp, sizeof(tmp));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(tmp), Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:\n", __FUNCTION__));
	return Ret;
}

static INT32 HQA_HIFTestSetTxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 len = 0;
	UINT32 tx_len = 0;
	UCHAR *raw = NULL;
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory(&tx_len, HqaCmdFrame->Data, sizeof(tx_len));
	tx_len = PKTL_TRAN_TO_HOST(tx_len);
	raw = HqaCmdFrame->Data + sizeof(UINT32);
	if(pAd->LbCtrl.DebugMode)
	{
		UINT32 j = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, TxExpect Dump(%u): ", __FUNCTION__, tx_len));
		for(j=0;j<tx_len;j++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%02x",raw[j]));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n"));
	}
	LoopBack_ExpectTx(pAd, tx_len, raw);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s::len:%u, tx_len:%u\n", __FUNCTION__, len, tx_len));
	return Ret;
}

static INT32 HQA_HIFTestSetRxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 rx_len = 0;
	UCHAR *raw = NULL;
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory(&rx_len, HqaCmdFrame->Data, sizeof(rx_len));
	rx_len = PKTL_TRAN_TO_HOST(rx_len);
	raw = HqaCmdFrame->Data + sizeof(UINT32);
	if(pAd->LbCtrl.DebugMode)
	{
		UINT32 j = 0;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, RxExpect Dump(%u): ", __FUNCTION__, rx_len));
		for(j=0;j<rx_len;j++)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%02x",raw[j]));
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n"));
	}
	LoopBack_ExpectRx(pAd, rx_len, raw);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s::len:%u, rx_len:%u\n", __FUNCTION__, len, rx_len));
	return Ret;
}

static INT32 HQA_HIFTestGetTxRxData(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UCHAR *out = HqaCmdFrame->Data + 2;
	//TODO:: Need check ated allocate size
	UINT32 tlen = 0;
	UINT32 out_len = 0;
	UINT32 tmp = 0;
	/* Tx Data */
	LoopBack_RawData(pAd, &out_len, TRUE, out + sizeof(UINT32));
	tmp = PKTL_TRAN_TO_NET(out_len);
	NdisMoveMemory(out, &tmp, sizeof(UINT32));
	tlen += out_len;
	out += out_len + sizeof(UINT32);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s::tx_out_len:%u\n", __FUNCTION__,out_len));
	/* Rx Data */
	LoopBack_RawData(pAd, &out_len, FALSE, out + sizeof(UINT32));
	tmp = PKTL_TRAN_TO_NET(out_len);
	NdisMoveMemory(out, &tmp, sizeof(UINT32));
	tlen += out_len;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s::rx_out_len:%u, total_len:%u\n", __FUNCTION__,out_len, tlen));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + tlen + sizeof(UINT32)*2, Ret);
	return Ret;
}

static INT32 HQA_UDMAAction(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:\n", __FUNCTION__));
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_WIFIPowerOff(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
    UINT32 val = 0;
	INT32 Ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s:\n", __FUNCTION__));
	NdisMoveMemory(HqaCmdFrame->Data + 2, &(val), sizeof(val));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}



static INT32 ToDoFunction(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_CMD_SET5[] =
{
	/* cmd id start from 0x1500 */
	HQA_GetFWInfo,				/* 0x1500 */
	HQA_StartContinousTx,		/* 0x1501 */	
	HQA_SetSTBC,				/* 0x1502 */	
	HQA_SetShortGI,				/* 0x1503 */
	HQA_SetDPD,					/* 0x1504 */
	HQA_SetTssiOnOff,			/* 0x1505 */
	HQA_GetRxStatisticsAll,		/* 0x1506 */
	HQA_StartContiTxTone,		/* 0x1507 */
	HQA_StopContiTxTone,        /* 0x1508 */
	HQA_CalibrationTestMode,    /* 0x1509 */
	HQA_DoCalibrationTestItem,  /* 0x150A */
	HQA_eFusePhysicalWrite,     /* 0x150B */
	HQA_eFusePhysicalRead,      /* 0x150C */
	HQA_eFuseLogicalRead,       /* 0x150D */
	HQA_eFuseLogicalWrite,      /* 0x150E */
	HQA_TMRSetting,             /* 0x150F */
	HQA_GetRxSNR,               /* 0x1510 */
	HQA_WriteBufferDone,		/* 0x1511 */
	HQA_FFT,					/* 0x1512 */
	HQA_SetTxTonePower,			/* 0x1513 */
	HQA_GetChipID,				/* 0x1514 */
	HQA_MPSSetSeqData,			/* 0x1515 */
	HQA_MPSSetPayloadLength,	/* 0x1516 */
	HQA_MPSSetPacketCount,		/* 0x1517 */
	HQA_MPSSetPowerGain,		/* 0x1518 */
	HQA_MPSStart,				/* 0x1519 */
	HQA_MPSStop,				/* 0x151A */
	ToDoFunction,				/* 0x151B */	
	ToDoFunction,				/* 0x151C */	
	ToDoFunction,				/* 0x151D */	
	ToDoFunction,				/* 0x151E */	
	ToDoFunction,				/* 0x151F */	
	ToDoFunction,				/* 0x1520 */	
	HQA_SetAIFS,				/* 0x1521 */
	HQA_CheckEfuseModeType,		/* 0x1522 */
	HQA_CheckEfuseNativeModeType,/* 0x1523 */
 	HQA_HIFTestSetStartLoopback,/* 0x1524 */
 	HQA_HIFTestSetStopLoopback,	/* 0x1525 */
 	HQA_HIFTestGetStatus,		/* 0x1526 */
 	HQA_HIFTestSetTxData,		/* 0x1527 */
 	HQA_HIFTestSetRxData,		/* 0x1528 */
 	HQA_HIFTestGetTxRxData,		/* 0x1529 */
 	HQA_UDMAAction,				/* 0x152A */
 	HQA_WIFIPowerOff,			/* 0x152B */
	HQA_SetBandMode,			/* 0x152C */	
	HQA_GetBandMode,			/* 0x152D */
 	HQA_RDDStartExt,			/* 0x152E */
 	HQA_RDDStopExt,				/* 0x152F */
	ToDoFunction,				/* 0x1530 */
 	HQA_BssInfoUpdate,			/* 0x1531 */
 	HQA_DevInfoUpdate,			/* 0x1532 */
 	HQA_LogOnOff,				/* 0x1533 */
	ToDoFunction,				/* 0x1534 */
	ToDoFunction,				/* 0x1535 */
	HQA_MPSSetNss,				/* 0x1536 */
	HQA_MPSSetPerpacketBW,			/* 0x1537 */
};

#ifdef TXBF_SUPPORT
#if defined(MT7615) || defined(MT7637)
VOID HQA_BF_INFO_CB(RTMP_ADAPTER *pAd, unsigned char *data, UINT32 len)
{
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	struct _EXT_EVENT_BF_STATUS_T *bf_info = (struct _EXT_EVENT_BF_STATUS_T *)data;
	struct _EXT_EVENT_IBF_STATUS_T *ibf_info = (struct _EXT_EVENT_IBF_STATUS_T *)data;
	UINT32 status = 0;
	UINT32 data_len = 0;
	UCHAR *bf_data = bf_info->aucBuffer;

	if(!(ate_ctrl->Mode & fATE_IN_BF))
		return;

	ate_ctrl->txbf_info_len = 0;
	os_alloc_mem(pAd, (UCHAR **)&ate_ctrl->txbf_info, sizeof(UCHAR)*len);
	if (!ate_ctrl->txbf_info){
		status = NDIS_STATUS_RESOURCES;
		goto err0;
	}
	os_zero_mem(ate_ctrl->txbf_info, sizeof(UCHAR)*len);
	ate_ctrl->txbf_info_len = len;

	switch (bf_info->ucBfDataFormatID)
	{
	case BF_PFMU_TAG:
		if (bf_info->fgBFer)
			data_len = sizeof(PFMU_PROFILE_TAG1) + sizeof(PFMU_PROFILE_TAG2);
		else
			data_len = sizeof(PFMU_PROFILE_TAG1);
		NdisMoveMemory(ate_ctrl->txbf_info, bf_data, data_len);
		ate_ctrl->txbf_info_len = data_len;
		break;
	case BF_PFMU_DATA:
		NdisMoveMemory(ate_ctrl->txbf_info, bf_data, sizeof(PFMU_DATA));
		data_len = sizeof(PFMU_DATA);
		ate_ctrl->txbf_info_len = data_len;
		break;
    case BF_CAL_PHASE:
        ate_ctrl->iBFCalStatus = ibf_info->ucStatus;
	default:
		break;
	}
	err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,
		("%s(%x)\n", __FUNCTION__, status));
	RTMP_OS_COMPLETE(&ate_ctrl->cmd_done);
}
#else
VOID HQA_BF_INFO_CB(RTMP_ADAPTER *pAd, unsigned char *data, UINT32 len)
{
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
		("%s not supported yet\n", __FUNCTION__));
}
#endif /* MT7615 */

static INT32 HQA_TxBfProfileTagInValid(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 invalid = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_INVALID_FAIL;
	}

	NdisMoveMemory((PUCHAR)&invalid, (PUCHAR)&HqaCmdFrame->Data, sizeof(invalid));
	invalid = PKTL_TRAN_TO_HOST(invalid);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%u", invalid);
#ifdef MT7615
	Set_TxBfProfileTag_InValid(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%u, str:%s\n", __FUNCTION__, invalid, cmd));
	os_free_mem(cmd);
BF_PROFILE_TAG_INVALID_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagPfmuIdx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 pfmuidx = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_PFMU_FAIL;
	}
	NdisMoveMemory((PUCHAR)&pfmuidx, (PUCHAR)&HqaCmdFrame->Data, sizeof(pfmuidx));
	pfmuidx = PKTL_TRAN_TO_HOST(pfmuidx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%u", pfmuidx);
#ifdef MT7615
	Set_TxBfProfileTag_PfmuIdx(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%u, str:%s\n", __FUNCTION__, pfmuidx, cmd));
	os_free_mem(cmd);
BF_PROFILE_TAG_PFMU_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagBfType(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 bftype = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_BFTYPE_FAIL;
	}
	NdisMoveMemory((PUCHAR)&bftype, (PUCHAR)&HqaCmdFrame->Data, 4);
	bftype = PKTL_TRAN_TO_HOST(bftype);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%u", bftype);
#ifdef MT7615
	Set_TxBfProfileTag_BfType(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%u, str:%s\n", __FUNCTION__, bftype, cmd));
	os_free_mem(cmd);
BF_PROFILE_TAG_BFTYPE_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagBw(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 tag_bw = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_BW_FAIL;
	}
	NdisMoveMemory((PUCHAR)&tag_bw, (PUCHAR)&HqaCmdFrame->Data, 4);
	tag_bw = PKTL_TRAN_TO_HOST(tag_bw);
 	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%u", tag_bw);
#ifdef MT7615
	Set_TxBfProfileTag_DBW(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%u, str:%s\n", __FUNCTION__, tag_bw, cmd));
	os_free_mem(cmd);
	BF_PROFILE_TAG_BW_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagSuMu(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 su_mu = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_TAG_SUMU_FAIL;
	}
	NdisMoveMemory((PUCHAR)&su_mu, (PUCHAR)&HqaCmdFrame->Data, 4);
	su_mu = PKTL_TRAN_TO_HOST(su_mu);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%u", su_mu);
#ifdef MT7615
	Set_TxBfProfileTag_SuMu(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%u, str:%s\n", __FUNCTION__, su_mu, cmd));
	os_free_mem(cmd);
BF_PROFILE_TAG_SUMU_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagMemAlloc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_TAG_ALLOC tmp, *layout;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MEMALLOC_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	if(len > sizeof(tmp)){
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_TAG_MEMALLOC_FAIL;
	}
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	layout = PKTLA_TRAN_TO_HOST(len/4, &tmp);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",layout->col_idx0,layout->row_idx0,layout->col_idx1,layout->row_idx1,layout->col_idx2,layout->row_idx2,layout->col_idx3,layout->row_idx3);

#ifdef MT7615
	Set_TxBfProfileTag_Mem(pAd, cmd);
#endif
HQA_TAG_MEMALLOC_FAIL:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, col0/row0:%x/%x, col1/row1:%x/%x, col2/row2:%x/%x, col3/row3:%x/%x\n", __FUNCTION__,layout->col_idx0,layout->row_idx0,layout->col_idx1,layout->row_idx1,layout->col_idx2,layout->row_idx2,layout->col_idx3,layout->row_idx3));
	return Ret;
}

static INT32 HQA_TxBfProfileTagMatrix(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_TAG_MATRIX tmp, *matrix;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MATRIX_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	matrix = PKTLA_TRAN_TO_HOST(len/4, &tmp);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x",matrix->nrow, matrix->ncol, matrix->ngroup, matrix->LM, matrix->code_book, matrix->htc_exist);

#ifdef MT7615
	Set_TxBfProfileTag_Matrix(pAd, cmd);
#endif
	os_free_mem(cmd);
HQA_TAG_MATRIX_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, nrow:%x, ncol:%x, ngroup:%x, LM:%x, code_book:%x, htc:%x\n", __FUNCTION__, matrix->nrow, matrix->ncol, matrix->ngroup, matrix->LM, matrix->code_book, matrix->htc_exist));
	return Ret;
}

static INT32 HQA_TxBfProfileTagSnr(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_TAG_SNR tmp, *snr;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SNR_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	snr = PKTLA_TRAN_TO_HOST(len/4, &tmp);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x" ,snr->snr_sts0, snr->snr_sts1, snr->snr_sts2, snr->snr_sts3);
#ifdef MT7615
	Set_TxBfProfileTag_SNR(pAd, cmd);
#endif
	os_free_mem(cmd);
HQA_TAG_SNR_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, snr_sts0:%x, snr_sts1:%x, snr_sts2:%x, snr_sts3:%x,\n", __FUNCTION__, snr->snr_sts0, snr->snr_sts1, snr->snr_sts2, snr->snr_sts3));
	return Ret;
}

static INT32 HQA_TxBfProfileTagSmtAnt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 smt_ant = 0;
	RTMP_STRING *cmd;

	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SMTANT_FAIL;
	}
	NdisMoveMemory((PUCHAR)&smt_ant, (PUCHAR)&HqaCmdFrame->Data, 4);
	/*
	 * DBDC mode:
	 * If Use Band 0, set [11:6]=0x0, set [5:0] as Ant cfg.
	 * If Use Band 1, set [11:6] as Ant cfg, set [5:0] = 0x0.
	 * Non-DBDC mode:
	 * Set [11:0] as Ant cfg. 
	 * */
	smt_ant = PKTL_TRAN_TO_HOST(smt_ant);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" ,smt_ant);
#ifdef MT7615
	Set_TxBfProfileTag_SmartAnt(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, smt_ant, cmd));
	os_free_mem(cmd);
HQA_TAG_SMTANT_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagSeIdx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 se_idx = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_SEIDX_FAIL;
	}
	NdisMoveMemory((PUCHAR)&se_idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	se_idx = PKTL_TRAN_TO_HOST(se_idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" ,se_idx);
#ifdef MT7615
	Set_TxBfProfileTag_SeIdx(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, se_idx, cmd));
	os_free_mem(cmd);
HQA_TAG_SEIDX_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagRmsdThrd(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 rmsd_thrd = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_RMSDTHRD_FAIL;
	}
	NdisMoveMemory((PUCHAR)&rmsd_thrd, (PUCHAR)&HqaCmdFrame->Data, 4);
	rmsd_thrd = PKTL_TRAN_TO_HOST(rmsd_thrd);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" ,rmsd_thrd);
#ifdef MT7615
	Set_TxBfProfileTag_RmsdThrd(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, cmd:%s\n", __FUNCTION__, rmsd_thrd, cmd));
	os_free_mem(cmd);
HQA_TAG_RMSDTHRD_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagMcsThrd(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_TAG_MCS_THRD tmp, *mcs_thrd;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_MCSTHRD_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	mcs_thrd = PKTLA_TRAN_TO_HOST(len/4, &tmp);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x" ,mcs_thrd->mcs_lss0, mcs_thrd->mcs_sss0, mcs_thrd->mcs_lss1, mcs_thrd->mcs_sss1, mcs_thrd->mcs_lss2, mcs_thrd->mcs_sss2);
#ifdef MT7615
	Set_TxBfProfileTag_McsThrd(pAd, cmd);
#endif
	os_free_mem(cmd);
HQA_TAG_MCSTHRD_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, len:%x, mcs_lss0:%x, mcs_sss0:%x, mcs_lss1:%x, mcs_sss1:%x, mcs_lss2:%x, mcs_sss2:%x\n", __FUNCTION__, len, mcs_thrd->mcs_lss0, mcs_thrd->mcs_sss0, mcs_thrd->mcs_lss1, mcs_thrd->mcs_sss1, mcs_thrd->mcs_lss2, mcs_thrd->mcs_sss2));
	return Ret;
}

static INT32 HQA_TxBfProfileTagTimeOut(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 bf_tout = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_TOUT_FAIL;
	}
	NdisMoveMemory((PUCHAR)&bf_tout, (PUCHAR)&HqaCmdFrame->Data, 4);
	bf_tout = PKTL_TRAN_TO_HOST(bf_tout);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" , bf_tout);
#ifdef MT7615
	Set_TxBfProfileTag_TimeOut(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, bf_tout, cmd));
	os_free_mem(cmd);
HQA_TAG_TOUT_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagDesiredBw(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 desire_bw = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DBW_FAIL;
	}
	NdisMoveMemory((PUCHAR)&desire_bw, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_bw = PKTL_TRAN_TO_HOST(desire_bw);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" , desire_bw);
#ifdef MT7615
	Set_TxBfProfileTag_DesiredBW(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, desire_bw, cmd));
	os_free_mem(cmd);
HQA_TAG_DBW_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagDesiredNc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 desire_nc = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNC_FAIL;
	}
	NdisMoveMemory((PUCHAR)&desire_nc, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_nc = PKTL_TRAN_TO_HOST(desire_nc);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" , desire_nc);
#ifdef MT7615
	Set_TxBfProfileTag_DesiredNc(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, desire_nc, cmd));
	os_free_mem(cmd);
HQA_TAG_DNC_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagDesiredNr(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 desire_nr = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNR_FAIL;
	}
	NdisMoveMemory((PUCHAR)&desire_nr, (PUCHAR)&HqaCmdFrame->Data, 4);
	desire_nr = PKTL_TRAN_TO_HOST(desire_nr);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" , desire_nr);
#ifdef MT7615
	Set_TxBfProfileTag_DesiredNr(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, desire_nr, cmd));
	os_free_mem(cmd);
HQA_TAG_DNR_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagWrite(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 idx = 0;	//WLAN_IDX
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_WRITE_FAIL;
	}
	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%d" , idx);
#ifdef MT7615
	Set_TxBfProfileTagWrite(pAd, cmd);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x, str:%s\n", __FUNCTION__, idx, cmd));
	os_free_mem(cmd);	
HQA_TAG_WRITE_FAIL:
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TxBfProfileTagRead(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 idx = 0, isBFer = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UCHAR *out = NULL;
	RTMP_STRING *cmd;
	
	ate_ctrl->txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_READ_FAIL;
	}
	
	ate_ctrl->Mode |= fATE_IN_BF;

	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	NdisMoveMemory((PUCHAR)&isBFer, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	isBFer = PKTL_TRAN_TO_HOST(isBFer);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x", idx, isBFer);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x %x, str:%s\n", __FUNCTION__, idx , isBFer, cmd));
//#ifdef MT7615
#if defined(MT7615) || defined(MT7637)
	Set_TxBfProfileTagRead(pAd, cmd);
	RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire);
#endif
	if(!ate_ctrl->txbf_info)
		goto HQA_TAG_READ_FAIL;

	out = PKTLA_TRAN_TO_NET(ate_ctrl->txbf_info_len/4, ate_ctrl->txbf_info);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x %x, str:%s\n", __FUNCTION__, idx , isBFer, cmd));
	NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2, (PUCHAR)out, ate_ctrl->txbf_info_len);
	ate_ctrl->Mode &= ~fATE_IN_BF;
	os_free_mem(ate_ctrl->txbf_info);
HQA_TAG_READ_FAIL:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + ate_ctrl->txbf_info_len, Ret);
	return Ret;
}

static INT32 HQA_StaRecCmmUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_STA_CMM_REC tmp, *rec;
	RTMP_STRING *cmd;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_PFMU_INFO *pfmu_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	
	if(len > sizeof(tmp)){
		Ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	rec = PKTLA_TRAN_TO_HOST((len-MAC_ADDR_LEN)/4, &tmp);

	if(rec->wlan_idx > ATE_BFMU_NUM){
		Ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}
	pfmu_info = &ate_ctrl->pfmu_info[rec->wlan_idx-1];	
	pfmu_info->wcid = rec->wlan_idx;
	pfmu_info->bss_idx = rec->bss_idx;
	NdisMoveMemory(pfmu_info->addr, rec->mac, MAC_ADDR_LEN);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",rec->wlan_idx, rec->bss_idx, rec->aid, PRINT_MAC(rec->mac));
#ifdef MT7615
	Set_StaRecCmmUpdate(pAd, cmd);
#endif
err0:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, wlan_idx:%x, bss_idx:%x, aid:%x, mac:%02x:%02x:%02x:%02x:%02x:%02x\n", __FUNCTION__, rec->wlan_idx, rec->bss_idx, rec->aid, PRINT_MAC(rec->mac)));
	return Ret;
}

static INT32 HQA_StaRecBfUpdate(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_STA_REC tmp, *rec;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_STAREC_BF_UPDATE_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	if(len > sizeof(tmp)){
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_STAREC_BF_UPDATE_FAIL;
	}
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	rec = PKTLA_TRAN_TO_HOST((len)/4, &tmp);
	PKTLA_DUMP( DBG_LVL_OFF, sizeof(*rec)/4, rec);	//Del after debug
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
			rec->wlan_idx, rec->bss_idx, rec->PfmuId, rec->su_mu, rec->etxbf_cap, rec->ndpa_rate, rec->ndp_rate, rec->report_poll_rate, 
			rec->tx_mode, rec->nc, rec->nr, rec->cbw, rec->spe_idx, rec->tot_mem_req, rec->mem_req_20m, rec->mem_row0, rec->mem_col0,
			rec->mem_row1, rec->mem_col1, rec->mem_row2, rec->mem_col2, rec->mem_row3, rec->mem_col3);
#ifdef MT7615
	Set_StaRecBfUpdate(pAd, cmd);
#endif
HQA_STAREC_BF_UPDATE_FAIL:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	printk("%s: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x\n", __func__,
			rec->wlan_idx, rec->bss_idx, rec->PfmuId, rec->su_mu, rec->etxbf_cap, rec->ndpa_rate, rec->ndp_rate, rec->report_poll_rate, 
			rec->tx_mode, rec->nc, rec->nr, rec->cbw, rec->tot_mem_req, rec->mem_req_20m, rec->mem_row0, rec->mem_col0, rec->mem_row1,
			rec->mem_col1, rec->mem_row2, rec->mem_col2, rec->mem_row3, rec->mem_col3);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, wlan_idx:%x, bss_idx:%x\n", __FUNCTION__, rec->wlan_idx, rec->bss_idx));
	return Ret;
}

static INT32 HQA_BFProfileDataRead(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 idx = 0, fgBFer = 0, subcarrIdx = 0, subcarr_start = 0, subcarr_end = 0;
	UINT32 offset = 0;
	UINT32 NumOfsub = 0;
	UCHAR *SubIdx = NULL;
	UCHAR *out = NULL;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	RTMP_STRING *cmd;
	INT debug_lvl = DebugLevel;

	ate_ctrl->txbf_info = NULL;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_DATA_READ_FAIL;
	}

	ate_ctrl->Mode |= fATE_IN_BF;
	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&HqaCmdFrame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	NdisMoveMemory((PUCHAR)&fgBFer, (PUCHAR)&HqaCmdFrame->Data + 4, 4);
	fgBFer = PKTL_TRAN_TO_HOST(fgBFer);
	NdisMoveMemory((PUCHAR)&subcarr_start, (PUCHAR)&HqaCmdFrame->Data + 4 + 4, 4);
	subcarr_start = PKTL_TRAN_TO_HOST(subcarr_start);
	NdisMoveMemory((PUCHAR)&subcarr_end, (PUCHAR)&HqaCmdFrame->Data + 4 + 4 + 4, 4);
	subcarr_end = PKTL_TRAN_TO_HOST(subcarr_end);

	NumOfsub = subcarr_end - subcarr_start + 1;
	NumOfsub = PKTL_TRAN_TO_HOST(NumOfsub);

	NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2, (PUCHAR)&NumOfsub, sizeof(NumOfsub));
	offset += sizeof(NumOfsub);

	DebugLevel = DBG_LVL_OFF;
	for ( subcarrIdx = subcarr_start; subcarrIdx <= subcarr_end; subcarrIdx++) {
		ate_ctrl->txbf_info = NULL;
	SubIdx = (UCHAR *)&subcarrIdx;

	memset(cmd, 0x00, HQA_BF_STR_SIZE);

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s, idx:%02x fgBFer:%02x Sub_H:%02x Sub_L:%02x subidx:%d\n",
			 __FUNCTION__, idx, fgBFer, SubIdx[1], SubIdx[0], subcarrIdx));

	sprintf(cmd, "%02x:%02x:%02x:%02x", idx, fgBFer, SubIdx[1], SubIdx[0]);
//#ifdef MT7615
#if defined(MT7615) || defined(MT7637)
	Set_TxBfProfileDataRead(pAd, cmd);
	RTMP_OS_WAIT_FOR_COMPLETION_TIMEOUT(&ate_ctrl->cmd_done, ate_ctrl->cmd_expire);
#endif
	if(!ate_ctrl->txbf_info)
		goto BF_PROFILE_DATA_READ_FAIL;

	out = PKTLA_TRAN_TO_NET(ate_ctrl->txbf_info_len/4, ate_ctrl->txbf_info);
		NdisMoveMemory((PUCHAR)&HqaCmdFrame->Data + 2 + offset, (PUCHAR)out, ate_ctrl->txbf_info_len);
		offset += ate_ctrl->txbf_info_len;
	os_free_mem(ate_ctrl->txbf_info);
	}
	ate_ctrl->Mode &= ~fATE_IN_BF;
BF_PROFILE_DATA_READ_FAIL:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2 + offset, Ret);
	DebugLevel = debug_lvl;
	return Ret;
}

static INT32 HQA_BFProfileDataWrite(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	INT debug_lvl = DebugLevel;
	HQA_BF_STA_PROFILE tmp, *profile;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto BF_PROFILE_DATA_WRITE_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	profile = PKTLA_TRAN_TO_HOST((len)/sizeof(UINT32), &tmp);
	DebugLevel = DBG_LVL_OFF;
	PKTLA_DUMP( DBG_LVL_INFO, sizeof(tmp)/sizeof(UINT32), &tmp);	//Del after debug
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:", profile->pfmuid);
	sprintf(cmd + strlen(cmd), "%03x:", profile->subcarrier);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi11);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi21);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi21);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi31);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi31);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi41);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi22);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi32);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi32);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi42);
	sprintf(cmd + strlen(cmd), "%03x:", profile->phi33);
	sprintf(cmd + strlen(cmd), "%02x:", profile->psi43);
	sprintf(cmd + strlen(cmd), "%02x:", profile->snr00);
	sprintf(cmd + strlen(cmd), "%02x:", profile->snr01);
	sprintf(cmd + strlen(cmd), "%02x:", profile->snr02);
	sprintf(cmd + strlen(cmd), "%02x", profile->snr03);

#ifdef MT7615
	Set_TxBfProfileDataWrite(pAd, cmd);
#endif
	os_free_mem(cmd);
BF_PROFILE_DATA_WRITE_FAIL:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s\n", __FUNCTION__));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	DebugLevel = debug_lvl;
	return Ret;
}

static INT32 HQA_BFSounding(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	HQA_BF_SOUDING tmp, *param;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto HQA_BFSOUNDING_FAIL;
	}
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);
	if(len > sizeof(tmp)){
		Ret = NDIS_STATUS_INVALID_DATA;
		goto HQA_BFSOUNDING_FAIL;
	}
	NdisMoveMemory((PUCHAR)&tmp, (PUCHAR)&HqaCmdFrame->Data, len);
	param = PKTLA_TRAN_TO_HOST((len)/sizeof(UINT32), &tmp);
	PKTLA_DUMP( DBG_LVL_OFF, sizeof(*param)/sizeof(UINT32), param);	//Del after debug
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x:%02x:%02x", param->su_mu, param->mu_num, param->snd_interval, param->wlan_id0, param->wlan_id1, param->wlan_id2, param->wlan_id3);
#ifdef MT7615
	MtATESetMacTxRx(pAd, ASIC_MAC_TX, TRUE, param->band_idx);
	Set_Trigger_Sounding_Proc(pAd, cmd);
#endif
HQA_BFSOUNDING_FAIL:
	if(cmd)
		os_free_mem(cmd);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s\n", __FUNCTION__));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
static INT32 HQA_TXBFSoundingStop(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
#ifdef MT7615
	Set_Stop_Sounding_Proc(pAd, NULL);
#endif
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s\n", __FUNCTION__));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_TXBFProfileDataWriteAllExt(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 bw = 0;
	UINT32 profile_idx = 0;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}
	NdisMoveMemory((PUCHAR)&bw, (PUCHAR)&HqaCmdFrame->Data, sizeof(bw));
	NdisMoveMemory((PUCHAR)&profile_idx, (PUCHAR)&HqaCmdFrame->Data + sizeof(bw), sizeof(profile_idx));
	bw = PKTL_TRAN_TO_HOST(bw);
	profile_idx = PKTL_TRAN_TO_HOST(profile_idx);
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%x:%x",profile_idx, bw);

	err0:
	if(cmd)
		os_free_mem(cmd);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, bw:%x, profile_idx:%x\n", __FUNCTION__, bw, profile_idx));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

#ifdef MT_MAC
static INT32 HQA_TxBfTxApply(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 eBF_enable = 0;
	UINT32 iBF_enable = 0;
	UINT32 wlan_id = 0;
	UINT32 MuTx_enable = 0;
	UINT32 iBFPhaseCali = 0;
	RTMP_STRING *cmd;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UCHAR *data = HqaCmdFrame->Data;
	
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		Ret = NDIS_STATUS_RESOURCES;
		goto err0;
	}
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&eBF_enable);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&iBF_enable);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&wlan_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&MuTx_enable);
	TESTMODE_SET_PARAM(ate_ctrl, wlan_id, eTxBf, eBF_enable);
	TESTMODE_SET_PARAM(ate_ctrl, wlan_id, iTxBf, iBF_enable);
	sprintf(cmd, "%02x:%02x:%02x:%02x:%02x",wlan_id, eBF_enable, iBF_enable, MuTx_enable, iBFPhaseCali);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s,wlan_id:%x, eBF enable:%x, iBF enable:%x, MuTx:%x\n", __FUNCTION__, wlan_id, eBF_enable, iBF_enable, MuTx_enable));
	Set_TxBfTxApply(pAd, cmd);
err0:
	if(cmd)
		os_free_mem(cmd);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
static INT32 HQA_ManualAssoc(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	P_MANUAL_CONN manual_cfg = &pAd->AteManualConnInfo;
	HQA_BF_MANUAL_CONN manual_conn;
	RTMP_STRING rate_str[64];
	char ucNsts;
	UINT_32 rate[8];
	RA_PHY_CFG_T TxPhyCfg;
	UCHAR *data = HqaCmdFrame->Data;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.type);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.wtbl_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.ownmac_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.phymode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.pfmuid);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.marate_mode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.marate_mcs);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.spe_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&manual_conn.aid);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)manual_conn.mac);

	manual_cfg->peer_op_type = manual_conn.type;
	manual_cfg->wtbl_idx = manual_conn.wtbl_idx;
	manual_cfg->ownmac_idx = manual_conn.ownmac_idx;
	switch(manual_conn.phymode) {
	/* abggnanac */
	case 0:
		manual_cfg->peer_phy_mode = WMODE_B|WMODE_G|WMODE_GN|WMODE_A|WMODE_AN|WMODE_AC;
		break;
	/* bggnan */
	case 1:
		manual_cfg->peer_phy_mode = WMODE_B|WMODE_GN|WMODE_G|WMODE_AN;
		break;
	/* aanac */
	case 2:
		manual_cfg->peer_phy_mode = WMODE_A|WMODE_AN|WMODE_AC;
		break;
	default:
		manual_cfg->peer_phy_mode = WMODE_B|WMODE_G|WMODE_GN|WMODE_A|WMODE_AN|WMODE_AC;
		break;
	}

	switch(manual_conn.bw){
	case ATE_BAND_WIDTH_20:
		manual_cfg->peer_bw = BAND_WIDTH_20;
		break;
	case ATE_BAND_WIDTH_40:
		manual_cfg->peer_bw = BAND_WIDTH_40;
		break;
	case ATE_BAND_WIDTH_80:
		manual_cfg->peer_bw = BAND_WIDTH_80;
		break;
	case ATE_BAND_WIDTH_10:
		manual_cfg->peer_bw = BAND_WIDTH_10;
		break;
	case ATE_BAND_WIDTH_5:
		manual_cfg->peer_bw = BAND_WIDTH_5;
		break;
	case ATE_BAND_WIDTH_160:
		manual_cfg->peer_bw = BAND_WIDTH_160;
		break;
	case ATE_BAND_WIDTH_8080:
		manual_cfg->peer_bw = BAND_WIDTH_8080;
		break;
	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s: Cannot find BW with manual_conn.bw:%x\n",
				 __func__, manual_conn.bw));
		manual_cfg->peer_bw = manual_conn.bw;
		break;
	}
	manual_cfg->pfmuId = manual_conn.pfmuid;
	manual_cfg->peer_maxrate_mode = manual_conn.marate_mode;
	manual_cfg->peer_maxrate_mcs = manual_conn.marate_mcs;
	manual_cfg->spe_idx = manual_conn.spe_idx;
	manual_cfg->aid = manual_conn.aid;
	manual_cfg->peer_nss = 1; /* MU */
	NdisMoveMemory(manual_cfg->peer_mac, manual_conn.mac, MAC_ADDR_LEN);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("%s():User manual configured peer STA info:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->AteManualConnInfo.peer_mac)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tBAND=>%d\n", pAd->AteManualConnInfo.peer_band));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tOwnMacIdx=>%d\n", pAd->AteManualConnInfo.ownmac_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tWTBL_Idx=>%d\n", pAd->AteManualConnInfo.wtbl_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tOperationType=>%d\n", pAd->AteManualConnInfo.peer_op_type));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tPhyMode=>%d\n", pAd->AteManualConnInfo.peer_phy_mode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tBandWidth=>%d\n", pAd->AteManualConnInfo.peer_bw));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tNSS=>%d\n", pAd->AteManualConnInfo.peer_nss));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tPfmuId=>%d\n", pAd->AteManualConnInfo.pfmuId));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tAid=>%d\n", pAd->AteManualConnInfo.aid));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tSpe_idx=>%d\n", pAd->AteManualConnInfo.spe_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tMaxRate_Mode=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("\tMaxRate_MCS=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mcs));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
	    ("Now apply it to hardware!\n"));

	/* This applied the manual config info into the mac table entry, including the HT/VHT cap, VHT MCS set */
	SetATEApplyStaToMacTblEntry(pAd);

	/* Fixed rate configuration */
	NdisZeroMemory(&rate_str[0], sizeof(rate_str));
	sprintf(rate_str, "%d-%d-%d-%d-%d-%d-%d-%d-%d-%d",
			pAd->AteManualConnInfo.wtbl_idx,
			pAd->AteManualConnInfo.peer_maxrate_mode,
			pAd->AteManualConnInfo.peer_bw,
			pAd->AteManualConnInfo.peer_maxrate_mcs,
			pAd->AteManualConnInfo.peer_nss,
			0, 0, 0, 0, 0);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\tSet fixed RateInfo string as %s\n", rate_str));
	//Set_Fixed_Rate_Proc(pAd, rate_str);
	ucNsts = get_nsts_by_mcs(pAd->AteManualConnInfo.peer_maxrate_mode, 
	                         pAd->AteManualConnInfo.peer_maxrate_mcs, 
	                         FALSE, 
	                         pAd->AteManualConnInfo.peer_nss);

	rate[0] = tx_rate_to_tmi_rate(pAd->AteManualConnInfo.peer_maxrate_mode,
	        		      pAd->AteManualConnInfo.peer_maxrate_mcs,
				      ucNsts,
				      FALSE,
				      0);
	rate[0] &= 0xfff;
            
	rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];

	os_zero_mem(&TxPhyCfg, sizeof(TxPhyCfg));
	TxPhyCfg.BW      = pAd->AteManualConnInfo.peer_bw;
	TxPhyCfg.ShortGI = FALSE;
	//TxPhyCfg.ldpc  = HT_LDPC | VHT_LDPC;
	TxPhyCfg.ldpc    = 0;

	MtAsicTxCapAndRateTableUpdate(pAd, 
	                              pAd->AteManualConnInfo.wtbl_idx, 
	                              &TxPhyCfg, 
	                              rate, 
	                              FALSE);
	/* WTBL configuration */
	SetATEApplyStaToAsic(pAd);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#else
static INT32 HQA_eBFEnable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 enable = 0;
	UINT32 band_idx = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	NdisMoveMemory((PUCHAR)&enable, (PUCHAR)&HqaCmdFrame->Data, sizeof(enable));
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data + sizeof(enable), sizeof(band_idx));
	enable = PKTL_TRAN_TO_HOST(enable);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, eTxBf, enable);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, enable:%x, band_idx:%x\n", __FUNCTION__, enable, band_idx));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
static INT32 HQA_iBFEnable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 enable = 0;
	UINT32 band_idx = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	NdisMoveMemory((PUCHAR)&enable, (PUCHAR)&HqaCmdFrame->Data, sizeof(enable));
	NdisMoveMemory((PUCHAR)&band_idx, (PUCHAR)&HqaCmdFrame->Data + sizeof(enable), sizeof(band_idx));
	enable = PKTL_TRAN_TO_HOST(enable);
	band_idx = PKTL_TRAN_TO_HOST(band_idx);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, iTxBf, enable);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, enable:%x, band_idx:%x\n", __FUNCTION__, enable, band_idx));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}
#endif
static HQA_CMD_HANDLER HQA_TXBF_CMDS[] = {
	HQA_TxBfProfileTagInValid,	/* 0x1540 */
	HQA_TxBfProfileTagPfmuIdx,	/* 0x1541 */
	HQA_TxBfProfileTagBfType,	/* 0x1542 */
	HQA_TxBfProfileTagBw,		/* 0x1543 */
	HQA_TxBfProfileTagSuMu,		/* 0x1544 */
	HQA_TxBfProfileTagMemAlloc,	/* 0x1545 */	
	HQA_TxBfProfileTagMatrix,	/* 0x1546 */
	HQA_TxBfProfileTagSnr,		/* 0x1547 */	
	HQA_TxBfProfileTagSmtAnt,	/* 0x1548 */	
	HQA_TxBfProfileTagSeIdx,	/* 0x1549 */	
	HQA_TxBfProfileTagRmsdThrd,	/* 0x154A */	
	HQA_TxBfProfileTagMcsThrd,	/* 0x154B */
	HQA_TxBfProfileTagTimeOut,	/* 0x154C */
	HQA_TxBfProfileTagDesiredBw,/* 0x154D */
	HQA_TxBfProfileTagDesiredNc,/* 0x154E */
	HQA_TxBfProfileTagDesiredNr,/* 0x154F */
	HQA_TxBfProfileTagWrite,	/* 0x1550 */
	HQA_TxBfProfileTagRead,		/* 0x1551 */
 	HQA_StaRecCmmUpdate,		/* 0x1552 */
 	HQA_StaRecBfUpdate,			/* 0x1553 */
	HQA_BFProfileDataRead,		/* 0x1554 */
 	HQA_BFProfileDataWrite,		/* 0x1555 */
 	HQA_BFSounding,				/* 0x1556 */
	HQA_TXBFSoundingStop,		/* 0x1557 */
	HQA_TXBFProfileDataWriteAllExt, /* 0x1558 */
#ifdef MT_MAC
	HQA_TxBfTxApply,		/* 0x1559 */
	HQA_ManualAssoc,		/* 0x155A */
#else
	HQA_eBFEnable,				/* 0x1559 */
    HQA_iBFEnable,				/* 0x155A */
#endif

};

#ifdef CFG_SUPPORT_MU_MIMO
static INT32 HQA_MUGetInitMCS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 gid = 0;
	MU_STRUCT_MU_GROUP_INIT_MCS init_mcs;
	HQA_MU_USR_INIT_MCS out;

	NdisMoveMemory((UCHAR *)&gid, HqaCmdFrame->Data, sizeof(gid));
	gid = PKTS_TRAN_TO_HOST(gid);
	os_zero_mem(&init_mcs, sizeof(init_mcs));
	os_zero_mem(&out, sizeof(out));

	Ret = hqa_wifi_test_mu_get_init_mcs(pAd, gid, &init_mcs);
	out.user0 = init_mcs.user0InitMCS;
	out.user1 = init_mcs.user1InitMCS;
	out.user2 = init_mcs.user2InitMCS;
	out.user3 = init_mcs.user3InitMCS;
            
	NdisMoveMemory(HqaCmdFrame->Data + 2, (UCHAR *)&out, sizeof(out));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, gid:%u\n", __FUNCTION__,gid));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(init_mcs), Ret);
	return Ret;
}

static INT32 HQA_MUCalInitMCS(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	//INT32 gid = 0;
	MU_STRUCT_SET_CALC_INIT_MCS param;
	UCHAR *data = HqaCmdFrame->Data;

	os_zero_mem(&param, sizeof(param));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_of_user);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.bandwidth);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss_of_user0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss_of_user1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss_of_user2);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss_of_user3);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pf_mu_id_of_user0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pf_mu_id_of_user1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pf_mu_id_of_user2);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pf_mu_id_of_user3);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_of_txer);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.spe_index);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.group_index);

	Ret = hqa_wifi_test_mu_cal_init_mcs(pAd, &param);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, spe_idx:%d\n", __FUNCTION__, param.spe_index));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUCalLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 Type = 0;
	INT32 gid = 0;
	UINT32 txer = 0;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_SET_SU_CALC_LQ param_su;
	MU_STRUCT_SET_CALC_LQ param_mu;

	os_zero_mem(&param_su, sizeof(param_su));
	os_zero_mem(&param_mu, sizeof(param_mu));

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Type);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.num_of_user);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.bandwidth);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.nss_of_user0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.nss_of_user1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.nss_of_user2);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.nss_of_user3);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.pf_mu_id_of_user0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.pf_mu_id_of_user1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.pf_mu_id_of_user2);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.pf_mu_id_of_user3);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.num_of_txer);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.spe_index);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_mu.group_index);

	param_su.num_of_user = param_mu.num_of_user;
	param_su.bandwidth = param_mu.bandwidth;
	param_su.nss_of_user0 = param_mu.nss_of_user0;
	param_su.pf_mu_id_of_user0 = param_mu.pf_mu_id_of_user0;
	param_su.num_of_txer = param_mu.num_of_txer;
	param_su.spe_index = param_mu.spe_index;
	param_su.group_index = param_mu.group_index;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, gid:%u, txer:%u spe_idx:%d\n", __FUNCTION__, gid, txer, param_mu.spe_index));

	if ( Type == 0 )
		Ret = hqa_wifi_test_su_cal_lq(pAd, &param_su);
	else
		Ret = hqa_wifi_test_mu_cal_lq(pAd, &param_mu);

	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUGetLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT32 Type = 0;
	UCHAR *data = HqaCmdFrame->Data;
	SU_STRUCT_LQ_REPORT lq_su;
	MU_STRUCT_LQ_REPORT lq_mu;

	os_zero_mem(&lq_su, sizeof(lq_su));
	os_zero_mem(&lq_mu, sizeof(lq_mu));

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Type);
	/* TODO:: Check if structure changed*/
	if ( Type == 0){
		Ret = hqa_wifi_test_su_get_lq(pAd, &lq_su);
		PKTLA_DUMP( DBG_LVL_OFF, sizeof(lq_su)/sizeof(int), &lq_su);
		NdisMoveMemory(HqaCmdFrame->Data + 2,(UCHAR *)&lq_su, sizeof(lq_su));
		ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(lq_su), Ret);
	}
	else {
		Ret = hqa_wifi_test_mu_get_lq(pAd, &lq_mu);
		PKTLA_DUMP( DBG_LVL_OFF, sizeof(lq_mu)/sizeof(int), &lq_mu);
		NdisMoveMemory(HqaCmdFrame->Data + 2,(UCHAR *)&lq_mu, sizeof(lq_mu));
		ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(lq_mu), Ret);
	}	
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s\n", __FUNCTION__));
	return Ret;
}

static INT32 HQA_MUSetSNROffset(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 offset = 0;

	NdisMoveMemory((UCHAR *)&offset, HqaCmdFrame->Data, sizeof(offset));
	offset = PKTL_TRAN_TO_HOST(offset);
	Ret = hqa_wifi_test_snr_offset_set(pAd, offset);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, offset:%x\n", __FUNCTION__, offset));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetZeroNss(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 zero_nss = 0;

	NdisMoveMemory((UCHAR *)&zero_nss, HqaCmdFrame->Data, sizeof(zero_nss));
	zero_nss = PKTL_TRAN_TO_HOST(zero_nss);	
	Ret = hqa_wifi_test_mu_set_zero_nss(pAd, (UCHAR)zero_nss);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, zero_nss:%x\n", __FUNCTION__, zero_nss));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetSpeedUpLQ(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 spdup_lq = 0;

	NdisMoveMemory((UCHAR *)&spdup_lq, HqaCmdFrame->Data, sizeof(spdup_lq));
	spdup_lq = PKTL_TRAN_TO_HOST(spdup_lq);	
	Ret = hqa_wifi_test_mu_speed_up_lq(pAd, spdup_lq);
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, spdup_lq:%x\n", __FUNCTION__, spdup_lq));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetMUTable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UCHAR *tbl = NULL;
	UINT32 su_mu = 0;
	MU_STRUCT_MU_TABLE info;
	
	os_zero_mem(&info, sizeof(info));
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length) - sizeof(su_mu);

	NdisMoveMemory((UCHAR *)&su_mu, HqaCmdFrame->Data, sizeof(su_mu));
	su_mu = PKTL_TRAN_TO_HOST(su_mu);
	
	Ret = os_alloc_mem(pAd, (UCHAR **)&tbl, sizeof(UCHAR)*len);
	if(Ret)
		goto err0;
	
	os_zero_mem(tbl, len);

	NdisMoveMemory((UCHAR *)tbl, HqaCmdFrame->Data + sizeof(su_mu), len);

	info.type = su_mu;
	info.length = len;
	info.prTable = tbl;
	Ret = hqa_wifi_test_mu_table_set(pAd, &info);
	err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, len:%u, su_mu:%u\n", __FUNCTION__,len, su_mu));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetGroup(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 val32;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_MU_GROUP grp;;
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.groupIndex = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.numOfUser = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0Ldpc = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1Ldpc = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2Ldpc = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3Ldpc = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.shortGI = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.bw = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0Nss = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1Nss = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2Nss = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3Nss = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.groupId = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0UP = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1UP = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2UP = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3UP = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0MuPfId = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1MuPfId = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2MuPfId = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3MuPfId = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user0InitMCS = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user1InitMCS = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user2InitMCS = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	grp.user3InitMCS = val32;
	
	NdisMoveMemory(grp.aucUser0MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser1MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser2MacAddr, data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(grp.aucUser3MacAddr, data, MAC_ADDR_LEN);
	
	Ret = hqa_wifi_test_mu_group_set(pAd, &grp);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, len:%x\n", __FUNCTION__, len));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("0:%02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(grp.aucUser0MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("1:%02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(grp.aucUser1MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("2:%02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(grp.aucUser2MacAddr)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("3:%02x:%02x:%02x:%02x:%02x:%02x\n",PRINT_MAC(grp.aucUser3MacAddr)));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUGetQD(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	INT debug_lvl = DebugLevel;
	INT32 subcarrier_idx = 0;
	MU_STRUCT_MU_QD qd;
	NdisMoveMemory((PUCHAR)&subcarrier_idx, HqaCmdFrame->Data, sizeof(subcarrier_idx));
	NdisZeroMemory(&qd, sizeof(qd));

	subcarrier_idx = PKTL_TRAN_TO_HOST(subcarrier_idx);
	DebugLevel = DBG_LVL_OFF;
	Ret = hqa_wifi_test_mu_get_qd(pAd, subcarrier_idx, &qd);
	PKTLA_DUMP( DBG_LVL_INFO, sizeof(qd)/sizeof(int), &qd);
	NdisMoveMemory(HqaCmdFrame->Data + 2,(UCHAR *)&qd, sizeof(qd));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s, sub:%d, qd.length:%zu, pqd:%p, HqaCmd->Data:%p\n"
		, __FUNCTION__, subcarrier_idx, sizeof(qd), &qd, HqaCmdFrame->Data));
	ResponseToQA(HqaCmdFrame, WRQ, 2 + sizeof(qd), Ret);
	DebugLevel = debug_lvl;
	return Ret;
}

static INT32 HQA_MUSetEnable(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT32 is_enable = 0;
	//ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);

	NdisMoveMemory((PUCHAR)&is_enable, HqaCmdFrame->Data, sizeof(is_enable));
	is_enable = PKTL_TRAN_TO_HOST(is_enable);
	
	Ret = hqa_wifi_test_mu_set_enable( pAd, is_enable);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s,MU is_enable:%x\n", __FUNCTION__, is_enable));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32 HQA_MUSetGID_UP(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UCHAR *data = HqaCmdFrame->Data;
	UINT32 val32 = 0;
	INT i = 0;
	MU_STRUCT_MU_STA_PARAM param;
	
	os_zero_mem(&param, sizeof(param));
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	for(i=0;i<2;i++) {
		NdisMoveMemory(&val32, data, sizeof(val32));
		val32 = PKTL_TRAN_TO_HOST(val32);
		param.gid[i] = val32;
		data += sizeof(val32);
	}

	for(i=0;i<4;i++) {
		NdisMoveMemory(&val32, data, sizeof(val32));
		val32 = PKTL_TRAN_TO_HOST(val32);
		param.up[i] = val32;
		data += sizeof(val32);
	}
	//Del after debug
	PKTLA_DUMP( DBG_LVL_OFF, sizeof(param)/sizeof(UINT32), &param);
	Ret = hqa_wifi_test_mu_set_sta_gid_and_up(pAd, &param);
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}

static INT32  HQA_MUTriggerTx(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	INT32 Ret = 0;
	UINT16 len = 0;
	UINT32 val32;
	UINT32 band_idx = 0;
	UCHAR *data = HqaCmdFrame->Data;
	MU_STRUCT_TRIGGER_MU_TX_FRAME_PARAM mu_tx_param;
	len = PKTS_TRAN_TO_HOST(HqaCmdFrame->Length);

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	band_idx = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.fgIsRandomPattern = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength0 = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength1 = val32;
	
	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength2 = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.msduPayloadLength3 = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.u4MuPacketCount = val32;

	Ret = EthGetParamAndShiftBuff(TRUE, sizeof(val32), &data, (UCHAR *)&val32);
	mu_tx_param.u4NumOfSTAs = val32;

	NdisMoveMemory(mu_tx_param.macAddrs[0], data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(mu_tx_param.macAddrs[1], data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(mu_tx_param.macAddrs[2], data, MAC_ADDR_LEN);
	data += MAC_ADDR_LEN;
	NdisMoveMemory(mu_tx_param.macAddrs[3], data, MAC_ADDR_LEN);
	
	Ret = hqa_wifi_test_mu_trigger_mu_tx(pAd, &mu_tx_param);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s, len:%x\n", __FUNCTION__, len));
	ResponseToQA(HqaCmdFrame, WRQ, 2, Ret);
	return Ret;
}


static HQA_CMD_HANDLER HQA_TXMU_CMDS[] = {
	HQA_MUGetInitMCS,			/* 0x1560 */
	HQA_MUCalInitMCS,			/* 0x1561 */
	HQA_MUCalLQ,				/* 0x1562 */
	HQA_MUGetLQ,				/* 0x1563 */
	HQA_MUSetSNROffset,			/* 0x1564 */
	HQA_MUSetZeroNss,			/* 0x1565 */
	HQA_MUSetSpeedUpLQ,			/* 0x1566 */
	HQA_MUSetMUTable,			/* 0x1567 */
	HQA_MUSetGroup,				/* 0x1568 */
	HQA_MUGetQD,				/* 0x1569 */
	HQA_MUSetEnable,			/* 0x156A */
	HQA_MUSetGID_UP,			/* 0x156B */
	HQA_MUTriggerTx,			/* 0x156C */
};
#endif /* CFG_SUPPORT_MU_MIMO */
#endif /* TXBF_SUPPORT */

static INT32 HQA_CapWiFiSpectrum(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *WRQ, HQA_CMD_FRAME *HqaCmdFrame)
{
	/* Set Param*/
	UINT32 Control, Trigger, RingCapEn, Event, Node, Len, StopCycle, BW, MACTriggerEvent, Band;
	UCHAR SourceAddress[MAC_ADDR_LEN];
	UINT32    SourceAddrLSB = 0;
	UINT32    SourceAddrMSB = 0;
	/* Get Param*/
	UINT32 StartAddr1, StartAddr2, StartAddr3, EndAddr, StopAddr, Wrap;
	UINT32 Value;
	UINT32 RespLen = 2;
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UCHAR *data = HqaCmdFrame->Data;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s:", __FUNCTION__));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Control);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Control:%d ", Control));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Trigger);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Trigger:%d ", Trigger));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&RingCapEn);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("RingCapEn:%d ", RingCapEn));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Event);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Event:%d ", Event));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Node);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Node:%d ", Node));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Len);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Len:%d ", Len));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&StopCycle);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("StopCycle:%d ", StopCycle));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&BW);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("BW:%d ", BW));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&MACTriggerEvent);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("MACTriggerEvent:%d ", MACTriggerEvent));
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)SourceAddress);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("SourceAddress:%02x:%02x:%02x:%02x:%02x:%02x\n ", PRINT_MAC(SourceAddress)));
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&Band);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("Band:%d ", Band));
	
	SourceAddrLSB = ((SourceAddress[0]) | (SourceAddress[1] << 8) | (SourceAddress[2]) << 16 | (SourceAddress[3]) << 24);
	SourceAddrMSB = ((SourceAddress[4]) | (SourceAddress[5]<<8) | (0x1 <<16));		

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("\n"));

	if (Control == 1){
		if(ATEOp->SetCapWiFiSpectrum)
			ATEOp->SetCapWiFiSpectrum(pAd, Trigger, RingCapEn, Event, Node, Len, StopCycle, BW, MACTriggerEvent, SourceAddrLSB, SourceAddrMSB, Band);
		else
			Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);
	} else if (Control == 2){
		if(ATEOp->GetCapWiFiSpectrum){
			Ret = ATEOp->GetCapWiFiSpectrum(pAd, &StartAddr1, &StartAddr2, &StartAddr3, &EndAddr, &StopAddr, &Wrap);

			data = HqaCmdFrame->Data;
   			Value = PKTL_TRAN_TO_HOST(StartAddr1);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
   			Value = PKTL_TRAN_TO_HOST(StartAddr2);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
   			Value = PKTL_TRAN_TO_HOST(StartAddr3);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
   			Value = PKTL_TRAN_TO_HOST(EndAddr);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
   			Value = PKTL_TRAN_TO_HOST(StopAddr);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);
   			Value = PKTL_TRAN_TO_HOST(Wrap);
			NdisMoveMemory(data + RespLen, (UCHAR *)&Value, sizeof(Value));
			RespLen += sizeof(Value);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, StartAddr1:%02x StartAddr2:%02x StartAddr3:%02x EndAddr:%02x StopAddr:%02x Wrap:%02x\n",
		 __FUNCTION__, StartAddr1, StartAddr2, StartAddr3, EndAddr, StopAddr, Wrap));
		}
		else
			Ret = TM_STATUS_NOTSUPPORT;
		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);
	}
//	else
//		ResponseToQA(HqaCmdFrame, WRQ, RespLen, Ret);

	return Ret;
}

static HQA_CMD_HANDLER HQA_ICAP_CMDS[] = {
	HQA_CapWiFiSpectrum,			/* 0x1580 */
};

static INT32 hqa_set_channel_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq
								 , HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UINT32 len = 0;
	HQA_EXT_SET_CH param;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data;
	UINT32 pri_ch = 0;
	UINT32 band_idx = 0;
	UINT32 bw = 0;
	UINT32 per_pkt_bw = 0;
	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.central_ch0);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.central_ch1);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.sys_bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.perpkt_bw);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.pri_sel);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.reason);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data,(UCHAR *)&param.ch_band);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.out_band_freq);
	
	if(param.band_idx > TESTMODE_BAND_NUM){
		ret = NDIS_STATUS_INVALID_DATA;
		goto err0;
	}
	band_idx = param.band_idx;
	switch(param.sys_bw){
	case ATE_BAND_WIDTH_20:
		bw = BAND_WIDTH_20;
		break;
	case ATE_BAND_WIDTH_40:
		bw = BAND_WIDTH_40;
		break;
	case ATE_BAND_WIDTH_80:
		bw = BAND_WIDTH_80;
		break;
	case ATE_BAND_WIDTH_10:
		bw = BAND_WIDTH_10;
		break;
	case ATE_BAND_WIDTH_5:
		bw = BAND_WIDTH_5;
		break;
	case ATE_BAND_WIDTH_160:
		bw = BAND_WIDTH_160;
		break;
	case ATE_BAND_WIDTH_8080:
		bw = BAND_WIDTH_8080;
		break;
	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s: Cannot find BW with param.sys_bw:%x\n",
				 __FUNCTION__, param.sys_bw));
		bw = param.sys_bw;
		break;
	}
	switch(param.perpkt_bw){
	case ATE_BAND_WIDTH_20:
		per_pkt_bw = BAND_WIDTH_20;
		break;
	case ATE_BAND_WIDTH_40:
		per_pkt_bw = BAND_WIDTH_40;
		break;
	case ATE_BAND_WIDTH_80:
		per_pkt_bw = BAND_WIDTH_80;
		break;
	case ATE_BAND_WIDTH_10:
		per_pkt_bw = BAND_WIDTH_10;
		break;
	case ATE_BAND_WIDTH_5:
		per_pkt_bw = BAND_WIDTH_5;
		break;
	case ATE_BAND_WIDTH_160:
	case ATE_BAND_WIDTH_8080:
		per_pkt_bw = BAND_WIDTH_160;
		break;
	default:
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
				("%s: Cannot find BW with param.sys_bw:%x\n",
				 __FUNCTION__, param.sys_bw));
		per_pkt_bw = bw;
		break;
	}
	/* Set Param */
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Channel, param.central_ch0);
#ifdef DOT11_VHT_AC
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Channel_2nd, param.central_ch1);
#endif
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, PerPktBW, per_pkt_bw);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, BW, bw);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, PriSel, param.pri_sel);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Ch_Band, param.ch_band);
    TESTMODE_SET_PARAM(ate_ctrl, band_idx, OutBandFreq, param.out_band_freq);
	
	ate_ops->SetChannel(pAd, param.central_ch0, band_idx, param.pri_sel, param.reason, param.ch_band);
	
	err0:
	NdisMoveMemory(cmd_frame->Data + 2,(UCHAR *)&param.ext_id, 4);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:len:%x, num_param:%x, band_idx:%x, ch0:%u, ch1:%u, sys_bw:%x, bw_conver:%x, ",
			__FUNCTION__, len, param.num_param , param.band_idx,
			param.central_ch0, param.central_ch1, param.sys_bw, bw));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("perpkt_bw:%x, pri_sel:%x, pri_ch:%u\n",
			param.perpkt_bw, param.pri_sel, pri_ch));
	ResponseToQA(cmd_frame, wrq, 6, ret);
	return ret;
}

static INT32 hqa_set_txcontent_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UINT32 len = 0, pl_len = 0;
	HQA_EXT_TX_CONTENT param;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UCHAR *data = cmd_frame->Data;
	HEADER_802_11 *phdr = NULL;
	UCHAR *addr1, *addr2, *addr3, *payload;
	UINT32 band_idx = 0;
	
	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);
	
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.FC);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.dur);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.seq);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.gen_payload_rule);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.txlen);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.payload_len);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr1);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr2);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, param.addr3);
	/* 52 for the size before payload */
	if (param.payload_len > ATE_MAX_PATTERN_SIZE)
		param.payload_len = ATE_MAX_PATTERN_SIZE;
	
	/* Set Param */
	band_idx = param.band_idx;
	addr1 = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Addr1);
	addr2 = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Addr2);
	addr3 = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Addr3);
   	NdisMoveMemory(addr1, param.addr1, MAC_ADDR_LEN);
   	NdisMoveMemory(addr2, param.addr2, MAC_ADDR_LEN);
   	NdisMoveMemory(addr3, param.addr3, MAC_ADDR_LEN);

	phdr = (HEADER_802_11*)TESTMODE_GET_PARAM(ate_ctrl, band_idx, TemplateFrame);
	pl_len = TESTMODE_GET_PARAM(ate_ctrl, band_idx, pl_len);
	//pl_addr = TESTMODE_GET_PADDR(ate_ctrl, band_idx, payload);
	payload = TESTMODE_GET_PARAM(ate_ctrl, band_idx, payload);
	NdisMoveMemory(&phdr->FC, &param.FC, sizeof(phdr->FC));
	phdr->Duration = (UINT16)param.dur;
	phdr->Sequence = (UINT16)param.seq;
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, FixedPayload, param.gen_payload_rule);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, TxLength, param.txlen);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, pl_len, param.payload_len);
	//payload = *pl_addr;
		
	EthGetParamAndShiftBuff(FALSE, param.payload_len, &data, payload);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&param.ext_id, sizeof(param.ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(param.ext_id), ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("ret:%u, len:%u, param_len:%u\n",
			ret, param.payload_len, pl_len));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, FC:%04x, dur:%u, seq:%u, plen:%u, txlen:%u, GENPKT:%u\n",
			__FUNCTION__, param.FC, param.dur, param.seq, param.payload_len,
			param.txlen, param.gen_payload_rule));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, addr1:%02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(addr1)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, addr2:%02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(addr2)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, addr3:%02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__, PRINT_MAC(addr3)));
	return ret;
}

static INT32 hqa_start_tx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT32 len = 0;
	UINT32 band_idx = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data; 
	HQA_EXT_TXV param;
	ATE_TXPOWER TxPower;
	UINT32 Channel = 0, Ch_Band = 0, SysBw = 0, PktBw = 0;
	len = PKTS_TRAN_TO_HOST(cmd_frame->Length);

	
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.num_param);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.band_idx);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pkt_cnt);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.phymode);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.rate);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.pwr);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.stbc);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ldpc);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ibf);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.ebf);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.wlan_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.aifs);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.gi);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.tx_path);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param.nss);
	band_idx = param.band_idx;
	if (!param.pkt_cnt)
		param.pkt_cnt = 0x8fffffff;
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, TxCount, param.pkt_cnt);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, PhyMode, param.phymode);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Mcs, param.rate);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Stbc, param.stbc);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Ldpc, param.ldpc);
#ifdef TXBF_SUPPORT
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, iTxBf, param.ibf);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, eTxBf, param.ebf);
#endif
	ate_ctrl->wcid_ref = param.wlan_id;
    // TODO: Need to modify
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, ipg_param.ipg, param.aifs);		//Fix me
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Sgi, param.gi);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, TxAntennaSel, param.tx_path);
	TESTMODE_SET_PARAM(ate_ctrl, band_idx, Nss, param.nss);

	Channel = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Channel);
	Ch_Band = TESTMODE_GET_PARAM(ate_ctrl, band_idx, Ch_Band);
	PktBw = TESTMODE_GET_PARAM(ate_ctrl, band_idx, PerPktBW);
	SysBw = TESTMODE_GET_PARAM(ate_ctrl, band_idx, BW);

	if(param.rate == 32 && PktBw != BAND_WIDTH_40 && SysBw != BAND_WIDTH_40) {
		ret = -1;
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			 ("%s:Bandwidth must to be 40 at MCS 32\n", __FUNCTION__));
		goto err0;
	}
	os_zero_mem(&TxPower, sizeof(TxPower));

	TxPower.Power = param.pwr;
	TxPower.Channel = Channel;
	TxPower.Dbdc_idx = band_idx;
	TxPower.Band_idx = Ch_Band;
    
	ret = ate_ops->SetTxPower0(pAd, TxPower);
    ret = ate_ops->SetIPG(pAd, band_idx);
	ret = ate_ops->StartTx(pAd, param.band_idx);

err0:
	NdisMoveMemory(cmd_frame->Data + 2,(UCHAR *)&param.ext_id, sizeof(param.ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(param.ext_id), ret);
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, band_idx:%u, pkt_cnt:%u, phy:%u, mcs:%u, stbc:%u, ldpc:%u\n",
			__FUNCTION__, param.band_idx, param.pkt_cnt, param.phymode, 
			param.rate, param.stbc, param.ldpc));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, ibf:%u, ebf:%u, wlan_id:%u, aifs:%u, gi:%u, tx_path:%x, nss:%x\n",
			__FUNCTION__, param.ibf, param.ebf, param.wlan_id, param.aifs,
			param.gi, param.tx_path, param.nss));
	return ret;
}

static INT32 hqa_start_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	UINT32 rx_path = 0;
	UCHAR own_mac[MAC_ADDR_LEN];
	NdisZeroMemory(own_mac, MAC_ADDR_LEN);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);
	EthGetParamAndShiftBuff(FALSE, MAC_ADDR_LEN, &data, (UCHAR *)&own_mac);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&rx_path);
	ret = ate_ops->SetRxAntenna(pAd, rx_path, band_idx);
	ret = ate_ops->SetAutoResp(pAd, band_idx, own_mac, 1);
	ret = ate_ops->StartRx(pAd, band_idx);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, param num:%u, band_sel:%u, rx_path:%x, mac:%02x:%02x:%02x:%02x:%02x:%02x\n",
			__FUNCTION__,param_num, band_idx, rx_path, PRINT_MAC(own_mac)));

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 2 + sizeof(ext_id), ret);
	return ret;
}

static INT32 hqa_stop_tx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);

	ret = ate_ops->StopTx(pAd, band_idx);

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 6, ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, band_idx:%u\n", __FUNCTION__, band_idx));
	return ret;
}

static INT32 hqa_stop_rx_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 param_num = 0;
	UINT32 band_idx = 0;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;

	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&param_num);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&band_idx);

	ret = ate_ops->StopRx(pAd, band_idx);
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	ResponseToQA(cmd_frame, wrq, 6, ret);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("%s, band_idx:%u\n", __FUNCTION__, band_idx));
	return ret;
}


#ifdef TXBF_SUPPORT
static INT32 hqa_iBFGetStatus_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UINT32 ext_id = 0;
	UINT32 u4Status = 0;
	

    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);

    if(!ate_ctrl->txbf_info)
		goto HQA_TAG_DNC_FAIL;

    u4Status = ate_ctrl->iBFCalStatus;
    
    ate_ctrl->Mode &= ~fATE_IN_BF;
	os_free_mem(ate_ctrl->txbf_info);

	//MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, val:%x\n", __FUNCTION__, u4Status));

    ext_id   = PKTL_TRAN_TO_HOST(ext_id);
    u4Status = PKTL_TRAN_TO_HOST(u4Status);
    
	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	NdisMoveMemory(cmd_frame->Data + 6, &u4Status, 4);
	
HQA_TAG_DNC_FAIL:	
	ResponseToQA(cmd_frame, wrq, 10, ret);

	return ret;
}


static INT32 hqa_iBFSetValue_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
	UINT32 ext_id = 0;
	UINT32 u4Action = 0;
	UINT32 u4InArg[8] = {0,0,0,0,0,0,0,0};
	RTMP_STRING *cmd;


	ate_ctrl->txbf_info = NULL;
	
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_IBF_CMD_FAIL;
	}

    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Action);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[0]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[1]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[2]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[3]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[4]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[5]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[6]);
    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4InArg[7]);

    switch (u4Action)
    {
    case ATE_TXBF_INIT:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);

	    SetATETxBfDutInitProc(pAd, cmd);
        break;
    case ATE_CHANNEL:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        if (u4InArg[1] == 1)
        {
            sprintf(cmd, "%d:1" , (UCHAR)u4InArg[0]);
        }
        else
        {
	        sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
        }
	    
        SetATEChannel(pAd, cmd);
        break;
    case ATE_TX_MCS:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
	    
        SetATETxMcs(pAd, cmd);
        break;
    case ATE_TX_POW0:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
	    
        SetATETxPower0(pAd, cmd);
        break;
    case ATE_TX_ANT:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
	    
        SetATETxAntenna(pAd, cmd);
        break;
    case ATE_RX_FRAME:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "RXFRAME");
	    
        SetATE(pAd, cmd);
        break;
    case ATE_RX_ANT:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
	    
        SetATERxAntenna(pAd, cmd);
        break;
    case ATE_TXBF_LNA_GAIN:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
	    sprintf(cmd, "%d" , (UCHAR)u4InArg[0]);
	    
        SetATETxBfLnaGain(pAd, cmd);
        break;
    case ATE_IBF_PHASE_COMP:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // BW:DBDC idx:Group:Read from E2P:Dis compensation 
	    sprintf(cmd, "%02x:%02x:%02x:%02x:%02x" , 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2], u4InArg[3],
	                 u4InArg[4]);

        if (SetATEIBfPhaseComp(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_PHASE_COMP is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    case ATE_IBF_TX:
        u4InArg[2] = 0; // for test purpose
        
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // fgBf:WLAN idx:Txcnt
	    sprintf(cmd, "%02x:%02x:%02x" , 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2]);

        if (SetATETxPacketWithBf(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_TX is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    case ATE_IBF_PROF_UPDATE:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Pfmu idx:Nr:Nc
	    sprintf(cmd, "%02x:%02x:%02x" , 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2]);

        if (SetATEIBfProfileUpdate(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_PROF_UPDATE is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }

        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Wlan Id:EBf:IBf:Mu:PhaseCalFlg
	    sprintf(cmd, "01:00:01:00:01");
        
        if (Set_TxBfTxApply(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" IBF flag setting in WTBL is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    case ATE_EBF_PROF_UPDATE:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Pfmu idx:Nr:Nc
	    sprintf(cmd, "%02x:%02x:%02x" , 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2]);

        if (SetATEEBfProfileConfig(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_EBF_PROF_UPDATE is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    case ATE_IBF_INST_CAL:
        ate_ctrl->Mode |= fATE_IN_BF;
        
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level
	    sprintf(cmd, "%02x:%02x:%02x:%02x:%02x", 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2], u4InArg[3],
	                 u4InArg[4]);

        if (SetATEIBfInstCal(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_GD_CAL is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    case ATE_IBF_INST_VERIFY:
        ate_ctrl->Mode |= fATE_IN_BF;

        u4InArg[3] = 4; // iBF phase verification with instrument
        u4InArg[4] = 1; // Force LNA gain is middle gain
        
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Group idx:Group_L_M_H:fgSX2:Calibration type:Lna level
	    sprintf(cmd, "%02x:%02x:%02x:%02x:%02x", 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2], u4InArg[3],
	                 u4InArg[4]);

        if (SetATEIBfInstCal(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_INST_VERIFY is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }

        break;
    case ATE_TXBF_GD_INIT:
        break;
    case ATE_IBF_PHASE_E2P_UPDATE:
        memset(cmd, 0x00, HQA_BF_STR_SIZE);
        // Group idx:fgSX2:E2P update type
	    sprintf(cmd, "%02x:%02x:%02x", 
	                 u4InArg[0], u4InArg[1], 
	                 u4InArg[2]);

        pAd->fgCalibrationFail = FALSE; // Enable EEPROM write of calibrated phase
        
        if (SetATETxBfPhaseE2pUpdate(pAd, cmd) == FALSE)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" ATE_IBF_PHASE_E2P_UPDATE is failed!!\n"));
            ret = NDIS_STATUS_FAILURE;
            goto HQA_IBF_CMD_FAIL;
        }
        break;
    default:
        break;

    }

	//MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, Action ID : %d, str:%s\n", __FUNCTION__, u4Action, cmd));
	os_free_mem(cmd);

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	
HQA_IBF_CMD_FAIL:	
	ResponseToQA(cmd_frame, wrq, 6, ret);

	return ret;

}


static INT32 hqa_iBFChanProfUpdate_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId, u4Subcarr, fgFinalData;
    UINT32 i2H11, i2AngleH11, i2H21, i2AngleH21, i2H31, i2AngleH31, i2H41, i2AngleH41;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_PROFILE_UPDATE_FAIL;
	}

    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Subcarr);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&fgFinalData);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H11);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH11);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H21);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH21);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H31);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH31);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2H41);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&i2AngleH41);
	
	
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x:%03x", 
                                     u4PfmuId, u4Subcarr, fgFinalData, i2H11, i2AngleH11, i2H21, i2AngleH21, 
                                     i2H31, i2AngleH31, i2H41, i2AngleH41);

	if (SetATETxBfChanProfileUpdate(pAd, cmd) == FALSE)
    {
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" SetATETxBfChanProfileUpdate is failed!!\n"));
        ret = NDIS_STATUS_FAILURE;
        goto HQA_PROFILE_UPDATE_FAIL;
    }

	//MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, str:%s\n", __FUNCTION__, cmd));
	os_free_mem(cmd);

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	
HQA_PROFILE_UPDATE_FAIL:	
	ResponseToQA(cmd_frame, wrq, 6, ret);

	return ret;
}


static INT32 hqa_iBFChanProfUpdateAll_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId, u4Temp;

    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Temp);

	if (TxBfProfileDataWrite20MAll(pAd, u4PfmuId, data) == FALSE)
    {
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,(" SetATETxBfChanProfileUpdate is failed!!\n"));
        ret = NDIS_STATUS_FAILURE;
        goto HQA_PROFILE_UPDATE_FAIL;
    }

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, str:%d\n", __FUNCTION__, u4PfmuId));

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	
HQA_PROFILE_UPDATE_FAIL:	
	ResponseToQA(cmd_frame, wrq, 6, ret);

	return ret;
}


static INT32 hqa_iBFProfileRead_ext(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	UCHAR *data = cmd_frame->Data;
	UINT32 ext_id = 0;
	UINT32 u4PfmuId, u4Subcarr;
	RTMP_STRING *cmd;
	os_alloc_mem(pAd, (UCHAR **)&cmd, sizeof(CHAR)*(HQA_BF_STR_SIZE));
	if (!cmd){
		ret = NDIS_STATUS_RESOURCES;
		goto HQA_TAG_DNC_FAIL;
	}

    EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&ext_id);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4PfmuId);
	EthGetParamAndShiftBuff(TRUE, sizeof(UINT32), &data, (UCHAR *)&u4Subcarr);
	
	memset(cmd, 0x00, HQA_BF_STR_SIZE);
	sprintf(cmd, "%03x:%03x", (UCHAR)u4PfmuId, (UCHAR)u4Subcarr);

	SetATETxBfProfileRead(pAd, cmd);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,("%s, str:%s\n", __FUNCTION__, cmd));
	os_free_mem(cmd);

	NdisMoveMemory(cmd_frame->Data + 2, (UCHAR *)&ext_id, sizeof(ext_id));
	NdisMoveMemory(cmd_frame->Data + 6, (UCHAR *)&pAd->prof, sizeof(PFMU_DATA));
	
HQA_TAG_DNC_FAIL:	
	ResponseToQA(cmd_frame, wrq, (6 + sizeof(PFMU_DATA)), ret);

	return ret;
}
#endif /* TXBF_SUPPORT */

static HQA_CMD_HANDLER hqa_ext_cmd_set[] =
{
	NULL,
	hqa_set_channel_ext,	/* 0x00000001 */
	hqa_set_txcontent_ext,	/* 0x00000002 */
	hqa_start_tx_ext,	    /* 0x00000003 */
	hqa_start_rx_ext,	    /* 0x00000004 */
	hqa_stop_tx_ext,	    /* 0x00000005 */
	hqa_stop_rx_ext,	    /* 0x00000006 */
	NULL,                   /* 0x00000007 */
#ifdef TXBF_SUPPORT	
	hqa_iBFSetValue_ext,    /* 0x00000008 */
	hqa_iBFGetStatus_ext,   /* 0x00000009 */
	hqa_iBFChanProfUpdate_ext,    /* 0x0000000A */
	hqa_iBFProfileRead_ext,       /* 0x0000000B */
	hqa_iBFChanProfUpdateAll_ext, /* 0x0000000C */
#endif /* TXBF_SUPPORT */	
};

static INT32 hqa_ext_cmds(PRTMP_ADAPTER pAd, RTMP_IOCTL_INPUT_STRUCT *wrq, HQA_CMD_FRAME *cmd_frame)
{
	INT32 ret = 0;
	INT32 idx = 0;

	NdisMoveMemory((PUCHAR)&idx, (PUCHAR)&cmd_frame->Data, 4);
	idx = PKTL_TRAN_TO_HOST(idx);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s: 0x%x\n", __FUNCTION__, idx));

	if (idx >= (sizeof(hqa_ext_cmd_set)/sizeof(HQA_CMD_HANDLER))) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: cmd idx 0x%x is over bounded\n",
			__FUNCTION__, idx));
		return ret;
	}

	if (hqa_ext_cmd_set[idx] != NULL)
		ret = (*hqa_ext_cmd_set[idx])(pAd, wrq, cmd_frame);
	else
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_WARN,
			("%s: cmd idx 0x%x is not supported\n",
			__FUNCTION__, idx));

	return ret;
}

static HQA_CMD_HANDLER HQA_CMD_SET6[] =
{
	/* cmd id start from 0x1600 */
	hqa_ext_cmds,	/* 0x1600 */
};


static HQA_CMD_TABLE HQA_CMD_TABLES[] = {
	{
		HQA_CMD_SET0,
		sizeof(HQA_CMD_SET0) / sizeof(HQA_CMD_HANDLER),
		0x1000,
	},
	{
		HQA_CMD_SET1,
		sizeof(HQA_CMD_SET1) / sizeof(HQA_CMD_HANDLER),
		0x1100,
	},
	{
		HQA_CMD_SET2,
		sizeof(HQA_CMD_SET2) / sizeof(HQA_CMD_HANDLER),
		0x1200,
	},
	{
		HQA_CMD_SET3,
		sizeof(HQA_CMD_SET3) / sizeof(HQA_CMD_HANDLER),
		0x1300,
	},
	{
		HQA_CMD_SET4,
		sizeof(HQA_CMD_SET4) / sizeof(HQA_CMD_HANDLER),
		0x1400,
	},
	{
		HQA_CMD_SET5,
		sizeof(HQA_CMD_SET5) / sizeof(HQA_CMD_HANDLER),
		0x1500,
	},
#ifdef TXBF_SUPPORT
	{
		HQA_TXBF_CMDS,
		sizeof(HQA_TXBF_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1540,
	},
#ifdef CFG_SUPPORT_MU_MIMO
	{
		HQA_TXMU_CMDS,
		sizeof(HQA_TXMU_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1560,
	},
#endif
#endif
	{
		HQA_ICAP_CMDS,
		sizeof(HQA_ICAP_CMDS) / sizeof(HQA_CMD_HANDLER),
		0x1580,
	},
	{
		HQA_CMD_SET6,
		sizeof(HQA_CMD_SET6) / sizeof(HQA_CMD_HANDLER),
		0x1600,
	},

};

UINT32 HQA_CMDHandler(
	RTMP_ADAPTER *pAd,
	RTMP_IOCTL_INPUT_STRUCT *Wrq,	
	HQA_CMD_FRAME *HqaCmdFrame)
{
	UINT32 Status = NDIS_STATUS_SUCCESS;
	UINT16 CmdId;
	UINT32 TableIndex = 0;
	UINT32 ATEMagicNum = 0;

	ATEMagicNum = PKTL_TRAN_TO_HOST(HqaCmdFrame->MagicNo);
	if(ATEMagicNum != HQA_CMD_MAGIC_NO)
		return TM_STATUS_NOTSUPPORT;

	CmdId = PKTS_TRAN_TO_HOST(HqaCmdFrame->Id);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: Command_Id = 0x%04x, testmode_ioctl\n", __FUNCTION__, CmdId));
	while (TableIndex < (sizeof(HQA_CMD_TABLES) / sizeof(HQA_CMD_TABLE)))
	{
 		UINT32 CmdIndex = 0;
 		CmdIndex = CmdId - HQA_CMD_TABLES[TableIndex].CmdOffset;
 		if (CmdIndex < HQA_CMD_TABLES[TableIndex].CmdSetSize)
 		{
			HQA_CMD_HANDLER *pCmdSet;

			pCmdSet = HQA_CMD_TABLES[TableIndex].CmdSet;

			if (pCmdSet[CmdIndex] != NULL)
				Status = (*pCmdSet[CmdIndex])(pAd, Wrq, HqaCmdFrame);
			break;
		}
		TableIndex++;
	}
	HqaCmdFrame->Type = TM_CMDRSP;
	return Status;
}
#else
VOID 
DummyFunction()
{
}
#endif /*(SUPPORT_QA_TOOL ==1) || !defined(COMPOS_WIN)*/
