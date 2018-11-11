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
	ate_agent.c
*/

#include "rt_config.h"

#define MCAST_WCID_TO_REMOVE 0	//Pat: TODO

#if defined(MT7615) || defined(MT7622)
#define ATE_ANT_USER_SEL 0x80000000
#endif

/*  CCK Mode */
static CHAR CCKRateTable[] = {0, 1, 2, 3, 8, 9, 10, 11, -1};

/*  OFDM Mode */
static CHAR OFDMRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, -1};

/*  HT Mixed Mode */
static CHAR HTMIXRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 
                                24, 25, 26, 27, 28, 29, 30, 31, 32, -1};

/* VHT Mode */
static CHAR VHTRateTable[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1};


UINT_8  Addr1[6] = {0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
UINT_8  Addr2[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
UINT_8  Addr3[6] = {0x00, 0x22, 0x22, 0x22, 0x22, 0x22};
UCHAR   g_BFBackOffMode = 4; // BF Backoff Mode: 2/3/4: apply 2T/3T/4T value in BF backoff table

#if defined(TXBF_SUPPORT) && defined(MT_MAC)
UCHAR TemplateFrame[32] = {0x88, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
                           0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
                           0x00, 0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

#else
static UCHAR TemplateFrame[32] = {0x08, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
					 		      0xFF, 0xFF, 0x00, 0xAA, 0xBB, 0x12, 0x34, 0x56,
					 		      0x00,	0x11, 0x22, 0xAA, 0xBB, 0xCC, 0x00, 0x00,
					 		      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */

INT32 SetTxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetRxStop(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


#ifdef DBG
VOID ATE_QA_Statistics(RTMP_ADAPTER *pAd, RXWI_STRUC *pRxWI, RXINFO_STRUC *pRxInfo, PHEADER_802_11 pHeader)
{

}


INT32 SetEERead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetEEWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetBBPRead(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetBBPWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetRFWrite(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;



}
#endif /* DBG */


VOID EEReadAll(PRTMP_ADAPTER pAd, UINT16 *Data)
{
	UINT16 Offset = 0;
	UINT16 Value;

	for (Offset = 0; Offset < (EEPROM_SIZE >> 1);)
	{
		RT28xx_EEPROM_READ16(pAd, (Offset << 1), Value);
		Data[Offset] = Value;
		Offset++;
	}
}

 INT32 SetATEDaByWtblTlv(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT32 Octet;
	RTMP_STRING *Value;
	/* Tag = 0, Generic */
	CMD_WTBL_GENERIC_T		rWtblGeneric = {0};

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: Da = %s\n", __FUNCTION__, Arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */

	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr1[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef MT_MAC
	/* WIndex = 1, WTBL1: PeerAddress */
	rWtblGeneric.u2Tag = WTBL_GENERIC;
	rWtblGeneric.u2Length = sizeof(CMD_WTBL_GENERIC_T);
	NdisMoveMemory(rWtblGeneric.aucPeerAddress , ATECtrl->Addr3, MAC_ADDR_LEN);
	CmdExtWtblUpdate(pAd, 1 /**/, SET_WTBL, (PUCHAR)&rWtblGeneric, sizeof(CMD_WTBL_GENERIC_T));
#endif
	
#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (DA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3],
		ATECtrl->Addr1[4], ATECtrl->Addr1[5]));

#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}
INT32 SetATEQid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	USHORT q_idx;

	q_idx = simple_strtol(Arg, 0, 10);
	ATECtrl->QID = q_idx;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: QID:%u\n", __FUNCTION__, q_idx));
	return TRUE;
}

INT32 SetATETxSEnable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT8 band_idx = 0;
	UINT32 param = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	param = simple_strtol(Arg, 0, 10);

	ATECtrl->txs_enable = param;
	return TRUE;
	err0:
		return FALSE;
}

INT32 SetATERxFilter(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	const INT param_num = 3;
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	MT_RX_FILTER_CTRL_T rx_filter;
	UINT32 input[param_num];
	CHAR *value;
	INT i;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, arg));
	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	
	for (i=0;i<param_num;i++)
		input[i] = 0;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) {
		if (i==param_num)
			break;
		input[i++] = simple_strtol(value, 0, 16);
	}
	
	os_zero_mem(&rx_filter, sizeof(rx_filter));
	rx_filter.bPromiscuous = input[0];
	rx_filter.bFrameReport= input[1];
	rx_filter.filterMask = input[2];
	rx_filter.BandIdx = band_idx;
	MtATESetRxFilter(pAd, rx_filter);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
			("%s: Promiscuous:%x, FrameReport:%x, filterMask:%x\n"
			, __FUNCTION__, rx_filter.bPromiscuous, rx_filter.bFrameReport, rx_filter.filterMask));
	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATERxStream(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	UINT32 param = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	param = simple_strtol(Arg, 0, 10);

	Ret = MtATESetRxPath(pAd, param, band_idx);
	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATETxStream(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	UINT32 param = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	param = simple_strtol(Arg, 0, 10);

	Ret = MtATESetTxStream(pAd, param, band_idx);
	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATEMACTRx(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	const INT param_num = 3;
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	UINT32 input[param_num];
	CHAR *value;
	INT i;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	for (i=0;i<param_num;i++)
		input[i] = 0;

	for (i=0, value = rstrtok(arg,":"); value; value = rstrtok(NULL,":")) {
		if (i==param_num)
			break;
		input[i++] = simple_strtol(value, 0, 16);
	}

	Ret = MtATESetMacTxRx(pAd, input[0], input[1], band_idx);
	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATEMPSStart(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;
	UINT enable;
	INT32 ret = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, arg));
	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	
	enable = simple_strtol(arg, 0, 10);
	if (enable)
		ret = ATEOp->MPSTxStart(pAd, band_idx);
	else
		ret = ATEOp->MPSTxStop(pAd, band_idx);
	if (!ret)
		return TRUE;
	err0:
	return FALSE;
}

INT32 SetATEMPSDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT ret = 0;
	UINT32 band_idx = simple_strtol(arg, 0, 10);
	ret = MT_SetATEMPSDump(pAd, band_idx);
	if (!ret)
		return TRUE;
	return FALSE;
}

static INT32 SetATEMPSParam(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg, UINT type)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	INT num_items = 0;
	CHAR *value;
	RTMP_STRING *tmp = arg;
	UINT32 *mps_setting = NULL;
	INT i;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Parm = %s\n", __FUNCTION__, arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	value = rstrtok(tmp,":");
	if (!value)
		goto err0;
	printk("value:%s, arg:%s, tmp:%s\n", value, arg, tmp);
	num_items = simple_strtol(value, 0, 10);
	if (!num_items)
		goto err0;

	Ret = os_alloc_mem(pAd, (UCHAR **)&mps_setting, sizeof(UINT32)*(num_items));
	if (Ret)
		goto err1;
	
	for (i=0, value = rstrtok(NULL,":"); value; value = rstrtok(NULL,":")) {
		if (!value)
			break;
		if (i==num_items)
			break;
		mps_setting[i++] = simple_strtol(value, 0, 10);
	}
	
	if (i!=num_items)
		goto err2;
	
	ATEOp->MPSSetParm(pAd, type, band_idx, num_items, mps_setting);
	
	if (mps_setting)
		os_free_mem(mps_setting);
	return TRUE;
	err2:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s: Number of items %d is not matched with number of params %d\n"
		, __FUNCTION__, num_items, i));
	if (mps_setting)
		os_free_mem(mps_setting);
	err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s: Mem allocate fail\n"
		, __FUNCTION__));
	err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s:[%u]Format: num_itmes:param1:param2:...\n"
		, __FUNCTION__, type));
	
	return FALSE;
}

INT32 SetATEMPSPhyMode(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_PHYMODE);
	return ret;
}

INT32 SetATEMPSRate(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_RATE);
	return ret;
}


INT32 SetATEMPSPath(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_PATH);
	return ret;
}

INT32 SetATEMPSPayloadLen(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_PAYLOAD_LEN);
	return ret;
}

INT32 SetATEMPSPktCnt(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_TX_COUNT);
	return ret;
}

INT32 SetATEMPSPwr(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_PWR_GAIN);
	return ret;
}

INT32 SetATEMPSNss(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_NSS);
	return ret;
}

INT32 SetATEMPSPktBw(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 ret = 0;
	ret = SetATEMPSParam(pAd, arg, MPS_PKT_BW);
	return ret;
}

INT32 SetATELOGDump(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_LOG_DUMP_CB *log_cb = &ATECtrl->log_dump[0];
	UINT32 log_type;

	log_type = simple_strtol(Arg, 0, 10);

	if (log_type >= ATE_LOG_TYPE_NUM)
		return FALSE;

	log_cb = &ATECtrl->log_dump[log_type-1];
	MT_ATEDumpLog(pAd, log_cb, log_type);	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: log_type:%08x, driver:%08x\n"
		, __FUNCTION__, log_type, ATECtrl->en_log));
	return TRUE;
}

INT32 SetATELOGEnable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 log_type;
	INT32 Ret = 0;

	log_type = simple_strtol(Arg, 0, 10);
	Ret = ATEOp->LogOnOff(pAd, log_type, TRUE, 2000);
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
		("%s: log_type:%u, driver:%08x\n"
		, __FUNCTION__, log_type, ATECtrl->en_log));
	return TRUE;
}

INT32 SetATELOGDisable(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 log_type;
	INT32 Ret = 0;

	log_type = simple_strtol(Arg, 0, 10);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: log_type:%u, driver:%08x\n", __FUNCTION__, log_type, ATECtrl->en_log));
	
	Ret = ATEOp->LogOnOff(pAd, log_type, FALSE, 0);
	return TRUE;
}

INT32 SetATEDeqCnt(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
#ifdef ATE_TXTHREAD
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	INT deq_cnt;

	deq_cnt = simple_strtol(Arg, 0, 10);
    ATECtrl->deq_cnt = deq_cnt;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s: deq_cnt:%d\n", __FUNCTION__, deq_cnt));
#endif
	return TRUE;
}

INT32 SetATEDa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
 	return SetATEDaByWtblTlv(pAd, Arg);
}

INT32 SetATESa(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	RTMP_STRING *Value;
	INT32 Octet;
	struct wifi_dev *wdev = pAd->wdev_list[ATECtrl->wdev_idx];
	UCHAR BandIdx;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Sa = %s\n", __FUNCTION__, Arg));

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}
#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr3[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef MT_MAC
	/* Set the specific MAC to ASIC */
	// TODO: Hanmin H/W HAL offload, below code is replaced by new code above, right PIC needs further check
	
#ifdef CONFIG_AP_SUPPORT
	BandIdx = HcGetBandByWdev(wdev);
	AsicDevInfoUpdate(
		pAd, 
		0x0, 
		ATECtrl->Addr3, 
		BandIdx, 
		TRUE, 
		DEVINFO_ACTIVE_FEATURE);
#endif /* CONFIG_AP_SUPPORT */

#endif
#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (SA = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3],
		ATECtrl->Addr3[4], ATECtrl->Addr3[5]));
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


INT32 SetATEBssid(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	RTMP_STRING *Value;
	INT32 Octet;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17 */
	if (strlen(Arg) != 17)
		return FALSE;

	for (Octet = 0, Value = rstrtok(Arg, ":"); Value; Value = rstrtok(NULL, ":"))
	{
		/* sanity check */
		if ((strlen(Value) != 2) || (!isxdigit(*Value)) || (!isxdigit(*(Value+1))))
		{
			return FALSE;
		}

#ifdef CONFIG_AP_SUPPORT
		AtoH(Value, &ATECtrl->Addr2[Octet++], 1);
#endif /* CONFIG_AP_SUPPORT */

	}

	/* sanity check */
	if (Octet != MAC_ADDR_LEN)
	{
		return FALSE;
	}

#ifdef CONFIG_AP_SUPPORT
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (BSSID = %02x:%02x:%02x:%02x:%02x:%02x)\n", __FUNCTION__,
		ATECtrl->Addr2[0], ATECtrl->Addr2[1], ATECtrl->Addr2[2], ATECtrl->Addr2[3],
		ATECtrl->Addr2[4], ATECtrl->Addr2[5]));
#endif /* CONFIG_AP_SUPPORT */

	return TRUE;
}


INT32 SetATEInitChan(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetADCDump(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetATETxPower0(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Power0 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

    ATECtrl->TxPower0 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = MT_ATEGetBandIdxByIf(pAd);

	Ret = ATEOp->SetTxPower0(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxPower1(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Power1 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

	ATECtrl->TxPower1 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = MT_ATEGetBandIdxByIf(pAd);

	Ret = ATEOp->SetTxPower1(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATETxPower2(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Power2 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

	ATECtrl->TxPower2 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = MT_ATEGetBandIdxByIf(pAd);

	Ret = ATEOp->SetTxPower2(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATETxPower3(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	ATE_TXPOWER TxPower;
	CHAR Power;
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Power3 = %s\n", __FUNCTION__, Arg));

	Power = simple_strtol(Arg, 0, 10);

	ATECtrl->TxPower3 = Power;
	os_zero_mem(&TxPower, sizeof(TxPower));
	TxPower.Power = Power;
	TxPower.Dbdc_idx = MT_ATEGetBandIdxByIf(pAd);

	Ret = ATEOp->SetTxPower3(pAd, TxPower);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

#ifdef ABSOLUTE_POWER_TEST
INT32 SetATEForceTxPower(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT8 ParamIdx;
	CHAR  *value = 0;
	UINT8 ucBandIdx = 0;
	INT_8 cTxPower = 0;
	UINT8 ucPhyMode = 0;
	UINT8 ucTxRate = 0;
	UINT8 ucBW = 0;
	INT32 Ret;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	/* obtain DBDC Band Index */
	ucBandIdx = MT_ATEGetBandIdxByIf(pAd);

	/* Sanity check for DBDC Band Index */
	if (ucBandIdx < 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Invalid Band Index !! \n", __FUNCTION__));
		goto err0;
	}

	/* Sanity check for input parameter */
	if (!Arg) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No Parameters !! \n", __FUNCTION__));
		goto err1;
	}

	/* Sanity check for input parameter format */
	if(strlen(Arg) != 11) {   
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong Parameter Format !!\n", __FUNCTION__));
		goto err1;
	}
	
	/* Parsing input parameter */ 
	for (ParamIdx = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), ParamIdx++) {
		switch (ParamIdx) {
			case 0:
				ucPhyMode = simple_strtol(value, 0, 10); /* 2-bit format */
				break;
			case 1:
				ucTxRate = simple_strtol(value, 0, 10);  /* 2-bit format */
				break;
			case 2:
				ucBW = simple_strtol(value, 0, 10);      /* 2-bit format */
				break;
			case 3:
				cTxPower = simple_strtol(value, 0, 10);  /* 2-bit format */
				break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Set Too Much Parameters !!\n", __FUNCTION__));
				goto err1;
		}
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Band(%d), TxMode(%d), MCS(%d), BW(%d), TxPower(%d)\n",
															__FUNCTION__, ucBandIdx, ucPhyMode, ucTxRate, ucBW, cTxPower));

	/* Command Handler for Force Power Control */
	Ret = ATEOp->SetTxForceTxPower(pAd, ucBandIdx, cTxPower, ucPhyMode, ucTxRate, ucBW);

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
	err1:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "Please input parameter via format \"Phymode:TxRate:BW:TxPower\"\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "Phymode:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) 0: CCK, 1: OFDM, 2: HT-MIXED, 3: HT-GREEN, 4: VHT\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "TxRate:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) CCK: 00~03, OFDM: 00~07, HT-MIXED: 00~07, HT-GREEN: 00~07, VHT: 00~09\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "BW:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) 0: BW20, 1: BW40, 2: BW80, 3:BW160\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "TxPower:\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KGRN "    (2-digit) absolute Tx power (unit: 0.5dB)\n" KNRM));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (KRED "Ex: iwpriv ra0 set ATEFORCETXPOWER=02:00:00:16\n" KNRM));
		return FALSE;
		
}
#endif /* ABSOLUTE_POWER_TEST */

INT32 SetATETxPowerEvaluation(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATETxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 Ant = 1;
	UINT8 band_idx = 0;
 	const INT idx_num = 2;
 	UINT32 param[idx_num];
	UINT8 loop_index = 0;
	CHAR *value;
#if defined(MT7615) || defined(MT7622)
    UINT32 mode = 0;
#endif

    /* Sanity check for input parameter */
    if (Arg == NULL) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        goto err0;
    }

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

    /* TX path setting */
    if (!strchr(Arg, ':')) {
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Ant = %s\n", __FUNCTION__, Arg));

        Ant = simple_strtol(Arg, 0, 10);
    } else {
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Mode:Value = %s\n", __FUNCTION__, Arg));

        for (loop_index=0; loop_index<idx_num; loop_index++)
            param[loop_index] = 0;

        for (loop_index=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL, ":")) {
            if (!value)
                break;
            if (loop_index == idx_num)
                break;
            param[loop_index] = simple_strtol(value, 0, 10);
            loop_index++;
        }
#if defined(MT7615) || defined(MT7622)
        mode = param[0];

        if (mode == ANT_MODE_SPE_IDX) {
            Ant = param[1] | ATE_ANT_USER_SEL;
        } else {
            Ant = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
        }
#else
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No need to set Spe_idx.\n", __FUNCTION__));
        goto err0;
#endif /* defined(MT7615) || defined(MT7622) */
    }

    Ret = ATEOp->SetTxAntenna(pAd, Ant, band_idx);

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}


INT32 SetATERxAntenna(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	CHAR Ant;
	UINT8 band_idx = 0;

    /* Sanity check for input parameter */
    if (Arg == NULL) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        goto err0;
    }

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Ant = %s\n", __FUNCTION__, Arg));

	Ant = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetRxAntenna(pAd, Ant, band_idx);

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}


INT32 Default_Set_ATE_TX_FREQ_OFFSET_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATETxFreqOffset(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UINT32 FreqOffset;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: FreqOffset = %s\n", __FUNCTION__, Arg));

	FreqOffset = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetTxFreqOffset(pAd, FreqOffset);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 Default_Set_ATE_TX_BW_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATETxLength(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UINT32 TxLength;
	UINT8 band_idx = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxLength = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	TxLength = simple_strtol(Arg, 0, 10);

	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxLength, TxLength);

	return TRUE;
	err0:
	return FALSE;
}


INT32 SetATETxCount(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UINT8 band_idx = 0;
	UINT32 TxCount = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxCount = %s\n", __FUNCTION__, Arg));
	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	TxCount = simple_strtol(Arg, 0, 10);

	if (TxCount == 0) {
#ifdef RTMP_PCI_SUPPORT
		TxCount = 0xFFFFFFFF;
#endif /* RTMP_MAC_PCI */
	}
	TESTMODE_SET_PARAM(ATECtrl, band_idx, TxCount, TxCount);
	return TRUE;
	err0:
	return FALSE;
}


INT32 CheckMCSValid(PRTMP_ADAPTER pAd, UCHAR PhyMode, UCHAR Mcs)
{
	int Index;
	PCHAR pRateTab = NULL;

	switch (PhyMode)
	{
		case MODE_CCK:
			pRateTab = CCKRateTable;
			break;
		case MODE_OFDM:
			pRateTab = OFDMRateTable;
			break;
		case MODE_HTMIX:
		case MODE_HTGREENFIELD:
			pRateTab = HTMIXRateTable;
			break;
		case MODE_VHT:
			pRateTab = VHTRateTable;
			break;
		default:
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unrecognizable Tx Mode %d\n", __FUNCTION__, PhyMode));
			return -1;
			break;
	}

	Index = 0;

	while (pRateTab[Index] != -1)
	{
		if (pRateTab[Index] == Mcs)
			return 0;
		Index++;
	}

	return -1;
}


INT32 SetATETxMcs(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Mcs, PhyMode = 0;
	INT32 Ret = 0;
	UINT8 band_idx = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Mcs = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	PhyMode = TESTMODE_GET_PARAM(ATECtrl, band_idx, PhyMode);

	Mcs = simple_strtol(Arg, 0, 10);

	Ret = CheckMCSValid(pAd, PhyMode, Mcs);

	if (Ret != -1) {
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Mcs, Mcs);
	} else {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s: Out of range, refer to rate table.\n"
				, __FUNCTION__));
		goto err0;
	}

	return TRUE;
	err0:
	return FALSE;
}

INT32 SetATEVhtNss(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Nss = 0;
	UINT8 band_idx = 0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Nss = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	Nss = simple_strtol(Arg, 0, 10);

	TESTMODE_SET_PARAM(ATECtrl, band_idx, Nss, Nss);

	return TRUE;
err0:
	return FALSE;
}

INT32 SetATETxLdpc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Ldpc;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		return FALSE;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Ldpc = %s\n", __FUNCTION__, Arg));

	Ldpc = simple_strtol(Arg, 0, 10);

	if (Ldpc > 1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Out of range (%d)\n", __FUNCTION__, Ldpc));
		return FALSE;
	} else 
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Ldpc, Ldpc);

	return TRUE;
}

INT32 SetATETxStbc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Stbc;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		return FALSE;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Stbc = %s\n", __FUNCTION__, Arg));

	Stbc = simple_strtol(Arg, 0, 10);

	if (Stbc > 1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Out of range (%d)\n", __FUNCTION__, Stbc));
		return FALSE;
	} else {
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Stbc, Stbc);
	}

	return TRUE;
}


INT32 SetATETxMode(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR PhyMode;
	UINT8 band_idx = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxMode = %s\n", __FUNCTION__, Arg));

	PhyMode = simple_strtol(Arg, 0, 10);
	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
		TESTMODE_SET_PARAM(ATECtrl, band_idx, PhyMode, PhyMode); 
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxMode = %x, band_idx:%u\n", __FUNCTION__, PhyMode, band_idx));
		return TRUE;
	err0:
	return FALSE;
}


INT32 SetATETxGi(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	UCHAR Sgi;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		return FALSE;
	
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Sgi = %s\n", __FUNCTION__, Arg));

	Sgi = simple_strtol(Arg, 0, 10);

	if (Sgi > 1) {
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, (":%s: Out of range (%d)\n", __FUNCTION__, Sgi));
		return FALSE;
	} else {
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Sgi, Sgi);
	}

	return TRUE;
}


INT32 SetATERxFer(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATETempSensor(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEReadRF(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{

	INT32 Ret = 0;

	Ret = ShowAllRF(pAd);

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}


INT32 SetATELoadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	UINT32 Ret = 0;
	RTMP_STRING *Src = EEPROM_BIN_FILE_NAME;
	RTMP_OS_FD Srcf;
	INT32 Retval;
	USHORT *WriteEEPROM = NULL;
	INT32 FileLength = 0;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);
	RTMP_OS_FS_INFO	OsFSInfo;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===> %s (value=%d)\n\n", __FUNCTION__, Value));

	Ret = os_alloc_mem(pAd,(PUCHAR *)&WriteEEPROM, EEPROM_SIZE);	//TODO verify
	if(Ret == NDIS_STATUS_FAILURE)
		return Ret;

	if (Value > 0)
	{
		/* zero the e2p buffer */
		NdisZeroMemory((PUCHAR)WriteEEPROM, EEPROM_SIZE);

		RtmpOSFSInfoChange(&OsFSInfo, TRUE);

		do
		{
			/* open the bin file */
			Srcf = RtmpOSFileOpen(Src, O_RDONLY, 0);

			if (IS_FILE_OPEN_ERR(Srcf))
			{
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s - Error opening file %s\n", __FUNCTION__, Src));
				break;
			}

			/* read the firmware from the file *.bin */
			FileLength = RtmpOSFileRead(Srcf, (RTMP_STRING *)WriteEEPROM, EEPROM_SIZE);

			if (FileLength != EEPROM_SIZE)
			{
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s : error file length (=%d) in e2p.bin\n",
					   __FUNCTION__, FileLength));
				break;
			}
			else
			{
				/* write the content of .bin file to EEPROM */
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
                {
                    UINT16 Index=0;
                    UINT16 Value=0;
                    INT32 E2pSize = 512;/* == 0x200 for PCI interface */
                    UINT16 TempData=0;

                    for (Index = 0 ; Index < (E2pSize >> 1); Index++)
                    {
                        /* "value" is especially for some compilers... */
                        TempData = le2cpu16(WriteEEPROM[Index]);
                        Value = TempData;
                        RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
                    }
                }
#else

//				rt_ee_write_all(pAd, WriteEEPROM);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
				Ret = TRUE;
			}
			break;
		} while(TRUE);

		/* close firmware file */
		if (IS_FILE_OPEN_ERR(Srcf))
		{
			;
		}
		else
		{
			Retval = RtmpOSFileClose(Srcf);

			if (Retval)
			{
				MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("--> Error %d closing %s\n", -Retval, Src));

			}
		}

		/* restore */
		RtmpOSFSInfoChange(&OsFSInfo, FALSE);
	}

	os_free_mem( WriteEEPROM);
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<=== %s (Ret=%d)\n", __FUNCTION__, Ret));

    return Ret;
}


#ifdef RTMP_EFUSE_SUPPORT
INT32 SetATELoadE2pFromBuf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	BOOLEAN Ret = FALSE;
	UINT32 Value = (UINT32)simple_strtol(Arg, 0, 10);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===> %s (Value=%d)\n\n", __FUNCTION__, Value));

	if (Value > 0)
	{
#if defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT)
            {
                UINT16 Index = 0;
                UINT16 Value = 0;
                INT32 E2PSize = 512;/* == 0x200 for PCI interface */
                UINT16 TempData = 0;

                for (Index = 0; Index < (E2PSize >> 1); Index++)
                {
                    /* "value" is especially for some compilers... */
		        	TempData = le2cpu16(pAd->EEPROMImage[Index]);
                    Value = TempData;
                    RT28xx_EEPROM_WRITE16(pAd, (Index << 1), Value);
                }
            }
#else

//		rt_ee_write_all(pAd, pAd->EEPROMImage);
#endif /* defined(RTMP_MAC_PCI) && defined(RTMP_PCI_SUPPORT) */
		Ret = TRUE;

	}

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<=== %s (Ret=%d)\n", __FUNCTION__, Ret));

    return Ret;

}
#endif /* RTMP_EFUSE_SUPPORT */


INT32 SetATEReadE2p(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret;
	UINT16 *Buffer = NULL;
	int i;

	ret = os_alloc_mem(pAd, (PUCHAR *)&Buffer, EEPROM_SIZE);
	if(ret == NDIS_STATUS_FAILURE)
		return ret;

	EEReadAll(pAd, (UINT16 *)Buffer);

	for (i = 0; i < (EEPROM_SIZE >> 1); i++)
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%4.4x ", *Buffer));
		if (((i+1) % 16) == 0)
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n"));
		Buffer++;
	}

	os_free_mem(Buffer);

	return TRUE;
}


INT32 SetATEAutoAlc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;


}


INT32 SetATEIpg(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 ret = 0;
	UINT32 value;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: IPG = %s\n", __FUNCTION__, Arg));

	value = simple_strtol(Arg, 0, 10);

#ifdef MT7615
	ATECtrl->ipg_param.ipg = value;
#endif

    ret = ATEOp->SetIPG(pAd, band_idx);

	if (!ret)
		return TRUE;
err0:
		return FALSE;

}


INT32 SetATEPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	RTMP_STRING *Value;

	Value = Arg;

	/* only one octet acceptable */
	if (strlen(Value) != 2)
		return FALSE;

	AtoH(Value, &(ATECtrl->Payload), 1);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Set_ATE_Payload_Proc (repeated pattern = 0x%2x)\n", ATECtrl->Payload));

	return TRUE;
}


INT32 SetATEFixedPayload(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL *ATECtrl = &pAd->ATECtrl;
	UINT32 Value;

	/* only one octet acceptable */
	Value = simple_strtol(Arg, 0, 10);

	if (Value == 0)
		ATECtrl->FixedPayload = 2;
	else
		ATECtrl->FixedPayload = 1;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: (Fixed Payload  = %u)\n", __FUNCTION__,
						ATECtrl->FixedPayload));

	return TRUE;
}


INT32 SetATETtr(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;

}

INT32 SetATEShow(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    ATE_CTRL      *ATECtrl = &pAd->ATECtrl;
#ifdef DBDC_MODE
    BAND_INFO     *Info = NULL;
#endif /* DBDC_MODE */
    RTMP_STRING   *Mode_String = NULL;
	RTMP_STRING   *TxMode_String = NULL;
    //HEADER_802_11 *phdr = NULL;
    UINT8         band_idx;
    UINT8         loop_index;
    INT           status = TRUE;
    CHAR          *value = 0;
    UCHAR         ExtendInfo = 0;

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
    if (Arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Please use parameter 0 for Summary INFO, 1 for Detail INFO!! \n", __FUNCTION__));
        return FALSE;
    }
    
    /* Parsing input parameter */ 
    for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++)
    {
        switch (loop_index)
        {
            case 0:
                ExtendInfo = simple_strtol(value, 0, 10);
                break;
            default:
            {
                status = FALSE;
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;
            }   
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ExtendInfo = %d \n", __FUNCTION__, ExtendInfo));
    
    band_idx = MT_ATEGetBandIdxByIf(pAd);

    if (band_idx < 0)
        goto err0;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: band_idx = %d !!!!!\n", __FUNCTION__, band_idx));

    /* initialize pointer to structure of parameters of Band1 */												
	if(band_idx == 0)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ATE Mode = 0x%x !!!!!\n", __FUNCTION__, ATECtrl->Mode));   
    }
#ifdef DBDC_MODE
    else
    {
        Info = &(ATECtrl->band_ext[band_idx-1]);
        if (Info != NULL)
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ATE Mode = 0x%x !!!!!\n", __FUNCTION__, Info->Mode));
        }
    }
#endif /* DBDC_MODE */

    /* check the ATE mode */
    if (band_idx == 0)
    {
        switch (ATECtrl->Mode)
    	{
    		case (fATE_IDLE):
    			Mode_String = "ATESTART";
    			break;
    		case (fATE_EXIT):
    			Mode_String = "ATESTOP";
    			break;
    		case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
    			Mode_String = "TXCONT";
    			break;
    		case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
    			Mode_String = "TXCARR";
    			break;
    		case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
    			Mode_String = "TXCARS";
    			break;
    		case (fATE_TX_ENABLE):
    			Mode_String = "TXFRAME";
    			break;
    		case (fATE_RX_ENABLE):
    			Mode_String = "RXFRAME";
    			break;
    		default:
    		{
    			Mode_String = "Unknown ATE mode";
    			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR! Unknown ATE mode!\n"));
    			break;
    		}
    	}
    }
#ifdef DBDC_MODE
    else
    {
        if (Info != NULL)
        {
            switch (Info->Mode)
            {
                case (fATE_IDLE):
                    Mode_String = "ATESTART";
                    break;
                case (fATE_EXIT):
                    Mode_String = "ATESTOP";
                    break;
                case ((fATE_TX_ENABLE)|(fATE_TXCONT_ENABLE)):
                    Mode_String = "TXCONT";
                    break;
                case ((fATE_TX_ENABLE)|(fATE_TXCARR_ENABLE)):
                    Mode_String = "TXCARR";
                    break;
                case ((fATE_TX_ENABLE)|(fATE_TXCARRSUPP_ENABLE)):
                    Mode_String = "TXCARS";
                    break;
                case (fATE_TX_ENABLE):
                    Mode_String = "TXFRAME";
                    break;
                case (fATE_RX_ENABLE):
                    Mode_String = "RXFRAME";
                    break;
                default:
                {
                    Mode_String = "Unknown ATE mode";
                    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR! Unknown ATE mode!\n"));
                    break;
                }
            }          
        }
    }
#endif /* DBDC_MODE */

    if(band_idx == 0)
    {       	
        switch (ATECtrl->PhyMode)
        {
            case 0:
                TxMode_String = "CCK";
                break;
            case 1:
                TxMode_String = "OFDM";
                break;
            case 2:
                TxMode_String = "HT-Mix";
                break;
            case 3:
                TxMode_String = "GreenField";
                break;
            default:
            {
                TxMode_String = "Unknown TxMode";
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR! Unknown TxMode!\n"));
                break;
            }
        }    	
    }
#ifdef DBDC_MODE
    else
    {
        if (Info != NULL)
        {
            switch (Info->PhyMode)
            {
                case 0:
                    TxMode_String = "CCK";
                    break;
                case 1:
                    TxMode_String = "OFDM";
                    break;
                case 2:
                    TxMode_String = "HT-Mix";
                    break;
                case 3:
                    TxMode_String = "GreenField";
                    break;
                default:
                {
                    TxMode_String = "Unknown TxMode";
                    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ERROR! Unknown TxMode!\n"));
                    break;
                }
            }    
        }
    }
#endif /* DBDC_MODE */

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                Band %d INFO\n", band_idx));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));

    if(band_idx == 0)
    {       
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATE Mode = %s\n", Mode_String));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxAntennaSel = 0x%x\n", ATECtrl->TxAntennaSel));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxAntennaSel = 0x%x\n", ATECtrl->RxAntennaSel));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BBPCurrentBW = %u\n", ATECtrl->BW));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("GI = %u\n", ATECtrl->Sgi));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS = %u\n", ATECtrl->Mcs));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxMode = %s\n", TxMode_String));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr1 = %02x:%02x:%02x:%02x:%02x:%02x\n",
    		ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr2 = %02x:%02x:%02x:%02x:%02x:%02x\n",
    		ATECtrl->Addr2[0], ATECtrl->Addr2[1], ATECtrl->Addr2[2], ATECtrl->Addr2[3], ATECtrl->Addr2[4], ATECtrl->Addr2[5]));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr3 = %02x:%02x:%02x:%02x:%02x:%02x\n",
    		ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3], ATECtrl->Addr3[4], ATECtrl->Addr3[5]));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel = %u\n", ATECtrl->Channel));

#ifdef DOT11_VHT_AC        
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_2nd = %u\n", ATECtrl->Channel_2nd));
#endif /* DOT11_VHT_AC */        

		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ch_Band = %d\n", ATECtrl->Ch_Band));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Control Channel = %d\n", ATECtrl->ControlChl));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxLength = %u\n", ATECtrl->TxLength));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxCount = %u\n", ATECtrl->TxCount));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RFFreqOffset = %u\n", ATECtrl->RFFreqOffset));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Payload Pattern = 0x%02x\n", ATECtrl->Payload));
    	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IPG = %dus\n", ATECtrl->ipg_param.ipg));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Duty Cycle = %d%%\n", ATECtrl->duty_cycle));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pkt Tx Time = %dus\n", ATECtrl->pkt_tx_time));
    }
#ifdef DBDC_MODE
    else
    {
        if (Info != NULL)
        {
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATE Mode = %s\n", Mode_String));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxAntennaSel = 0x%x\n", Info->TxAntennaSel));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RxAntennaSel = 0x%x\n", Info->RxAntennaSel));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BBPCurrentBW = %u\n", Info->BW));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("GI = %u\n", Info->Sgi));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MCS = %u\n", Info->Mcs));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxMode = %s\n", TxMode_String));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr1 = %02x:%02x:%02x:%02x:%02x:%02x\n",
                Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5]));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr2 = %02x:%02x:%02x:%02x:%02x:%02x\n",
                Info->Addr2[0], Info->Addr2[1], Info->Addr2[2], Info->Addr2[3], Info->Addr2[4], Info->Addr2[5]));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Addr3 = %02x:%02x:%02x:%02x:%02x:%02x\n",
                Info->Addr3[0], Info->Addr3[1], Info->Addr3[2], Info->Addr3[3], Info->Addr3[4], Info->Addr3[5]));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel = %u\n", Info->Channel));
            
#ifdef DOT11_VHT_AC  			
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Channel_2nd = %u\n", Info->Channel_2nd));
#endif /* DOT11_VHT_AC */             
			
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ch_Band = %d\n", Info->Ch_Band));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Control Channel = %d\n", Info->ControlChl));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxLength = %u\n", Info->TxLength));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxCount = %u\n", Info->TxCount));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("RFFreqOffset = %u\n", Info->RFFreqOffset));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("IPG = %dus\n", Info->ipg_param.ipg));
        	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Duty Cycle = %d%%\n", Info->duty_cycle));
        	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Pkt Tx Time = %dus\n", Info->pkt_tx_time));
        }
    }
#endif /* DBDC_MODE */

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                Tx Power INFO\n"));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxPower0 = %d\n", ATECtrl->TxPower0));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxPower1 = %d\n", ATECtrl->TxPower1));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxPower2 = %d\n", ATECtrl->TxPower2));
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxPower3 = %d\n", ATECtrl->TxPower3));

    if (ExtendInfo)
    {
        if(band_idx == 0)
        {            
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                   TXBF INFO \n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));

#ifdef TXBF_SUPPORT
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETXBF = %d\n", ATECtrl->eTxBf));   
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ITXBF = %d\n", ATECtrl->iTxBf));
#endif /* TXBF_SUPPORT */

            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 Tx FRAME INFO \n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HW Length = %d\n", ATECtrl->HLen));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Payload Length = %d\n", ATECtrl->pl_len));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Sequence = %d\n", ATECtrl->seq));

            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                Extension INFO\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SKB Allocate = %d\n", ATECtrl->is_alloc_skb));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxStatus = %d\n", ATECtrl->TxStatus));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wdev_idx = %d\n", ATECtrl->wdev_idx));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("QID = %d\n", ATECtrl->QID));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PriSel = %d\n", ATECtrl->PriSel));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nss = %d\n", ATECtrl->Nss));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PerPktBW = %d\n", ATECtrl->PerPktBW));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PrimaryBWSel = %d\n", ATECtrl->PrimaryBWSel));
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STBC = %d\n", ATECtrl->Stbc));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LDPC = %d\n", ATECtrl->Ldpc));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Preamble = %d\n", ATECtrl->Preamble));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FixedPayload = %d\n", ATECtrl->FixedPayload));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxDoneCount = %d\n", ATECtrl->TxDoneCount));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxedCount = %d\n", ATECtrl->TxedCount));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Thermal Value = %d\n", ATECtrl->thermal_val));      
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PrimaryBWSel = %d\n", ATECtrl->PrimaryBWSel));   
        }
#ifdef DBDC_MODE
        else
        {       
            if (Info != NULL)
            {
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 TXBF INFO \n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
#ifdef TXBF_SUPPORT
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ETXBF = %d\n", Info->eTxBf));   
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ITXBF = %d\n", Info->iTxBf));
#endif /* TXBF_SUPPORT */
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 Tx FRAME INFO \n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("HW Length = %d\n", Info->HLen));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Payload Length = %d\n", Info->pl_len));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Sequence = %d\n", Info->seq));

                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                 Extension INFO\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("=============================================\n"));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("SKB Allocate = %d\n", Info->is_alloc_skb));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxStatus = %d\n", Info->TxStatus));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wdev_idx = %d\n", Info->wdev_idx));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("QID = %d\n", Info->QID));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PriSel = %d\n", Info->PriSel));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Nss = %d\n", Info->Nss));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PerPktBW = %d\n", Info->PerPktBW));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PrimaryBWSel = %d\n", Info->PrimaryBWSel));
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("STBC = %d\n", Info->Stbc));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("LDPC = %d\n", Info->Ldpc));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Preamble = %d\n", Info->Preamble));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("FixedPayload = %d\n", Info->FixedPayload));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxDoneCount = %d\n", Info->TxDoneCount));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TxedCount = %d\n", Info->TxedCount));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Thermal Value = %d\n", Info->thermal_val));      
                MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("PrimaryBWSel = %d\n", Info->PrimaryBWSel));  
            }
        }
#endif /* DBDC_MODE */

#ifdef ATE_TXTHREAD
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Current_Init_Thread = %d \n", ATECtrl->current_init_thread));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Dequeue Count = %d \n", ATECtrl->deq_cnt));
#endif /* ATE_TXTHREAD */

#ifdef TXBF_SUPPORT
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("fgEBfEverEnabled = %d \n", ATECtrl->fgEBfEverEnabled));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TXBF INFO Length = %d \n", ATECtrl->txbf_info_len));
#endif /* TXBF_SUPPORT */
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MU Enable = %d \n", ATECtrl->mu_enable));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("MU Users = %d \n", ATECtrl->mu_usrs));
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("wcid_ref = %d \n", ATECtrl->wcid_ref));    
    }

    return TRUE;
    err0:
        return FALSE;
}


INT32 set_ate_duty_cycle(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *Arg)
{
    INT32 Ret = 0;
    ATE_CTRL *ATECtrl = &pAd->ATECtrl;
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
    UINT32 duty_cycle = 0;
    UINT8 band_idx = 0;

    /* Sanity check for input parameter */
    if (Arg == NULL) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!! \n", __FUNCTION__));
        goto err0;
    }
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Duty cycle=%s%%\n", __FUNCTION__, Arg));

    band_idx = MT_ATEGetBandIdxByIf(pAd);
    if (band_idx < 0)
        goto err0;

	duty_cycle = simple_strtol(Arg, 0, 10);
    if ((duty_cycle < 0) || (duty_cycle > 100))
        goto err1;

    Ret = ATEOp->SetDutyCycle(pAd, duty_cycle, band_idx);

    if (!Ret)
        return TRUE;
err1:
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Unexpected input!!\n", __FUNCTION__));
err0:
    return FALSE;
}


INT32 set_ate_pkt_tx_time(
	RTMP_ADAPTER *pAd,
	RTMP_STRING *Arg)
{
    INT32 Ret = 0;
    ATE_CTRL *ATECtrl = &pAd->ATECtrl;
    ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
    UINT32 pkt_tx_time = 0;
    UINT8 band_idx = 0;

    /* Sanity check for input parameter */
    if (Arg == NULL) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): No parameters!! \n", __FUNCTION__));
        goto err0;
    }
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Pkt Tx time=%sus\n", __FUNCTION__, Arg));

    band_idx = MT_ATEGetBandIdxByIf(pAd);
    if (band_idx < 0)
        goto err0;

	pkt_tx_time = simple_strtol(Arg, 0, 10);
    if (pkt_tx_time < 0)
        goto err1;

    Ret = ATEOp->SetPktTxTime(pAd, pkt_tx_time, band_idx);

    if (!Ret)
        return TRUE;
err1:
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Unexpected input!!\n", __FUNCTION__));
err0:
    return FALSE;
}


#if defined(TXBF_SUPPORT) && defined(MT_MAC)
INT SetATEApplyStaToMacTblEntry(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN pManual_cfg = &pAd->AteManualConnInfo;
	UCHAR WCID = pManual_cfg->wtbl_idx;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[WCID];

	/* Currently, for MU-MIMO, we only care the VHT/HT Cap Info and VHT MCS set */

	os_move_mem(&pEntry->vht_cap_ie.vht_cap, &pManual_cfg->vht_cap_info, sizeof(pEntry->vht_cap_ie.vht_cap));
	os_move_mem(&pEntry->HTCapability.HtCapInfo, &pManual_cfg->ht_cap_info, sizeof(pEntry->HTCapability.HtCapInfo));
	os_move_mem(&pEntry->vht_cap_ie.mcs_set, &pManual_cfg->vht_mcs_set, sizeof(pEntry->vht_cap_ie.mcs_set));

	return TRUE;
}


INT SetATEApplyStaToAsic(RTMP_ADAPTER *pAd)
{
	P_MANUAL_CONN manual_cfg = &pAd->AteManualConnInfo;
	UCHAR WCID = manual_cfg->wtbl_idx;
	UCHAR *pAddr = &manual_cfg->peer_mac[0];
	MT_WCID_TABLE_INFO_T WtblInfo;
	//MAC_TABLE_ENTRY *mac_entry = NULL;
#ifdef CONFIG_WTBL_TLV_MODE
#else
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
#endif /*CONFIG_WTBL_TLV_MODE */

#ifdef CONFIG_WTBL_TLV_MODE
#else
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
	{
		WCID = (WCID < wtbl_ctrl->wtbl_entry_cnt[0] ? WCID : MCAST_WCID_TO_REMOVE);
	}
	else
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return FALSE;
	}
#endif /* CONFIG_WTBL_TLV_MODE */

	os_zero_mem(&WtblInfo,sizeof(MT_WCID_TABLE_INFO_T));
	WtblInfo.Wcid = WCID;
	os_move_mem(&WtblInfo.Addr[0],&pAddr[0],6);
	// TODO: shiang-MT7615, risk here!!!
	//if (WCID < MAX_LEN_OF_MAC_TABLE)
	//	mac_entry = &pAd->MacTab.Content[WCID];

	if (WCID == MCAST_WCID_TO_REMOVE || WCID == MAX_LEN_OF_MAC_TABLE)
	{
		WtblInfo.MacAddrIdx = 0xe;
		WtblInfo.WcidType = MT_WCID_TYPE_BMCAST;
		 WtblInfo.CipherSuit = WTBL_CIPHER_NONE;
	}
	else
	{
		//if (!mac_entry) {
		//	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		//				("%s(): mac_entry is NULL!\n", __FUNCTION__));
		//	return;
		//}

		if (pAd->AteManualConnInfo.peer_op_type == OPMODE_AP)
			WtblInfo.WcidType = MT_WCID_TYPE_AP;
		else
		WtblInfo.WcidType = MT_WCID_TYPE_CLI;
		WtblInfo.MacAddrIdx = manual_cfg->ownmac_idx; //mac_entry->wdev->OmacIdx;
		//WtblInfo.Aid = manual_cfg->wtbl_idx; //mac_entry->Aid;
		WtblInfo.CipherSuit = WTBL_CIPHER_NONE;

		//if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_WMM_CAPABLE))
		WtblInfo.SupportQoS = TRUE;

		if(WMODE_CAP_N(manual_cfg->peer_phy_mode))
		{
			WtblInfo.SupportHT = TRUE;
			//if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE))
			{
				WtblInfo.SupportRDG= TRUE;
			}
			WtblInfo.SmpsMode = 0; //mac_entry->MmpsMode ;
			WtblInfo.MpduDensity = 0; //mac_entry->MpduDensity;
			WtblInfo.MaxRAmpduFactor = 3; //mac_entry->MaxRAmpduFactor;

#ifdef DOT11_VHT_AC
			if (WMODE_CAP_AC(manual_cfg->peer_phy_mode))
			{
				WtblInfo.SupportVHT = TRUE;
			}
#endif /* DOT11_VHT_AC */
		}
	}

    WtblInfo.Aid     = manual_cfg->aid;
	WtblInfo.PfmuId  = manual_cfg->pfmuId;
	WtblInfo.spe_idx = manual_cfg->spe_idx;
/*	
	WtblInfo.rca2    = manual_cfg->rca2;
	WtblInfo.rv      = manual_cfg->rv;
*/
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		("%s():Update WTBL table, WCID=%d, Addr=%02x:%02x:%02x:%02x:%02x:%02x, WtblInfo.MacAddrIdx=%d\n",
		__FUNCTION__, WCID, PRINT_MAC(pAddr), WtblInfo.MacAddrIdx));


	MtAsicUpdateRxWCIDTable(pAd, WtblInfo);

#ifdef MANUAL_MU
        if (WMODE_CAP_N(manual_cfg->peer_phy_mode)) {
		MT_BA_CTRL_T BaCtrl;
              INT tid;

		os_zero_mem(&BaCtrl,sizeof(MT_BA_CTRL_T));
		BaCtrl.BaSessionType = BA_SESSION_ORI;
		BaCtrl.BaWinSize = 64;
		BaCtrl.isAdd = TRUE;
		BaCtrl.Sn = 0;
		BaCtrl.Wcid = WtblInfo.Wcid;
		BaCtrl.band_idx = 0;
		os_move_mem(&BaCtrl.PeerAddr[0],&WtblInfo.Addr[0],MAC_ADDR_LEN);
              for (tid = 0; tid < 4; tid++) {
		    BaCtrl.Tid = 0;
        		MtAsicUpdateBASession(pAd, BaCtrl);
              }
        }

        dump_wtbl_info(pAd, WtblInfo.Wcid);
#endif /* MANUAL_MU */

	return TRUE;
}


static INT ATEMacStr2Hex(RTMP_STRING *arg, UINT8 *mac)
{
	INT i;
	RTMP_STRING *token, sepValue[] = ":";

	if (arg == NULL)
		return FALSE;

	/* Mac address acceptable format 01:02:03:04:05:06 length 17*/
	if(strlen(arg) < 17)
		return FALSE;

	for (i = 0, token = rstrtok(arg, &sepValue[0]); token; token = rstrtok(NULL, &sepValue[0]), i++)
	{
		if (i > 6)
			break;

		if((strlen(token) != 2) || (!isxdigit(*token)) || (!isxdigit(*(token+1))))
			return FALSE;
		AtoH(token, (&mac[i]), 1);
	}

	if(i != 6)
		return FALSE;

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("\n%02x:%02x:%02x:%02x:%02x:%02x\n",
				mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));

	return TRUE;
}


INT ATEManualParsingParam(RTMP_ADAPTER *pAd, RTMP_STRING *type, RTMP_STRING *val)
{
	UINT8 mac[MAC_ADDR_LEN] = {0};
	INT op_type = 0;
	INT wtbl_idx = 1;
	INT own_mac_idx = 0;
	INT phy_mode = 0;
	INT bw = BW_20;
	INT nss = 1;
	INT maxrate_mode = MODE_CCK;
	INT maxrate_mcs = 0;
	INT pfmuId = 0, speIdx = 24;
	INT aid = 0;
	UINT8 rca2 = 0, rv = 0;
	UINT8 fgIsSuBFee = 0;
	UINT8 fgIsMuBFee = 0;
	UINT8 fgIsSGIFor20 = 0;
	UINT8 fgIsSGIFor40 = 0;
	UINT8 fgIsSGIFor80 = 0;
	UINT8 fgIsSGIFor160 = 0;
	UINT8 bFeeNsts = 0;
	UINT8 mcsSupport = 0;

	if ((!type) || (!val))
		return FALSE;

	/* mac:xx:xx:xx:xx:xx:xx */
	if (strcmp("mac", type) == 0)
	{
		if (ATEMacStr2Hex(val, &mac[0]) == FALSE) {
			NdisZeroMemory(&mac[0], MAC_ADDR_LEN);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid MAC address(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid MAC address(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		NdisMoveMemory(&pAd->AteManualConnInfo.peer_mac[0], mac, MAC_ADDR_LEN);
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		        ("%s(): MAC=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]));
	}

	/* type:ap/sta */
	if (strcmp("type", type) == 0)
	{
		if (strcmp(val, "ap") == 0) {
			op_type = OPMODE_AP;
		}
		else if (strcmp(val, "sta") == 0)
		{
			op_type = OPMODE_STA;
		}
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid type(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.peer_op_type = op_type;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): TYPE=%d\n", __FUNCTION__, op_type));
	}

	/* wtbl:1~127 */
	if (strcmp("wtbl", type) == 0) {
		if (strlen(val)) {
			wtbl_idx = simple_strtol(val, 0, 10);
			if (wtbl_idx <= 0 || wtbl_idx > 127) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				wtbl_idx = 1;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}
		pAd->AteManualConnInfo.wtbl_idx = wtbl_idx;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): WTBL_IDX=%d\n", __FUNCTION__, wtbl_idx));
	}

	/* ownmac:0~4, 0x10~0x1f */
	if (strcmp("ownmac", type)  == 0) {
		if (strlen(val)) {
			own_mac_idx = simple_strtol(val, 0, 10);
			if (!((own_mac_idx >= 0 || own_mac_idx <= 4) || (own_mac_idx >= 0x10 || own_mac_idx <= 0x1f)))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid OwnMac idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				own_mac_idx = 1;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid wtbl idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}
		pAd->AteManualConnInfo.ownmac_idx = own_mac_idx;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): OWN_MAC_IDX=%d\n", __FUNCTION__, own_mac_idx));
	}

	/* pfmuId: */
	if (strcmp("pfmuId", type)  == 0) {
		if (strlen(val)) {
			pfmuId = simple_strtol(val, 0, 10);
			if (!(pfmuId >= 0x00 || pfmuId <= 0x3f))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid PFMU idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				pfmuId = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid PFMU idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}
		pAd->AteManualConnInfo.pfmuId = pfmuId;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): PFMU_IDX=%d\n", __FUNCTION__, pfmuId));
	}

	/* aid: */
	if (strcmp("aid", type) == 0) {
		if (strlen(val)) {
			aid = simple_strtol(val, 0, 10);
			if (!(aid >= 0x00 || aid <= 2007))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid aid(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				aid = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid aid(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}
		pAd->AteManualConnInfo.aid = aid;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): AID =%d\n", __FUNCTION__, aid));
	}

	/* spe-idx: */
	if (strcmp("speIdx", type)  == 0) {
		if (strlen(val)) {
			speIdx = simple_strtol(val, 0, 10);
			if (!(speIdx >= 0 || speIdx <= 30))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid SPE idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				speIdx = 24;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid PFMU idx(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}
		pAd->AteManualConnInfo.spe_idx = speIdx;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): SPE_IDX=%d\n", __FUNCTION__, speIdx));
	}

	if (strcmp("mubfee", type) == 0) {
		if (strlen(val)) {
			fgIsMuBFee = simple_strtol(val, 0, 10);
			if (!(fgIsMuBFee == 0 || fgIsMuBFee == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid mubfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsMuBFee = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid mubfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsMuBFee) {
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_mu = fgIsMuBFee;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): mubfee =%d\n", __FUNCTION__, fgIsMuBFee));
	}

	if (strcmp("sgi160", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor160 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor160 == 0 || fgIsSGIFor160 == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid sgi160(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsSGIFor160 = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid sgi160(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsSGIFor160) {
			pAd->AteManualConnInfo.vht_cap_info.sgi_160M = fgIsSGIFor160;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): sgi160 =%d\n", __FUNCTION__, fgIsSGIFor160));
	}

	if (strcmp("sgi80", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor80 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor80 == 0 || fgIsSGIFor80 == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid sgi80(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsSGIFor80 = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid sgi80(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsSGIFor80) {
			pAd->AteManualConnInfo.vht_cap_info.sgi_80M = fgIsSGIFor80;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): sgi80 =%d\n", __FUNCTION__, fgIsSGIFor80));
	}

	if (strcmp("sgi40", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor40 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor40 == 0 || fgIsSGIFor40 == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid sgi40(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsSGIFor40 = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid sgi40(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsSGIFor40) {
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor40 = fgIsSGIFor40;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): sgi40 =%d\n", __FUNCTION__, fgIsSGIFor40));
	}

	if (strcmp("sgi20", type) == 0) {
		if (strlen(val)) {
			fgIsSGIFor20 = simple_strtol(val, 0, 10);
			if (!(fgIsSGIFor20 == 0 || fgIsSGIFor20 == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid sgi20(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsSGIFor20 = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid sgi20(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsSGIFor20) {
			pAd->AteManualConnInfo.ht_cap_info.ShortGIfor20 = fgIsSGIFor20;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): sgi20 =%d\n", __FUNCTION__, fgIsSGIFor20));
	}

	if (strcmp("rxmcsnss1", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid rxmcsnss1(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				mcsSupport = 3;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid rxmcsnss1(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss1 = mcsSupport;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): rxmcsnss1 =%d\n", __FUNCTION__, mcsSupport));
	}

	if (strcmp("rxmcsnss2", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid rxmcsnss2(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				mcsSupport = 3;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid rxmcsnss2(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss2 = mcsSupport;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): rxmcsnss2 =%d\n", __FUNCTION__, mcsSupport));
	}

	if (strcmp("rxmcsnss3", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid rxmcsnss3(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				mcsSupport = 3;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid rxmcsnss3(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss3 = mcsSupport;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): rxMcsNSS3 =%d\n", __FUNCTION__, mcsSupport));
	}

	if (strcmp("rxmcsnss4", type) == 0) {
		if (strlen(val)) {
			mcsSupport = simple_strtol(val, 0, 10);
			if (!(mcsSupport >= 0 || mcsSupport <= 3))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid rxmcsnss4(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				mcsSupport = 3;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid rxmcsnss4(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.vht_mcs_set.rx_mcs_map.mcs_ss4 = mcsSupport;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): rxmcsnss4 =%d\n", __FUNCTION__, mcsSupport));
	}

	if (strcmp("subfee", type) == 0) {
		if (strlen(val)) {
			fgIsSuBFee = simple_strtol(val, 0, 10);
			if (!(fgIsSuBFee == 0 || fgIsSuBFee == 1))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid subfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				fgIsSuBFee = 0;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid subfee(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		if (fgIsSuBFee) {
			pAd->AteManualConnInfo.vht_cap_info.bfee_cap_su = fgIsSuBFee;
		}

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): subfee =%d\n", __FUNCTION__, fgIsSuBFee));
	}

	if (strcmp("bfeensts", type) == 0) {
		if (strlen(val)) {
			bFeeNsts = simple_strtol(val, 0, 10);
			if (!(bFeeNsts >= 0 || bFeeNsts < 4))
			{
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid bfeensts(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
				bFeeNsts = 4;
			}
		} else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid bfeensts(%s), use default\n", __FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.vht_cap_info.bfee_sts_cap = bFeeNsts;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): bfeensts =%d\n", __FUNCTION__, bFeeNsts));
	}

	/* mode:a/bg/n/ac */
	if (strcmp("mode", type) == 0) {
		RTMP_STRING *tok;

		tok = val;
		while(strlen(tok)) {
			if (*tok == 'b')
			{
				phy_mode |= WMODE_B;
				tok++;
			}
			else if (*tok == 'g')
			{
				if ((*(tok+1) == 'n') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_GN;
					tok += 2;
				} else {
					phy_mode |= WMODE_G;
					tok += 1;
				}
			}
			else if (*tok == 'a')
			{
				if ((*(tok+1) == 'n') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_AN;
					tok += 2;
				} else if ((*(tok+1) == 'c') && (strlen(tok) >=2)) {
					phy_mode |= WMODE_AC;
					tok += 2;
				} else {
					phy_mode |= WMODE_A;
					tok += 1;
				}
			} else {
				printk("\t%s(): Invalid phy_mode %c\n", __FUNCTION__, *tok);
				tok++;
			}
		}

		pAd->AteManualConnInfo.peer_phy_mode = phy_mode;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
		    ("%s(): phy_mode=%s, convert to PhyMode= 0x%x\n",
			__FUNCTION__, (val == NULL ? "" : val), phy_mode));
	}

	/* bw:20/40/80/160 */
	if (strcmp("bw", type) == 0) {
		if (strlen(val)) {
			bw = simple_strtol(val, 0, 10);
			switch (bw) {
				case 20:
					bw = BW_20;
					break;
				case 40:
					bw = BW_40;
					break;
				case 80:
					bw = BW_80;
					break;
				case 160:
					bw = BW_160;
					break;
				default:
					bw = BW_20;
					break;
			}
		}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid BW string(%s), use default!\n",
				__FUNCTION__, (val == NULL ? "" : val)));
		}

		pAd->AteManualConnInfo.peer_bw = bw;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): BW=%d\n", __FUNCTION__, bw));
	}

	if (strcmp("nss", type) == 0) {
		if (strlen(val)) {
			UINT8 max_tx_path;

			if(pAd->CommonCfg.dbdc_mode) {
				if (WMODE_CAP_2G(phy_mode)) {
					max_tx_path = pAd->dbdc_2G_tx_stream;
				} else {
					max_tx_path = pAd->dbdc_5G_tx_stream;
				}
			} else {
				max_tx_path = pAd->Antenna.field.TxPath;
			}

			nss = simple_strtol(val, 0, 10);
			if (nss > max_tx_path) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid NSS string(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val)));
				nss = 1;
			}

		}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid NSS setting, use default!\n", __FUNCTION__));
		}

		pAd->AteManualConnInfo.peer_nss = nss;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): NSS=%d\n", __FUNCTION__, nss));
	}

    /* rca2 = 0/1 */
	if (strcmp("rca2", type) == 0) {
		if (strlen(val)) {
			rca2 = simple_strtol(val, 0, 10);
			if (rca2 > 1) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid RCA2 string(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val)));
				rca2 = 0;
			}

		}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid RCA2 setting, use default!\n", __FUNCTION__));
		}

		pAd->AteManualConnInfo.rca2 = rca2;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): RCA2=%d\n", __FUNCTION__, rca2));
	}

    /* rv = 0/1 */
	if (strcmp("rv", type) == 0) {
		if (strlen(val)) {
			rv = simple_strtol(val, 0, 10);
			if (rv > 1) {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid RV string(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val)));
				rv = 0;
			}

		}
		else {
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): Invalid RV setting, use default!\n", __FUNCTION__));
		}

		pAd->AteManualConnInfo.rv = rv;
		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): RV=%d\n", __FUNCTION__, rv));
	}

	/* maxrate:cck/ofdm/htmix/htgf/vht/_0~32 */
	if (strcmp("maxrate", type) == 0) {
		RTMP_STRING *tok;

		if (strlen(val)) {
			tok = rtstrchr(val, '_');
			if (tok && strlen(tok) > 1) {
				*tok = 0;
				tok++;
			} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid maxmcs setting(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val)));
				goto maxrate_final;
			}
		} else {
				MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				    ("\t%s(): Invalid maxrate setting(%s), use default!\n",
					__FUNCTION__, (val == NULL ? "" : val)));
				goto maxrate_final;
		}

		if (strlen(tok)) {
			maxrate_mcs = simple_strtol(tok, 0, 10);
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("\t%s(): input MCS string(%s) =%d\n",
				__FUNCTION__, tok, maxrate_mcs));
		}

		if (strcmp(val, "cck") == 0)
		{
			maxrate_mode = MODE_CCK;
			if (maxrate_mcs > 4)
				maxrate_mcs = 3;
		}
		else if (strcmp(val, "ofdm") == 0)
		{
			maxrate_mode = MODE_OFDM;
			if (maxrate_mcs > 7)
				maxrate_mcs = 7;
		}
		else if (strcmp(val, "htmix") == 0)
		{
			maxrate_mode = MODE_HTMIX;
			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		}
		else if (strcmp(val, "htgf") == 0)
		{
			maxrate_mode = MODE_HTGREENFIELD;
			if (maxrate_mcs > 32)
				maxrate_mcs = 32;
		}
		else if (strcmp(val, "vht") == 0)
		{
			maxrate_mode = MODE_VHT;
			if (maxrate_mcs > 9)
				maxrate_mcs = 9;
		}
		else
		{
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			    ("%s(): Invalid RateMode string(%s), use default!\n",
				__FUNCTION__, val));
			maxrate_mode = MODE_CCK;
			maxrate_mcs = 0;
		}

maxrate_final:
		pAd->AteManualConnInfo.peer_maxrate_mode = maxrate_mode;
		pAd->AteManualConnInfo.peer_maxrate_mcs = maxrate_mcs;

		MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): MAXRATE=>MODE=%d,MCS=%d\n",
				__FUNCTION__, maxrate_mode, maxrate_mcs));
	}

	return TRUE;
}


/*
	Assoc Parameters:
		mac:xx:xx:xx:xx:xx:xx-type:ap/sta-mode:a/b/g/gn/an/ac-bw:20/40/80/160-nss:1/2/3/4-pfmuId:xx-aid:xx-maxrate:

	@jeffrey: For MU-MIMO, we need to configure the HT/VHP cap info to emulate different STAs (# of STA >= 2)  which
		  supports different Tx and Rx dimension for early algorithm verification
*/
INT SetATEAssocProc(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
	char sep_type = '-', sep_val = ':';
	RTMP_STRING *tok, *param_str, *param_type, *param_val;
	INT stat;
	char ucNsts;
	UINT_32 rate[8];
	RA_PHY_CFG_T TxPhyCfg;
	RTMP_STRING rate_str[64];

	NdisZeroMemory(&pAd->AteManualConnInfo, sizeof(MANUAL_CONN));

	tok = arg;
	while (tok)
	{
		if (strlen(tok)) {
			param_str = tok;
			tok = rtstrchr(tok, sep_type);
			if (tok) {
				*tok = 0;
				tok++;
			}
			MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): param_str=%s\n", __FUNCTION__, param_str));
			if (strlen(param_str)) {
				param_type = param_str;
				param_val = rtstrchr(param_str, sep_val);
				if (param_val)
				{
					*param_val = 0;
					param_val++;
				}

				if (strlen(param_type) && param_val && strlen(param_val)) {
					stat = ATEManualParsingParam(pAd, param_type, param_val);
					if (stat == FALSE)
						goto err_dump_usage;
				}
			}
		} else {
			break;
		}
	}

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("%s():User manual configured peer STA info:\n", __FUNCTION__));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tMAC=>0x%02x:%02x:%02x:%02x:%02x:%02x\n", PRINT_MAC(pAd->AteManualConnInfo.peer_mac)));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tBAND=>%d\n", pAd->AteManualConnInfo.peer_band));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tOwnMacIdx=>%d\n", pAd->AteManualConnInfo.ownmac_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tWTBL_Idx=>%d\n", pAd->AteManualConnInfo.wtbl_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tOperationType=>%d\n", pAd->AteManualConnInfo.peer_op_type));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tPhyMode=>%d\n", pAd->AteManualConnInfo.peer_phy_mode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tBandWidth=>%d\n", pAd->AteManualConnInfo.peer_bw));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tNSS=>%d\n", pAd->AteManualConnInfo.peer_nss));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tPfmuId=>%d\n", pAd->AteManualConnInfo.pfmuId));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tAid=>%d\n", pAd->AteManualConnInfo.aid));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tSpe_idx=>%d\n", pAd->AteManualConnInfo.spe_idx));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tMaxRate_Mode=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mode));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tMaxRate_MCS=>%d\n", pAd->AteManualConnInfo.peer_maxrate_mcs));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
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

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
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

    // dump WTBL again
    //dump_wtbl_info(pAd, pAd->AteManualConnInfo.wtbl_idx);

	return TRUE;

err_dump_usage:
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("Parameter Usage:\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\tiwpriv ra0 set assoc=[mac:hh:hh:hh:hh:hh:hh]-[wtbl:dd]-[ownmac:dd]-[type:xx]-[mode:mmm]-[bw:dd]-[nss:ss]-[maxrate:kkk_dd]\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\tmac: peer's mac address in hex format\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> mac:00:0c:43:12:34:56\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\twtbl: the WTBL entry index peer will occupied, in range 1~127\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> wtbl:1\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\townmac: the OwnMAC index we'll used to send frame to this peer, in range 0~4 or 16~31\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> ownmac:0\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\ttype: peer's operation type, is a ap or sta, allow input: \"ap\" or \"sta\"\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> type:ap\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\tmode: peer's phy operation mode, allow input: a/b/g/gn/an/ac \n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> mode:aanac	to indicate peer can support A/AN/AC mode\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\tbw: Peer's bandwidth capability, in range to 20/40/80/160\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> bw:40	indicate peer can support BW_40\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\tnss: Peer's capability for Spatial stream which can tx/rx, in range of 1~4 with restriction of Software/Hardware cap.\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> nss:2	indicate peer can support 2ss for both tx/rx\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\tmaxrate: Peer's data rate capability for tx/rx, separate as two parts and separate by '_' character\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\t\t kkk: phy modulation mode, allow input:'cck', 'ofdm', 'htmix', 'htgf', 'vht'\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\t\t dd:phy mcs rate, for CCK:0~3, OFDM:0~7, HT:0~32, VHT:0~9\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> maxrate:cck_1	indicate we only can transmit CCK and MCS 1(2Mbps) or lower MCS to peer\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> maxrate:ofdm_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n"));
	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	    ("\t\t\tExample=> maxrate:htmix_3	indicate we only can transmit OFDM and MCS 3(24Mbps) to peer\n"));

	return FALSE;
}



INT SetATETxBfDutInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
	BOOLEAN     fgBw160;
	RTMP_STRING cmdStr[24];
	ULONG       stTimeChk0, stTimeChk1;
	UCHAR       BandIdx;
	
	fgBw160          = simple_strtol(Arg, 0, 10);
	ATECtrl->fgBw160 = fgBw160;
	
	BandIdx = MT_ATEGetBandIdxByIf(pAd);
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BandIdx = %d\n", BandIdx));

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

	/* set ATEDA=00:11:11:11:11:11 */
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[1], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[2], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[3], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[4], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr1[5], 0x11);

	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[1], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[2], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[3], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[4], 0x11);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr1[5], 0x11);
	
	snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	                                 0x11, 0x11, 0x11, 0x11, 0x11);
	SetATEDa(pAd, cmdStr);
	
	/* set ATESA=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[1], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[2], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[3], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[4], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr2[5], 0x22);

	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[1], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[2], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[3], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[4], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr2[5], 0x22);
	
	//snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	//                                 0x22, 0x22, 0x22, 0x22, 0x22);
	//SetATESa(pAd, cmdStr);

    if (fgBw160)
    {
        AsicDevInfoUpdate(
    		pAd, 
    		0x0, 
    		ATECtrl->Addr2, 
    		BandIdx, 
    		TRUE, 
    		DEVINFO_ACTIVE_FEATURE);
    }
    else
    {
        if (BandIdx == 0)
        {
        	AsicDevInfoUpdate(
        		pAd, 
        		0x0, 
        		ATECtrl->Addr2, 
        		BandIdx, 
        		TRUE, 
        		DEVINFO_ACTIVE_FEATURE);
        }
        else
        {
            AsicDevInfoUpdate(
        		pAd, 
        		0x11, 
        		ATECtrl->Addr2, 
        		BandIdx, 
        		TRUE, 
        		DEVINFO_ACTIVE_FEATURE);
        }
    }
	
	/* set ATEBSSID=00:22:22:22:22:22 */
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[1], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[2], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[3], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[4], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, BandIdx, Addr3[5], 0x22);

	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[0], 0x00);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[1], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[2], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[3], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[4], 0x22);
	TESTMODE_SET_PARAM(ATECtrl, 0, Addr3[5], 0x22);
	
	//snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	//                                 0x22, 0x22, 0x22, 0x22, 0x22);
	//SetATEBssid(pAd, cmdStr);

    if (fgBw160)
    {
        snprintf(cmdStr, sizeof(cmdStr), "00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
    }
    else
    {
        if (BandIdx == 0)
        {
    	    snprintf(cmdStr, sizeof(cmdStr), "00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
        }
        else
        {
            snprintf(cmdStr, sizeof(cmdStr), "11:01:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
        }
    }
    
	Set_BssInfoUpdate(pAd, cmdStr);

	/* set ATETXMODE=2 */
	SetATETxMode(pAd, "2");
	
	/* set ATETXMCS=0 */
	SetATETxMcs(pAd, "0");
	
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");

	/* Enable i/eBF */
	SetATETXBFProc(pAd, "3");

	if ((fgBw160) || (BandIdx == 1))
	{
	    /* set ATETXANT=3 2T */
	    SetATETxAntenna(pAd, "3");
	
	    /* set ATERXANT=3  2R*/
	    SetATERxAntenna(pAd, "3");
	}
	else
	{
		UCHAR TxStream;

		if (pAd->CommonCfg.dbdc_mode)
		{
			if (BandIdx == DBDC_BAND0)
				TxStream = pAd->dbdc_2G_tx_stream;
			else
				TxStream = pAd->dbdc_5G_tx_stream;
		} else {
			TxStream = pAd->Antenna.field.TxPath;
		}

	    switch (TxStream)
	    {
	    case TX_PATH_2:
	        /* set ATETXANT=3 2T */
	        SetATETxAntenna(pAd, "3");
	
	        /* set ATERXANT=3  2R*/
	        SetATERxAntenna(pAd, "3");
	        break;
	    case TX_PATH_3:
	        /* set ATETXANT=7 3T */
	        SetATETxAntenna(pAd, "7");
	
	        /* set ATERXANT=7  3R*/
	        SetATERxAntenna(pAd, "7");
	        break;
	    case TX_PATH_4: 
	    default:
	        /* set ATETXANT=15 4T */
	        SetATETxAntenna(pAd, "15");
	
	        /* set ATERXANT=15  4R*/
	        SetATERxAntenna(pAd, "15");
	        break;
	    }
	}

	//SetATETxPower0(pAd, "14");

	ATECtrl->fgEBfEverEnabled = FALSE;

	NdisGetSystemUpTime(&stTimeChk1);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
			"%s : Time consumption : %lu sec\n",__FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));

	return TRUE;
}


INT SetATETxBfGdInitProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
	BOOLEAN     fgBw160;
	RTMP_STRING cmdStr[80];
	ULONG       stTimeChk0, stTimeChk1;
	UCHAR       BandIdx;
	
	fgBw160 = simple_strtol(Arg, 0, 10);

	NdisGetSystemUpTime(&stTimeChk0);

	/* Do ATESTART */
	SetATE(pAd, "ATESTART");

	/* set ATEDA=00:22:22:22:22:22 */
	ATECtrl->Addr1[0] = 0x00;
	ATECtrl->Addr1[1] = 0x22;
	ATECtrl->Addr1[2] = 0x22;
	ATECtrl->Addr1[3] = 0x22;
	ATECtrl->Addr1[4] = 0x22;
	ATECtrl->Addr1[5] = 0x22;
	snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	                                 0x22, 0x22, 0x22, 0x22, 0x22);
	SetATEDa(pAd, cmdStr);
	
	/* set ATESA=00:11:11:11:11:11 */
	ATECtrl->Addr2[0] = 0x00;
	ATECtrl->Addr2[1] = 0x11;
	ATECtrl->Addr2[2] = 0x11;
	ATECtrl->Addr2[3] = 0x11;
	ATECtrl->Addr2[4] = 0x11;
	ATECtrl->Addr2[5] = 0x11;
	//snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	//                                 0x11, 0x11, 0x11, 0x11, 0x11);
	//SetATESa(pAd, cmdStr);
	BandIdx = MT_ATEGetBandIdxByIf(pAd);
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("BandIdx = %d\n", BandIdx));

    if (fgBw160)
    {
        AsicDevInfoUpdate(
        		pAd, 
        		0x0, 
        		ATECtrl->Addr2, 
        		BandIdx, 
        		TRUE, 
        		DEVINFO_ACTIVE_FEATURE);
    }
    else
    {
        if (BandIdx == 0)
        {
        	AsicDevInfoUpdate(
        		pAd, 
        		0x0, 
        		ATECtrl->Addr2, 
        		BandIdx, 
        		TRUE, 
        		DEVINFO_ACTIVE_FEATURE);
        }
        else
        {
            AsicDevInfoUpdate(
        		pAd, 
        		0x11, 
        		ATECtrl->Addr2, 
        		BandIdx, 
        		TRUE, 
        		DEVINFO_ACTIVE_FEATURE);
        }
    }
	
	/* set ATEBSSID=00:22:22:22:22:22 */
	ATECtrl->Addr3[0] = 0x00;
	ATECtrl->Addr3[1] = 0x22;
	ATECtrl->Addr3[2] = 0x22;
	ATECtrl->Addr3[3] = 0x22;
	ATECtrl->Addr3[4] = 0x22;
	ATECtrl->Addr3[5] = 0x22;
	//snprintf(cmdStr, sizeof(cmdStr), "00:%.2x:%.2x:%.2x:%.2x:%.2x", 
	//                                 0x22, 0x22, 0x22, 0x22, 0x22);
	//SetATEBssid(pAd, cmdStr);

    if (fgBw160)
    {
    	snprintf(cmdStr, sizeof(cmdStr), "00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
    }
    else
    {
        if (BandIdx == 0)
        {
    	    snprintf(cmdStr, sizeof(cmdStr), "00:00:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
        }
        else
        {
            snprintf(cmdStr, sizeof(cmdStr), "11:01:00:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                     0x22, 0x22, 0x22, 0x22, 0x22);
        }
    }
    
	Set_BssInfoUpdate(pAd, cmdStr);

	/* set ATETXMODE=2 */
	SetATETxMode(pAd, "2");
	
	/* set ATETXMCS=0 */
	SetATETxMcs(pAd, "0");
	
	/* set ATETXBW=0 */
	SetATETxBw(pAd, "0");
	
	/* set ATETXGI=0 */
	SetATETxGi(pAd, "0");

	/* set ATETXANT=1 1T */
	SetATETxAntenna(pAd, "1");
	
	/* set ATERXANT=1  1R*/
	SetATERxAntenna(pAd, "1");

	/* Configure WTBL */
    //iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0
    snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:ap-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0\n", 
                                     ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
    printk("%s\n", cmdStr);
    SetATEAssocProc(pAd, cmdStr);


	NdisGetSystemUpTime(&stTimeChk1);

	MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
			"%s : Time consumption : %lu sec\n",__FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));

	return TRUE;
}


INT32 SetATETxPacketWithBf(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    INT32    i;
    UCHAR    ucWlanId, ucTxCnt, BandIdx, *value, ucBuf[4], cmdStr[32];
    BOOLEAN  fgBf;


	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 8)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

    fgBf     = ucBuf[0];
    ucWlanId = ucBuf[1];
    ucTxCnt  = ucBuf[2];

    // Assign Wlan ID for fixed rate TxD
    ATECtrl->wcid_ref = ucWlanId;

    // Get band index
    BandIdx = MT_ATEGetBandIdxByIf(pAd);

    // Stop Rx before ready to Tx
    SetATE(pAd, "RXSTOP");

    // At TxD, enable/disable BF Tx at DW6 bit28
    if (fgBf)
    {
        /* Invalid iBF profile */
        TxBfProfileTagRead(pAd, 2, TRUE);
        TxBfProfileTag_InValid(&pAd->rPfmuTag1, FALSE);
        TxBfProfileTagWrite(pAd,
	                        &pAd->rPfmuTag1,
	                        &pAd->rPfmuTag2,
	                        2);

        //ATECtrl->eTxBf = TRUE;
        //ATECtrl->iTxBf = TRUE;
        TESTMODE_SET_PARAM(ATECtrl, BandIdx, eTxBf, TRUE);
        TESTMODE_SET_PARAM(ATECtrl, BandIdx, iTxBf, TRUE);
        ATECtrl->fgEBfEverEnabled = TRUE;

        // Stop Tx when the action of Tx packet is done
        SetATE(pAd, "TXSTOP");
    
        // Set the number of Tx packets
        snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
        SetATETxCount(pAd, cmdStr);

        // Start packet Tx
        SetATE(pAd, "TXFRAME");
    }
    else
    {
        if (ATECtrl->fgEBfEverEnabled == FALSE)
        {
            //ATECtrl->eTxBf = FALSE;
            //ATECtrl->iTxBf = FALSE;
            TESTMODE_SET_PARAM(ATECtrl, BandIdx, eTxBf, FALSE);
            TESTMODE_SET_PARAM(ATECtrl, BandIdx, iTxBf, FALSE);

            // Stop Tx when the action of Tx packet is done
            SetATE(pAd, "TXSTOP");
    
            // Set the number of Tx packets
            snprintf(cmdStr, sizeof(cmdStr), "%d", ucTxCnt);
            SetATETxCount(pAd, cmdStr);

            // Start packet Tx
            SetATE(pAd, "TXFRAME");
        }
        else
        {
            /* Invalid iBF profile */
            TxBfProfileTagRead(pAd, 2, TRUE);
            TxBfProfileTag_InValid(&pAd->rPfmuTag1, TRUE);
            TxBfProfileTagWrite(pAd,
	                            &pAd->rPfmuTag1,
	                            &pAd->rPfmuTag2,
	                            2);

            ATECtrl->fgEBfEverEnabled = FALSE;
        }
    }
    
	return TRUE;
}


INT32 SetATETxBfChanProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    //ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
    INT32       i;
    UCHAR       *value;
    RTMP_STRING cmdStr[80];
	CHAR	    value_T[12];
	UCHAR       strLen;
	BOOLEAN     fgFinalData;
    UINT16      u2Buf[11],   u2PfmuId,   u2Subcarr;
    INT16       i2Phi11,     i2Phi21,    i2Phi31;
    INT16       i2H11,       i2AngleH11, i2H21, i2AngleH21, i2H31, i2AngleH31, i2H41, i2AngleH41;
    INT32       Ret = 0;
	UCHAR       TxStream;
	UCHAR       BandIdx;

	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 43)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
	    if((strlen(value) > 3) || (!isxdigit(*value)) || (!isxdigit(*(value+1)))) 
			return FALSE;  /*Invalid*/
	    
		strLen=strlen(value);
		if (strLen & 1)
		{
		    strcpy(value_T, "0");
		    strcat(value_T, value);
		    AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
		    u2Buf[i] = be2cpu16(u2Buf[i]);
		    i++;
		}
	}

    u2PfmuId   = u2Buf[0];
    u2Subcarr  = u2Buf[1];
    fgFinalData= u2Buf[2];
    i2H11      = (INT16)(u2Buf[3] << 3) >> 3;
    i2AngleH11 = (INT16)(u2Buf[4] << 3) >> 3;
    i2H21      = (INT16)(u2Buf[5] << 3) >> 3;
    i2AngleH21 = (INT16)(u2Buf[6] << 3) >> 3;
    i2H31      = (INT16)(u2Buf[7] << 3) >> 3;
    i2AngleH31 = (INT16)(u2Buf[8] << 3) >> 3;
    i2H41      = (INT16)(u2Buf[9] << 3) >> 3;
    i2AngleH41 = (INT16)(u2Buf[10] << 3) >> 3;

    i2Phi11    = 0;
    i2Phi21    = 0;
    i2Phi31    = 0;

	BandIdx = MT_ATEGetBandIdxByIf(pAd);
	if (pAd->CommonCfg.dbdc_mode)
	{
		if (BandIdx == DBDC_BAND0)
			TxStream = pAd->dbdc_2G_tx_stream;
		else
			TxStream = pAd->dbdc_5G_tx_stream;
	} else {
		TxStream = pAd->Antenna.field.TxPath;
	}

    switch (TxStream)
	{
	    case TX_PATH_3:
	        i2Phi11 = i2AngleH31 - i2AngleH11;
            i2Phi21 = i2AngleH31 - i2AngleH21;
	        break;
	    case TX_PATH_4:
	    default:
            i2Phi11 = i2AngleH41 - i2AngleH11;
            i2Phi21 = i2AngleH41 - i2AngleH21;
            i2Phi31 = i2AngleH41 - i2AngleH31;
            break;
    }

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : i2AngleH11 = 0x%x, i2AngleH21 = 0x%x, i2AngleH31 = 0x%x, i2AngleH41 = 0x%x\n", 
                                                           __FUNCTION__, i2AngleH11, i2AngleH21, i2AngleH31, i2AngleH41));

    /* Update the tag to enable eBF profile */
    if (fgFinalData)
    {
        snprintf(cmdStr, sizeof(cmdStr), "%02x:01", u2PfmuId);
        Set_TxBfProfileTagRead(pAd, cmdStr);

        pAd->rPfmuTag1.rField.ucInvalidProf = TRUE;
        Set_TxBfProfileTagWrite(pAd, cmdStr);
    }

    /* Update the profile data per subcarrier */
    switch (TxStream)
	{
	    case TX_PATH_3:
	        snprintf(cmdStr, sizeof(cmdStr), "%02x:%03x:%03x:00:%03x:00:000:00:000:00:000:00:000:00:00:00:00:00", 
                                     u2PfmuId, u2Subcarr, 
                                     (UINT16)((UINT16)i2Phi11 & 0xFFF), 
                                     (UINT16)((UINT16)i2Phi21 & 0xFFF));
	        break;
	    case TX_PATH_4:
	    default:
            snprintf(cmdStr, sizeof(cmdStr), "%02x:%03x:%03x:00:%03x:00:%03x:00:000:00:000:00:000:00:00:00:00:00", 
                                     u2PfmuId, u2Subcarr, 
                                     (UINT16)((UINT16)i2Phi11 & 0xFFF), 
                                     (UINT16)((UINT16)i2Phi21 & 0xFFF), 
                                     (UINT16)((UINT16)i2Phi31 & 0xFFF));
            break;
    }

    printk("%s\n", cmdStr);
    
    Ret = Set_TxBfProfileDataWrite(pAd, cmdStr);

    return Ret;
}


INT32 SetATETxBfProfileRead(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    //ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
    INT32       i;
    UCHAR       ucPfmuId,    *value;
    RTMP_STRING cmdStr[32];
    CHAR	    value_T[12];
	UCHAR       strLen;
    UINT16      u2Buf[2],    u2SubCarrier;


	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 7)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
	    if((strlen(value) != 3) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) || (!isxdigit(*(value+2)))) 
			return FALSE;  /*Invalid*/
	    
		strLen=strlen(value);
		if (strLen & 1)
		{
		    strcpy(value_T, "0");
		    strcat(value_T, value);
		    AtoH(value_T, (PCHAR)(&u2Buf[i]), 2);
		    u2Buf[i] = be2cpu16(u2Buf[i]);
		    i++;
		}
	}

    ucPfmuId     = u2Buf[0];
    u2SubCarrier = u2Buf[1];

    snprintf(cmdStr, 11, "%.2x:01:%.2x:%.2x", ucPfmuId, (u2SubCarrier >> 8), (u2SubCarrier & 0xFF));
    Set_TxBfProfileDataRead(pAd, cmdStr);

    return TRUE;
}


INT32 SetATETXBFProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	UCHAR TxBfEn;
#ifdef MT7615	
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
#endif

	TxBfEn = simple_strtol(Arg, 0, 10);
	

#ifdef MT7615
    switch (TxBfEn)
	{
		case 0:
			/* no BF */
			ATECtrl->iTxBf= FALSE;
			ATECtrl->eTxBf = FALSE;
			break;
		case 1:
			/* ETxBF */
			ATECtrl->iTxBf= FALSE;
			ATECtrl->eTxBf = TRUE;
			break;
		case 2:
			/* ITxBF */
			ATECtrl->iTxBf= TRUE;
			ATECtrl->eTxBf = FALSE;
			break;
		case 3:
			/* Enable TXBF support */
			ATECtrl->iTxBf= TRUE;
			ATECtrl->eTxBf = TRUE;
			break;

		default:
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("Set_ATE_TXBF_Proc: Invalid parameter %d\n", TxBfEn));
			Ret = TRUE;
			break;
	}
#endif

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}

        
INT32 SetATEIBfGdCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    INT32       i;
    UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[4];
    BOOLEAN     fgSX2;
    UCHAR       ucPhaseCal, ucPhaseVerifyLnaGainLevel;
    INT32       Ret = 0;


	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 11)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", __FUNCTION__));

    ucGroup       = ucBuf[0];
    ucGroup_L_M_H = ucBuf[1];
    fgSX2         = ucBuf[2];
    ucPhaseCal    = ucBuf[3];
    ucPhaseVerifyLnaGainLevel = 0;

    Ret = CmdITxBfPhaseCal(pAd,
                           ucGroup, 
                           ucGroup_L_M_H,
                           fgSX2,
                           ucPhaseCal,
                           ucPhaseVerifyLnaGainLevel);

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfInstCal(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    INT32       i;
    UCHAR       ucGroup,    ucGroup_L_M_H, *value, ucBuf[5];
    BOOLEAN     fgSX2;
    UCHAR       ucPhaseCal, ucPhaseLnaGainLevel;
    INT32       Ret = 0;


	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 14)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", __FUNCTION__));

    ucGroup             = ucBuf[0];
    ucGroup_L_M_H       = ucBuf[1];
    fgSX2               = ucBuf[2];
    ucPhaseCal          = ucBuf[3];
    ucPhaseLnaGainLevel = ucBuf[4];

    Ret = CmdITxBfPhaseCal(pAd,
                           ucGroup, 
                           ucGroup_L_M_H,
                           fgSX2,
                           ucPhaseCal,
                           ucPhaseLnaGainLevel);

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxBfLnaGain(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    UCHAR       ucLnaGain;
    INT32       Ret = 0;


	if (Arg == NULL)
		return FALSE;

	ucLnaGain = simple_strtol(Arg, 0, 10);

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", __FUNCTION__));

    Ret = CmdTxBfLnaGain(pAd,
                         ucLnaGain);

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfProfileUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    INT32   i;
    UCHAR   Nr, Nc, PfmuIdx, NdpNss, *value, ucBuf[3];
    RTMP_STRING cmdStr[80];
    UCHAR       BandIdx;

	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 8)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

    PfmuIdx = ucBuf[0];

    BandIdx = MT_ATEGetBandIdxByIf(pAd);
    if ((BandIdx == 1) || (ATECtrl->fgBw160))
    {
        Nr  = 1;
    }    
    else
    {
        UCHAR TxStream;

        if (pAd->CommonCfg.dbdc_mode)
        {
            if (BandIdx == DBDC_BAND0)
                TxStream = pAd->dbdc_2G_tx_stream;
            else
                TxStream = pAd->dbdc_5G_tx_stream;
        } else {
            TxStream = pAd->Antenna.field.TxPath;
        }

        Nr  = TxStream - 1;
    }
   
    Nc = ucBuf[2];

    /* Configure iBF tag */
    // PFMU ID
    snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
    Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
    
    // ITxBf
    Set_TxBfProfileTag_BfType(pAd, "0");
    
    // BW20
    Set_TxBfProfileTag_DBW(pAd, "0");
    
    // SU
    Set_TxBfProfileTag_SuMu(pAd, "0");
    
    // PFMU memory allocation
    snprintf(cmdStr, 24, "00:04:00:05:00:06:00:07");
    Set_TxBfProfileTag_Mem(pAd, cmdStr);
    
    // Nr:Nc:Ng:LM:CB:HTCE
    //snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc);
    snprintf(cmdStr, 18, "%.2x:%.2x:00:00:00:00", Nr, Nc);
    Set_TxBfProfileTag_Matrix(pAd, cmdStr);
    
    // SNR
    snprintf(cmdStr, 12, "00:00:00:00");
    Set_TxBfProfileTag_SNR(pAd, cmdStr);
    
    // SMART Antenna
    Set_TxBfProfileTag_SmartAnt(pAd, "0");
    
    // SE index
    Set_TxBfProfileTag_SeIdx(pAd, "0");
    
    // Rmsd
    Set_TxBfProfileTag_RmsdThrd(pAd, "0");
    
    // MCS threshold
    snprintf(cmdStr, 18, "00:00:00:00:00:00");
    Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
    
    // Time out disable
    Set_TxBfProfileTag_TimeOut(pAd, "255");
    
    // Desired BW20
    Set_TxBfProfileTag_DesiredBW(pAd, "0");
    
    // Nr
    snprintf(cmdStr, sizeof(cmdStr), "%d", Nr);
    Set_TxBfProfileTag_DesiredNr(pAd, cmdStr);
    
    // Nc
    snprintf(cmdStr, sizeof(cmdStr), "%d", Nc);
    Set_TxBfProfileTag_DesiredNc(pAd, cmdStr);
    
    // Invalid the tag
    Set_TxBfProfileTag_InValid(pAd, "1");

    // Update PFMU tag
    snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
    Set_TxBfProfileTagWrite(pAd, cmdStr);

    /* Configure the BF StaRec */
    if (ATECtrl->fgBw160)
    {
        snprintf(cmdStr, sizeof(cmdStr), "01:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
    }
    else
    {
        if (BandIdx == 0)
        {
            snprintf(cmdStr, sizeof(cmdStr), "01:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
        }
        else
        {
            snprintf(cmdStr, sizeof(cmdStr), "01:01:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
        }
    }
    
    Set_StaRecCmmUpdate(pAd, cmdStr);

    switch (Nr)
    {
    case 1:
        NdpNss = 8;  // MCS8, 2 streams
        break;
    case 2:
        NdpNss = 16; // MCS16, 3 streams
        break;
    case 3:
        NdpNss = 24; // MCS24, 4 streams
        break;
    default:
        NdpNss = 24;
        break;
    }
    
    snprintf(cmdStr, sizeof(cmdStr), "01:00:%.2x:00:00:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:04:00:05:00:06:00:07:00", PfmuIdx, NdpNss, Nc, Nr);
    //printk("%s\n", cmdStr);
    Set_StaRecBfUpdate(pAd, cmdStr);
    Set_StaRecBfRead(pAd, "1");

    /* Configure WTBL */
    //iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0
    snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n", 
                                     ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5],
                                     (Nc + 1), PfmuIdx);
    //printk("%s\n", cmdStr);
    SetATEAssocProc(pAd, cmdStr);

    /* Update the information requested by ATE Tx Gen */
    ATECtrl->pfmu_info[0].wcid    = 1;
    ATECtrl->pfmu_info[0].bss_idx = 0;
    NdisMoveMemory(ATECtrl->pfmu_info[0].addr, ATECtrl->Addr1, MAC_ADDR_LEN);

    return TRUE;
}


INT32 SetATEEBfProfileConfig(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    INT32   i;
    UCHAR   Nr, Nc, PfmuIdx, NdpNss, *value, ucBuf[3];
    RTMP_STRING cmdStr[80];
    UCHAR   BandIdx;

	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 8)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

    PfmuIdx = ucBuf[0];

    BandIdx = MT_ATEGetBandIdxByIf(pAd);
    if ((BandIdx == 1) || (ATECtrl->fgBw160))
    {
        Nr  = 1;
    }
    else
    {
        UCHAR TxStream;

        if (pAd->CommonCfg.dbdc_mode)
        {
            if (BandIdx == DBDC_BAND0)
                TxStream = pAd->dbdc_2G_tx_stream;
            else
                TxStream = pAd->dbdc_5G_tx_stream;
        } else {
            TxStream = pAd->Antenna.field.TxPath;
        }

        Nr  = TxStream - 1;
    }
    Nc      = ucBuf[2];

    /* Configure iBF tag */
    // PFMU ID
    snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
    Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
    
    // ETxBf
    Set_TxBfProfileTag_BfType(pAd, "1");
    
    // BW20
    Set_TxBfProfileTag_DBW(pAd, "0");
    
    // SU
    Set_TxBfProfileTag_SuMu(pAd, "0");
    
    // PFMU memory allocation
    snprintf(cmdStr, 24, "00:00:00:01:00:02:00:03");
    Set_TxBfProfileTag_Mem(pAd, cmdStr);
    
    // Nr:Nc:Ng:LM:CB:HTCE
    snprintf(cmdStr, 18, "%.2x:%.2x:00:01:00:00", Nr, Nc);
    Set_TxBfProfileTag_Matrix(pAd, cmdStr);
    
    // SNR
    snprintf(cmdStr, 12, "00:00:00:00");
    Set_TxBfProfileTag_SNR(pAd, "00:00:00:00");
    
    // SMART Antenna
    Set_TxBfProfileTag_SmartAnt(pAd, "0");
    
    // SE index
    Set_TxBfProfileTag_SeIdx(pAd, "0");
    
    // Rmsd
    Set_TxBfProfileTag_RmsdThrd(pAd, "0");
    
    // MCS threshold
    snprintf(cmdStr, 18, "00:00:00:00:00:00");
    Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
    
    // Invalid the tag
    Set_TxBfProfileTag_InValid(pAd, "1");

    // Update PFMU tag
    snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuIdx);
    Set_TxBfProfileTagWrite(pAd, cmdStr);

    /* Configure the BF StaRec */
    if (ATECtrl->fgBw160)
    {
        snprintf(cmdStr, sizeof(cmdStr), "01:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
    }
    else
    {
        if (BandIdx == 0)
        {
            snprintf(cmdStr, sizeof(cmdStr), "01:00:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
        }
        else
        {
            snprintf(cmdStr, sizeof(cmdStr), "01:01:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
        }
    }
    Set_StaRecCmmUpdate(pAd, cmdStr);

    switch (Nr)
    {
    case 1:
        NdpNss = 8;  // MCS8, 2 streams
        break;
    case 2:
        NdpNss = 16; // MCS16, 3 streams
        break;
    case 3:
        NdpNss = 24; // MCS24, 4 streams
        break;
    default:
        NdpNss = 24;
        break;
    }
    
    snprintf(cmdStr, sizeof(cmdStr), "01:00:%.2x:00:01:00:%.2x:00:02:%.2x:%.2x:00:00:00:00:00:00:01:00:02:00:03:00", PfmuIdx, NdpNss, Nc, Nr);
    //printk("%s\n", cmdStr);
    Set_StaRecBfUpdate(pAd, cmdStr);
    Set_StaRecBfRead(pAd, "1");

    /* Configure WTBL */
    //iwpriv ra0 set ManualAssoc =mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0
    snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:%d-pfmuId:%d\n", 
                                     ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5],
                                     (Nc + 1), PfmuIdx);

    //printk("%s\n", cmdStr);
    SetATEAssocProc(pAd, cmdStr);

    /* Update the information requested by ATE Tx Gen */
    ATECtrl->pfmu_info[0].wcid    = 1;
    ATECtrl->pfmu_info[0].bss_idx = 0;
    NdisMoveMemory(ATECtrl->pfmu_info[0].addr, ATECtrl->Addr1, MAC_ADDR_LEN);
        
    return TRUE;
}


INT32 SetATEIBfPhaseComp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    //ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    INT32   Ret,         i;
    UCHAR   ucBW,        ucGroup, ucBand, ucDbdcBandIdx, *value, ucBuf[5];
    BOOLEAN fgRdFromE2p, fgDisComp;

	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 14)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

    ucBW          = ucBuf[0];
    ucDbdcBandIdx = ucBuf[1];
    ucGroup       = ucBuf[2];
    fgRdFromE2p   = ucBuf[3];
    fgDisComp     = ucBuf[4];

    ucBand = (ucGroup == 1) ? 1 : 0;

    Ret = CmdITxBfPhaseComp(pAd, ucBW, ucBand, ucDbdcBandIdx, ucGroup, fgRdFromE2p, fgDisComp);

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATEIBfPhaseVerify(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    //ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
    INT32   Ret, i;
    UCHAR   ucGroup, ucGroup_L_M_H, ucPhaseCalType, ucBand, *value, ucBuf[6];
    BOOLEAN fgSX2,   fgRdFromE2p;
    UCHAR   ucPhaseVerifyLnaGainLevel;

	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 17)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s\n", __FUNCTION__));

    ucGroup        = ucBuf[0];
    ucGroup_L_M_H  = ucBuf[1];
    fgSX2          = ucBuf[2];
    ucPhaseCalType = ucBuf[3];
    ucPhaseVerifyLnaGainLevel = ucBuf[4];
    fgRdFromE2p    = ucBuf[5];

    ucBand = (ucGroup == 1) ? 1 : 0;
    
    Ret = CmdITxBfPhaseComp(pAd, 
                            BW_20, 
                            ucBand,
                            fgSX2, 
                            ucGroup, 
                            fgRdFromE2p,
                            FALSE);

    if (Ret)
		return FALSE;

    Ret = CmdITxBfPhaseCal(pAd,
                           ucGroup, 
                           ucGroup_L_M_H,
                           fgSX2,
                           ucPhaseCalType,
                           ucPhaseVerifyLnaGainLevel);

    if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 SetATETxBfPhaseE2pUpdate(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
    INT32   i;
    UCHAR   ucGroup, ucUpdateAllType, *value, ucBuf[3];
    BOOLEAN fgSX2;


	if (Arg == NULL)
		return FALSE;

	if(strlen(Arg) != 8)
		return FALSE;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) 
	{
		if((strlen(value) != 2) || (!isxdigit(*value)) || (!isxdigit(*(value+1))) ) 
			return FALSE;  /*Invalid*/

		AtoH(value, &ucBuf[i++], 1);
	}

    ucGroup         = ucBuf[0];
    fgSX2           = ucBuf[1];
    ucUpdateAllType = ucBuf[2];

    iBFPhaseCalE2PUpdate(pAd, ucGroup, fgSX2, ucUpdateAllType);

    return TRUE;
}


INT32 SetATETxSoundingProc(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32         Ret = 0;
	UCHAR         SoundingMode;
	ATE_CTRL      *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: SoundingMode = %s\n", __FUNCTION__, Arg));

	SoundingMode = simple_strtol(Arg, 0, 10);

	Ret = ATEOp->SetATETxSoundingProc(pAd, SoundingMode);

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}

INT32 SetATEConTxETxBfInitProc(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
    UINT8       loop_index;
    INT         status = TRUE;
    CHAR        *value = 0;
    UINT8       TxMode = 0;
    UINT8       MCS = 0;
    UINT8       BW = 0;
    UINT8       VhtNss = 0;
    UINT8       TRxStream = 0;
    UINT8       Power = 0;
    UINT8       Channel = 0;
    UINT8       Channel2 = 0;
    UINT8       Channl_band = 0;
    UINT16      TxPktLength = 0;
    UINT8       Nr = 0;
    UINT8       LM = 0;
    CHAR        BandIdx = 0;
    UCHAR       OwnMacIdx = 0;
    CHAR        wdev_idx = 0;
    UCHAR       WlanIdx = 1;
    UCHAR       BssIdx = 0;
    UCHAR       PfmuId = WlanIdx - 1;
    ULONG       stTimeChk0, stTimeChk1;
    RTMP_STRING cmdStr[80];
    ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
    UCHAR       *template;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */
    struct wifi_dev    *pWdev = NULL;

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
	if (!Arg) {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
    }

	if(strlen(Arg) != 33) {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }
    
    /* Parsing input parameter */ 
	for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++) {
		switch (loop_index) {
			case 0:
				TxMode = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 1:
				MCS = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 2:
				BW = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 3:
				VhtNss = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 4:
                TRxStream = simple_strtol(value, 0, 10); // 2-bit format
                break;
            case 5:    
				Power = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 6:
				Channel = simple_strtol(value, 0, 10); // 3-bit format
				break;
            case 7:
				Channel2 = simple_strtol(value, 0, 10); // 3-bit format
				break;  
            case 8:
				Channl_band = simple_strtol(value, 0, 10); // 1-bit format
				break;    
			case 9:
				TxPktLength = simple_strtol(value, 0, 10); // 5-bit format
				break;
			default:
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set too much parameters\n", __FUNCTION__));
				break;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TxMode(%d), MCS(%d), BW(%d), VhtNss(%d), TRxStream(%d)\n",
															__FUNCTION__, TxMode, MCS, BW, VhtNss, TRxStream));
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TxPower(%d), Channel(%d), Channel2(%d), Channl_band(%d), TxPktLength(%d)\n",
															__FUNCTION__, Power, Channel, Channel2, Channl_band, TxPktLength));

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Load Preliminary Configuration */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* obtain Band index */
#ifdef CONFIG_AP_SUPPORT
    BandIdx = MT_ATEGetBandIdxByIf(pAd);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx));       
#endif /* CONFIG_AP_SUPPORT */

    /* obtain wireless device index */
    ATECtrl->wdev_idx = MT_ATEGetWDevIdxByBand(pAd, BandIdx);
    wdev_idx = TESTMODE_GET_PARAM(ATECtrl, BandIdx, wdev_idx);

	/* sanity check for Wireless Device index */
	if (wdev_idx < 0)
		goto err0;
	
    pWdev = pAd->wdev_list[wdev_idx];
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev_idx = %d \n", __FUNCTION__, wdev_idx));

    /* obtain TemplateFrame */
    if (BAND0 == BandIdx) {
		NdisMoveMemory(ATECtrl->TemplateFrame, TemplateFrame, 32);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx) {
        NdisMoveMemory(Info->TemplateFrame, TemplateFrame, 32);
    }
#endif /*DBDC_MODE*/
    /* sanity check for Band index */
    if (BandIdx < BAND0)
        goto err0;      

    /* sanity check for Device list */
	if(!pWdev)
		goto err0;

	/* obtain Own MAC index */
	OwnMacIdx = pWdev->OmacIdx;

	/* obtain BSSID index */
	BssIdx = pWdev->BssIdx;

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: OwnMacIdx: %d, BssIdx: %d \n", __FUNCTION__, OwnMacIdx, BssIdx));

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* DUT TxBf Initialization */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    NdisGetSystemUpTime(&stTimeChk0);

    /* Start ATE Mode */
    SetATE(pAd, "ATESTART");

	/* Enable ETxBF Capability */
	CmdTxBfHwEnableStatusUpdate(pAd, TRUE, FALSE);

    if (BAND0 == BandIdx)
    {
        /* set ATEDA=00:11:11:11:11:11 */
        ATECtrl->Addr1[0] = Addr1[0];
        ATECtrl->Addr1[1] = Addr1[1];
        ATECtrl->Addr1[2] = Addr1[2];
        ATECtrl->Addr1[3] = Addr1[3];
        ATECtrl->Addr1[4] = Addr1[4];
        ATECtrl->Addr1[5] = Addr1[5];
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                         ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
        SetATEDa(pAd, cmdStr);

        /* set ATESA=00:22:22:22:22:22 */
        ATECtrl->Addr2[0] = Addr2[0];
        ATECtrl->Addr2[1] = Addr2[1];
        ATECtrl->Addr2[2] = Addr2[2];
        ATECtrl->Addr2[3] = Addr2[3];
        ATECtrl->Addr2[4] = Addr2[4];
        ATECtrl->Addr2[5] = Addr2[5];

        /* set ATEBSSID=00:22:22:22:22:22 */
        ATECtrl->Addr3[0] = Addr3[0];
        ATECtrl->Addr3[1] = Addr3[1];
        ATECtrl->Addr3[2] = Addr3[2];
        ATECtrl->Addr3[3] = Addr3[3];
        ATECtrl->Addr3[4] = Addr3[4];
        ATECtrl->Addr3[5] = Addr3[5];

#ifdef CONFIG_AP_SUPPORT
        AsicDevInfoUpdate(
            pAd, 
            OwnMacIdx, 
            ATECtrl->Addr2, 
            BandIdx, 
            TRUE, 
            DEVINFO_ACTIVE_FEATURE);
#endif /* CONFIG_AP_SUPPORT */

        /* BSS Info Update */
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             OwnMacIdx, BssIdx, ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3], ATECtrl->Addr3[4], ATECtrl->Addr3[5]);
        Set_BssInfoUpdate(pAd, cmdStr);


    }
#ifdef DBDC_MODE    
    else if (BAND1 == BandIdx)
    {
        /* set ATEDA=00:11:11:11:11:11 */
        Info->Addr1[0] = Addr1[0];
        Info->Addr1[1] = Addr1[1];
        Info->Addr1[2] = Addr1[2];
        Info->Addr1[3] = Addr1[3];
        Info->Addr1[4] = Addr1[4];
        Info->Addr1[5] = Addr1[5];
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                         Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5]);
        SetATEDa(pAd, cmdStr);

        /* set ATESA=00:22:22:22:22:22 */
        Info->Addr2[0] = Addr2[0];
        Info->Addr2[1] = Addr2[1];
        Info->Addr2[2] = Addr2[2];
        Info->Addr2[3] = Addr2[3];
        Info->Addr2[4] = Addr2[4];
        Info->Addr2[5] = Addr2[5];

        /* set ATEBSSID=00:22:22:22:22:22 */
        Info->Addr3[0] = Addr3[0];
        Info->Addr3[1] = Addr3[1];
        Info->Addr3[2] = Addr3[2];
        Info->Addr3[3] = Addr3[3];
        Info->Addr3[4] = Addr3[4];
        Info->Addr3[5] = Addr3[5];

#ifdef CONFIG_AP_SUPPORT
        AsicDevInfoUpdate(
            pAd, 
            OwnMacIdx, 
            Info->Addr2,
            BandIdx, 
            TRUE, 
            DEVINFO_ACTIVE_FEATURE);
#endif /* CONFIG_AP_SUPPORT */

        /* BSS Info Update */
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             OwnMacIdx, BssIdx, Info->Addr3[0], Info->Addr3[1], Info->Addr3[2], Info->Addr3[3], Info->Addr3[4], Info->Addr3[5]);
        Set_BssInfoUpdate(pAd, cmdStr);
        

    }
#endif /* DBDC_MODE */

    /* Set ATE Tx Frame content */
    template = TESTMODE_GET_PARAM(ATECtrl, BandIdx, TemplateFrame); // structure type of TemplateFrame structure is HEADER_802_11
    NdisMoveMemory(template +  4, Addr1, MAC_ADDR_LEN);
    NdisMoveMemory(template + 10, Addr2, MAC_ADDR_LEN);
    NdisMoveMemory(template + 16, Addr3, MAC_ADDR_LEN);

    /* Set Tx mode */
    snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
    SetATETxMode(pAd, cmdStr);  // 0: CCK  1: OFDM  2: HT Mixe dmode 3: HT Green Mode   4: VHT mode

    /* Set Tx MCS */
    snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
    SetATETxMcs(pAd, cmdStr);

    /* Set Tx BW */
    snprintf(cmdStr, sizeof(cmdStr), "%d", BW);
    SetATETxBw(pAd, cmdStr);  // 0: 20MHz  1: 40MHz  2: 80MHz  3: 160MHz(160C)  4: 5M  5: 10M  6: 160MHz (160NC)

	/* Set Tx VhtNss */
	if (TxMode == 4)
	{
		snprintf(cmdStr, sizeof(cmdStr), "%d", VhtNss);
		SetATEVhtNss(pAd, cmdStr);
	}
    
    /* set ATETXGI=0 */
    SetATETxGi(pAd, "0");

    /* Set Tx Ant (bitwise representration, ex: 0x3 means wifi[0] and wifi[1] ON) */
    if (BAND0 == BandIdx)
    {
        if (TRxStream == 4)
            SetATETxAntenna(pAd, "15");  // 15 (0xF:  wifi[0], wifi[1], wifi[2], wifi[3] on)
        else if (TRxStream == 3)
            SetATETxAntenna(pAd, "7");   //  7 (0x7:  wifi[0], wifi[1], wifi[2] on)
        else if (TRxStream == 2)
            SetATETxAntenna(pAd, "3");   //  3 (0x3:  wifi[0], wifi[1] on)
    }
    else if (BAND1 == BandIdx)
    {
        SetATETxAntenna(pAd, "3");       //  3 (0x3:  wifi[2], wifi[3] on for DBDC mode)
        TRxStream = 2; // force TxStream to be 2T for DBDC mode 
    }

    /* Set Rx Ant (bitwise representration, ex: 0x3 means wifi[0] and wifi[1] ON) */
    if (BAND0 == BandIdx)
    {
        if (TRxStream == 4)
            SetATERxAntenna(pAd, "15");  // 15 (0xF all on)
        else if (TRxStream == 3)
            SetATERxAntenna(pAd, "7");   //  7 (0x7:  wifi[0], wifi[1], wifi[2] on)
        else if (TRxStream == 2)
            SetATERxAntenna(pAd, "3");   //  3 (0x3:  wifi[0], wifi[1] on)
    }
    else if (BAND1 == BandIdx)
        SetATERxAntenna(pAd, "3");       //  3 (0x3:  wifi[2], wifi[3] on for DBDC mode

	/* Set ATE Channel */
    TESTMODE_SET_PARAM(ATECtrl, BandIdx, Channel, Channel);
    TESTMODE_SET_PARAM(ATECtrl, BandIdx, Channel_2nd, Channel2);
	
    /* Set ATE Tx Power = 36 (unit is 0.5 dBm) */
	snprintf(cmdStr, sizeof(cmdStr), "%d", Power);
	SetATETxPower0(pAd, cmdStr);

    NdisGetSystemUpTime(&stTimeChk1);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, (
            "%s : DUT Init Time consumption : %lu sec\n",__FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));

	/* Device info Update */
    if (BAND0 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             OwnMacIdx, ATECtrl->Addr2[0], ATECtrl->Addr2[1], ATECtrl->Addr2[2], ATECtrl->Addr2[3], ATECtrl->Addr2[4], ATECtrl->Addr2[5], BandIdx);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                             OwnMacIdx, Info->Addr2[0], Info->Addr2[1], Info->Addr2[2], Info->Addr2[3], Info->Addr2[4], Info->Addr2[5], BandIdx);
    }
#endif /* DBDC_MODE */
    Set_DevInfoUpdate(pAd, cmdStr);

	/* STOP AUTO Sounding */
	Set_Stop_Sounding_Proc(pAd, "1");

    /* Enable MAC Rx */
    SetATE(pAd, "RXFRAME");
        
	/*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* EBF Profile Cnfiguration */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* set TxBfProfileTag_PFMU ID */
    snprintf(cmdStr, sizeof(cmdStr), "%d", PfmuId);
    Set_TxBfProfileTag_PfmuIdx(pAd, cmdStr);
    
    /* set TxBfProfileTag_Bf Type */
    Set_TxBfProfileTag_BfType(pAd, "1"); // 0: iBF  1: eBF
    
    /* set TxBfProfileTag_DBW */
    snprintf(cmdStr, sizeof(cmdStr), "%d", BW); // 0: 20MHz  1: 40MHz  2: 80MHz  3: 160MHz(160NC)
    Set_TxBfProfileTag_DBW(pAd, cmdStr);

	/* set TxBfProfileTag_SUMU */
    Set_TxBfProfileTag_SuMu(pAd, "0"); // 0: SU  1: MU
    
    /* PFMU memory allocation */
    snprintf(cmdStr, 24, "00:00:00:01:00:02:00:03");
    Set_TxBfProfileTag_Mem(pAd, cmdStr);
    
    /* set TxBfProfileTag_Matrix */
	if (TxMode == 4 && ((BW == 3)||(BW ==6)))
    {   
        if (TRxStream == 4)
            Nr = 1;
        else
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : Invalid Configuration for BW160!! For BW160, TxStream number must be 4!! \n",__FUNCTION__));
    }    
	else	
    {
        if (TRxStream == 4)
            Nr = 3;
        else if (TRxStream == 3)
            Nr = 2;
        else if (TRxStream == 2)
            Nr = 1;
    }

    if (TxMode == 4)
        LM = 2;
    else if (TxMode == 2)
        LM = 1;
    
    snprintf(cmdStr, 18, "%.2x:00:00:%.2x:00:00", Nr, LM); // Nr:Nc:Ng:LM:CB:HTCE
    Set_TxBfProfileTag_Matrix(pAd, cmdStr); 
    
    /* set TxBfProfileTag_SNR */
    snprintf(cmdStr, 12, "00:00:00:00");
    Set_TxBfProfileTag_SNR(pAd, "00:00:00:00");
    
    /* set TxBfProfileTag_Smart Antenna */
    Set_TxBfProfileTag_SmartAnt(pAd, "0");
    
    /* set TxBfProfileTag_SE index */
    Set_TxBfProfileTag_SeIdx(pAd, "0");
    
    /* set TxBfProfileTag_Rmsd */
    Set_TxBfProfileTag_RmsdThrd(pAd, "0");
    
    /* set TxBfProfileTag_MCS Threshold */
    snprintf(cmdStr, 18, "00:00:00:00:00:00");
    Set_TxBfProfileTag_McsThrd(pAd, cmdStr);
    
    /* set TxBfProfileTag_Invalid Tag */
    Set_TxBfProfileTag_InValid(pAd, "1");

    /* Update PFMU Tag */
    snprintf(cmdStr, sizeof(cmdStr), "00");
    Set_TxBfProfileTagWrite(pAd, cmdStr);

    /* Station Record Common Info Update */
    if (BAND0 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                            WlanIdx, BssIdx, ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:00:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                            WlanIdx, BssIdx, Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5]);
    }
#endif /* DBDC_MODE */
    Set_StaRecCmmUpdate(pAd, cmdStr);

    /* Station Record BF Info Update */
    if(TxMode == 4)
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:09:00:09:04:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
    else if(TxMode == 2)
    {
        if (TRxStream == 4)
            snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:18:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
        else if (TRxStream == 3)
            snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:10:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
        else if (TRxStream == 2)
            snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:00:01:00:08:00:02:00:%.2x:%.2x:00:00:00:00:00:01:00:02:00:03:00", WlanIdx, BssIdx, PfmuId, Nr, BW);
    }
    
    Set_StaRecBfUpdate(pAd, cmdStr);

    /* Read Station Record BF Info */
    Set_StaRecBfRead(pAd, "1");

    /* Configure WTBL and Manual Association */
    /* iwpriv ra0 set ManualAssoc=mac:22:22:22:22:22:22-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:2-pfmuId:0 */
    if (BAND0 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:20-nss:2-pfmuId:0\n", 
                                     ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5], WlanIdx, OwnMacIdx);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:sta-wtbl:%.2x-ownmac:%.2x-mode:aanac-bw:20-nss:2-pfmuId:0\n",
                                             Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5], WlanIdx, OwnMacIdx);
    }
#endif /* DBDC_MODE */
    SetATEAssocProc(pAd, cmdStr);

    /* Read Station Record BF Info */
	Set_StaRecBfRead(pAd, "1");

    /* Read PFMU Tag Info */
	Set_TxBfProfileTagRead(pAd, "00:01");

    /* Update the information requested by ATE Tx packet generation */
    ATECtrl->pfmu_info[PfmuId].wcid    = WlanIdx;
    ATECtrl->pfmu_info[PfmuId].bss_idx = BssIdx;

    NdisMoveMemory(ATECtrl->pfmu_info[PfmuId].addr, ATECtrl->Addr1, MAC_ADDR_LEN);   

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* EBF TxBf Apply */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* WTBL Update TxBf Apply */
	Set_TxBfTxApply(pAd, "01:01:00:00:00");

    /* Read Station Bf Record */
	Set_StaRecBfRead(pAd, "1");
    
	/* Trigger one shot Sounding packet */
	Set_Trigger_Sounding_Proc(pAd, "00:01:00:01:00:00:00");

	/*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Continuous packet Tx Initializaton */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set ATE EBF Enable */
    SetATEEBfTx(pAd, "1");  // need to before switch channel for TxStream config since TxStream only can update correct when etxbf is enable for 3T and 2T
    
	/* Set ATE Channel */
	snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band ,Channel2);
	SetATEChannel(pAd, cmdStr);
	RtmpOsMsDelay(1000);
   
	/* Set ATE Tx packet Length (unit is byte)  */
	snprintf(cmdStr, sizeof(cmdStr), "%d", TxPktLength);
	SetATETxLength(pAd, cmdStr);

    /* Set ATE Tx packet number = 0 (Continuous packet Tx)  */
    SetATETxCount(pAd, "0");
    
	/* Set ATE Tx packet Length = 4 (unit is slot time)  */
	SetATEIpg(pAd, "4");
	
	/* Set Queue Priority = 1 (WMM_BK Queue)  */
	SetATEQid(pAd, "1");
	
	/* Set ATE Tx Dequeue size = 4 (allocate 4 packet after receiving 1 free count) (NOT Use Now!!!)*/

    /* ATE Start Continuos Packet Tx */
	//SetATE(pAd, "TXFRAMESKB");

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Periodical Sounding Trigger */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

	/* Trigger Periodical Sounding packet */
	Set_Trigger_Sounding_Proc(pAd, "02:01:FF:01:00:00:00");

	return status;
    err0:
        return FALSE;
}


INT32 SetATEConTxETxBfGdProc(	
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{

    UINT8       loop_index;
    INT         status = TRUE;
    CHAR        *value = 0;
    UINT8       band_idx = MT_ATEGetBandIdxByIf(pAd);
    UINT32      TxMode = 0;
    UINT32      MCS = 0;
    UINT32      BW = 0;
    UINT32      Channel = 0;
    UINT8       Channel2 = 0;
    UINT8       Channl_band = 0;
    UINT32      CRvalue = 0;
    CHAR        BandIdx = 0;
    UCHAR       OwnMacIdx = 0;
    CHAR        wdev_idx = 0;
    UCHAR       BssIdx = 0;
    ULONG       stTimeChk0, stTimeChk1;
    RTMP_STRING cmdStr[80];
    ATE_CTRL    *ATECtrl = &(pAd->ATECtrl);
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */    
    struct wifi_dev    *pWdev = NULL;

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
    if (!Arg) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(Arg) != 18) {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }

    /* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++) {
		switch (loop_index) {
			case 0:
				TxMode = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 1:
				MCS = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 2:
				BW = simple_strtol(value, 0, 10); // 2-bit format
				break;
			case 3:
				Channel = simple_strtol(value, 0, 10); // 3-bit format
				break;
            case 4:
				Channel2 = simple_strtol(value, 0, 10); // 3-bit format
				break;
            case 5:
				Channl_band = simple_strtol(value, 0, 10); // 1-bit format
				break;
			default:
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
		}
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TxMode(%d), MCS(%d), BW(%d), Channel(%d), Channel2(%d), Channl_band(%d) \n",
															__FUNCTION__, TxMode, MCS, BW, Channel, Channel2, Channl_band));

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
	/* Load Preliminary Configuration */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

#ifdef CONFIG_AP_SUPPORT
    BandIdx = MT_ATEGetBandIdxByIf(pAd);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx));       
#endif /* CONFIG_AP_SUPPORT */

    /* obtain wireless device index */
    ATECtrl->wdev_idx = MT_ATEGetWDevIdxByBand(pAd, BandIdx);
    wdev_idx = TESTMODE_GET_PARAM(ATECtrl, BandIdx, wdev_idx);

	/* sanity check for Wireless Device index */
	if (wdev_idx < 0)
		goto err0;
	
    pWdev = pAd->wdev_list[wdev_idx];
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: wdev_idx = %d \n", __FUNCTION__, wdev_idx));

    /* sanity check for Band index */
    if (BandIdx < BAND0)
        goto err0;      

    /* sanity check for Device list */
    if(!pWdev)
        goto err0;

	/* obtain Own MAC index */
    OwnMacIdx = pWdev->OmacIdx;

	/* obtain BSSID index */
	BssIdx = pWdev->BssIdx;
	
    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: OwnMacIdx: %d, BssIdx: %d \n", __FUNCTION__, OwnMacIdx, BssIdx));

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* GOLDEN TxBf Initialization */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    NdisGetSystemUpTime(&stTimeChk0);

	/* Start ATE Mode */
    SetATE(pAd, "ATESTART");

    if (BAND0 == BandIdx)
    {
    	/* set ATEDA=00:22:22:22:22:22 */
    	ATECtrl->Addr1[0] = Addr2[0];
    	ATECtrl->Addr1[1] = Addr2[1];
    	ATECtrl->Addr1[2] = Addr2[2];
    	ATECtrl->Addr1[3] = Addr2[3];
    	ATECtrl->Addr1[4] = Addr2[4];
    	ATECtrl->Addr1[5] = Addr2[5];
    	snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                         ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5]);
    	SetATEDa(pAd, cmdStr);
    	
    	/* set ATESA=00:11:11:11:11:11 */
    	ATECtrl->Addr2[0] = Addr1[0];
    	ATECtrl->Addr2[1] = Addr1[1];
    	ATECtrl->Addr2[2] = Addr1[2];
    	ATECtrl->Addr2[3] = Addr1[3];
    	ATECtrl->Addr2[4] = Addr1[4];
    	ATECtrl->Addr2[5] = Addr1[5];
        
    	/* set ATEBSSID=00:22:22:22:22:22 */
    	ATECtrl->Addr3[0] = Addr3[0];
    	ATECtrl->Addr3[1] = Addr3[1];
    	ATECtrl->Addr3[2] = Addr3[2];
    	ATECtrl->Addr3[3] = Addr3[3];
    	ATECtrl->Addr3[4] = Addr3[4];
    	ATECtrl->Addr3[5] = Addr3[5];

#ifdef CONFIG_AP_SUPPORT
    	AsicDevInfoUpdate(
    		pAd, 
    		OwnMacIdx, 
    		ATECtrl->Addr2, 
    		BandIdx, 
    		TRUE, 
    		DEVINFO_ACTIVE_FEATURE);
#endif /* CONFIG_AP_SUPPORT */

        /* BSS Info Update */
    	snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
    	                                 OwnMacIdx, BssIdx, ATECtrl->Addr3[0], ATECtrl->Addr3[1], ATECtrl->Addr3[2], ATECtrl->Addr3[3], ATECtrl->Addr3[4], ATECtrl->Addr3[5]);
    	Set_BssInfoUpdate(pAd, cmdStr);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        /* set ATEDA=00:22:22:22:22:22 */
        Info->Addr1[0] = Addr2[0];
        Info->Addr1[1] = Addr2[1];
        Info->Addr1[2] = Addr2[2];
        Info->Addr1[3] = Addr2[3];
        Info->Addr1[4] = Addr2[4];
        Info->Addr1[5] = Addr2[5];
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                         Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5]);
        SetATEDa(pAd, cmdStr);
        
        /* set ATESA=00:11:11:11:11:11 */
        Info->Addr2[0] = Addr1[0];
        Info->Addr2[1] = Addr1[1];
        Info->Addr2[2] = Addr1[2];
        Info->Addr2[3] = Addr1[3];
        Info->Addr2[4] = Addr1[4];
        Info->Addr2[5] = Addr1[5];
        
        /* set ATEBSSID=00:22:22:22:22:22 */
        Info->Addr3[0] = Addr3[0];
        Info->Addr3[1] = Addr3[1];
        Info->Addr3[2] = Addr3[2];
        Info->Addr3[3] = Addr3[3];
        Info->Addr3[4] = Addr3[4];
        Info->Addr3[5] = Addr3[5];

#ifdef CONFIG_AP_SUPPORT
        AsicDevInfoUpdate(
            pAd, 
            OwnMacIdx, 
            Info->Addr2, 
            BandIdx, 
            TRUE, 
            DEVINFO_ACTIVE_FEATURE);
#endif /* CONFIG_AP_SUPPORT */

        /* BSS Info Update */
        snprintf(cmdStr, sizeof(cmdStr), "%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x", 
                                         OwnMacIdx, BssIdx, Info->Addr3[0], Info->Addr3[1], Info->Addr3[2], Info->Addr3[3], Info->Addr3[4], Info->Addr3[5]);
        Set_BssInfoUpdate(pAd, cmdStr);
    }
#endif /* DBDC_MODE */

    /* Set Tx mode */
    snprintf(cmdStr, sizeof(cmdStr), "%d", TxMode);
    SetATETxMode(pAd, cmdStr);  // 0: CCK  1: OFDM  2: HT Mixe dmode 3: HT Green Mode   4: VHT mode

    /* Set Tx MCS */
    snprintf(cmdStr, sizeof(cmdStr), "%d", MCS);
    SetATETxMcs(pAd, cmdStr);

    /* Set Tx BW */
    snprintf(cmdStr, sizeof(cmdStr), "%d", BW);
    SetATETxBw(pAd, cmdStr);  // 0: 20MHz  1: 40MHz  2: 80MHz  3: 160MHz(160C)  4: 5M  5: 10M  6: 160MHz (160NC)
    
    /* set ATETXGI=0 */
    SetATETxGi(pAd, "0");

    /* Set Tx Ant (bitwise representration, ex: 0x5 means wifi[0] and wifi[2] ON) */
    if (BAND0 == BandIdx)
    {
        if ((BW == 3)||(BW == 6))
    		SetATETxAntenna(pAd, "5"); // for BW160C, BW160NC (0x5:  wifi[0], wifi[2] on)
    	else
    		SetATETxAntenna(pAd, "1"); // for BW20, BW40, BW80 (0x1:  wifi[0] on)
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        SetATETxAntenna(pAd, "1"); // for DBDC Band1 (0x1:  wifi[2] on)
    }
#endif /* DBDC_MODE */
    
    /* Set Rx Ant (bitwise representration, ex: 0x5 means wifi[0] and wifi[2] ON) */
    if (BAND0 == BandIdx)
    {
        if ((BW == 3)||(BW == 6))
    		SetATERxAntenna(pAd, "5"); // for BW160C, BW160NC (0x5:  wifi[0], wifi[2] on)
    	else
    		SetATERxAntenna(pAd, "1"); // for BW20, BW40, BW80 (0x1:  wifi[0] on)
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        SetATERxAntenna(pAd, "1"); // for DBDC Band1 (0x1:  wifi[2] on)
    }
#endif /* DBDC_MODE */

    /* Configure WTBL */
    //iwpriv ra0 set ManualAssoc = mac:222222222222-type:sta-wtbl:1-ownmac:0-mode:aanac-bw:20-nss:1-pfmuId:0
    if (BAND0 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:ap-wtbl:1-ownmac:%.2x-mode:aanac-bw:20-nss:1-pfmuId:0\n", 
                                         ATECtrl->Addr1[0], ATECtrl->Addr1[1], ATECtrl->Addr1[2], ATECtrl->Addr1[3], ATECtrl->Addr1[4], ATECtrl->Addr1[5], OwnMacIdx);
    }
#ifdef DBDC_MODE
    else if (BAND1 == BandIdx)
    {
        snprintf(cmdStr, sizeof(cmdStr), "mac:%.2x:%.2x:%.2x:%.2x:%.2x:%.2x-type:ap-wtbl:1-ownmac:%.2x-mode:aanac-bw:20-nss:1-pfmuId:0\n", 
                                         Info->Addr1[0], Info->Addr1[1], Info->Addr1[2], Info->Addr1[3], Info->Addr1[4], Info->Addr1[5], OwnMacIdx);
    }
#endif /* DBDC_MODE */

    SetATEAssocProc(pAd, cmdStr);

    NdisGetSystemUpTime(&stTimeChk1);

    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%s : SetATETxBfGdInitProc Time consumption : %lu sec\n",__FUNCTION__, (stTimeChk1 - stTimeChk0)*1000/OS_HZ));

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Turn On BBP CR for Rx */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* iwpriv ra0 mac 82070280=00008001 */
    PHY_IO_WRITE32(pAd, 0x10280, 0x00008001);

    /* check  */
    PHY_IO_READ32(pAd, 0x10280, &CRvalue);
    MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s : <0x82070280> = 0x%x\n",__FUNCTION__, CRvalue));


    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Sounding Mechanism TRx configuration */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* Set Channel Info to ATE Control structure */
    TESTMODE_SET_PARAM(ATECtrl, BandIdx, Channel, Channel);
    TESTMODE_SET_PARAM(ATECtrl, BandIdx, Channel_2nd, Channel2);

    /* Set ATE Channel */  
	snprintf(cmdStr, sizeof(cmdStr), "%d:%d:0:%d", Channel, Channl_band ,Channel2);
    SetATEChannel(pAd, cmdStr);
    RtmpOsMsDelay(1000);

    /* ATE Start Continuos Packet Rx */
    SetATE(pAd, "RXFRAME");

    /* ATE MAC TRx configuration */
    MtATESetMacTxRx(pAd, 6, 1, band_idx); // ENUM_ATE_MAC_RX_RXV: MAC to PHY Rx Enable

    return status;
    err0:
        return FALSE;
}

INT32 SetATESpeIdx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{

	UINT8	loop_index;
	CHAR	*value = 0;
	INT 	status = TRUE;

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
    if (Arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(Arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }

    /* Parsing input parameter */ 
	for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++)
	{
		switch (loop_index)
		{
			case 0:
				pAd->fgEBFCertOn = simple_strtol(value, 0, 10);
				break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BF certification On: %d\n", __FUNCTION__, pAd->fgEBFCertOn));

	return status;
}

INT32 SetATEEBfTx(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	UINT8		loop_index;
	CHAR		*value = 0;
	UINT32  	eTxBf = 0;
	INT 		status = TRUE;
	UCHAR   	addr[6]={0x00, 0x11, 0x11, 0x11, 0x11, 0x11};
	ATE_CTRL 	*ATECtrl = &pAd->ATECtrl;
	UCHAR 		*pate_pkt;
    UCHAR       BandIdx = 0;
    UCHAR       WlanIdx = 1;
    UCHAR       PfmuId = WlanIdx - 1;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
	if (Arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
		return FALSE;
    }

    if(strlen(Arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }

    /* Parsing input parameter */ 
	for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++)
	{
		switch (loop_index)
		{
			case 0:
				eTxBf = simple_strtol(value, 0, 10);
				break;
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}

#ifdef CONFIG_AP_SUPPORT
    BandIdx = MT_ATEGetBandIdxByIf(pAd);
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: BandIdx = %d \n", __FUNCTION__, BandIdx));       
#endif /* CONFIG_AP_SUPPORT */

    /* sanity check of DBDCMode index */
    if (BandIdx < 0)
        goto err0;      

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* EBF Configuration */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    if (BandIdx == 0)
    {
    	ATECtrl->eTxBf = eTxBf;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: ATECtrl->eTxBf = %d !!!!! \n", __FUNCTION__, ATECtrl->eTxBf));
    }
#ifdef DBDC_MODE
    else if (BandIdx == 1)
    {
        Info->eTxBf = eTxBf;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Info->eTxBf = %d !!!!! \n", __FUNCTION__, Info->eTxBf));
    }
#endif /* DBDC_MODE */
    ATECtrl->wcid_ref = WlanIdx; // For Sportan certification, only Golden
	NdisCopyMemory(ATECtrl->pfmu_info[PfmuId].addr, addr, MAC_ADDR_LEN);

    if (BandIdx == 0)
    {
        if (ATECtrl->eTxBf)
        	SetATESpeIdx(pAd, "1");   	
        else
        	SetATESpeIdx(pAd, "0");    		
    }
#ifdef DBDC_MODE
    else if (BandIdx == 1)
    {
        if (Info->eTxBf)
        	SetATESpeIdx(pAd, "1");   	
        else
        	SetATESpeIdx(pAd, "0");    		
    }
#endif /* DBDC_MODE */
	pate_pkt = TESTMODE_GET_PARAM(ATECtrl, BandIdx, pate_pkt);

	/* Generate new packet with new contents */
	MT_ATEGenPkt(pAd, pate_pkt, BandIdx);

	return status;
    err0:
        return FALSE;

}

INT32 SetATEEBFCE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{

	UINT8	loop_index;
	CHAR	*value = 0;
	INT 	status = TRUE;

    /*----------------------------------------------------------------------------------------------------------------------------------------*/
    /* Configure Input Parameter */
    /*----------------------------------------------------------------------------------------------------------------------------------------*/

    /* sanity check for input parameter*/
    if (Arg == NULL)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: No parameters!! \n", __FUNCTION__));
        return FALSE;
    }

    if(strlen(Arg) != 1)
    {   
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Wrong parameter format!! \n", __FUNCTION__));
        return FALSE;
    }

    /* Parsing input parameter */
	for (loop_index = 0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":"), loop_index++)
	{
		switch (loop_index)
		{
			case 0:
				pAd->fgEBFCertification = simple_strtol(value, 0, 10);
				break;	
			default:
			{
				status = FALSE;
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
				break;
			}	
		}
	}
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: eBF Certification: %d\n", __FUNCTION__, pAd->fgEBFCertification));
	
	return status;
}

INT32 SetATEEBFCEInfo(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT 	status = TRUE;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: eBF Certification: %d\n", __FUNCTION__, pAd->fgEBFCertification));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BF certification On: %d !!!!!\n", __FUNCTION__, pAd->fgEBFCertOn));

	return status;
}

INT32 SetATEEBFCEHelp(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT 	status = TRUE;
	
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================================= \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                            ATE ETxBF Certification Procedure Guide                           \n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================================= \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For HT20 mode                                                                                 \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  iwpriv ra0 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:00:036:112:1 (Use in Golden Device)             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:00:01:04:18:036:112:1:04000                   \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra0 set ATE=TXFRAME                                                               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  check Iqxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------------------------------------- \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For HT40 mode                                                                                 \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  iwpriv ra0 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=02:00:01:036:112:1 (Use in Golden Device)             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=02:00:01:01:04:18:036:112:1:04000                   \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra0 set ATE=TXFRAMESKB                                                            \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  check Iqxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------------------------------------- \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For VHT80 mode                                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  iwpriv ra0 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:02:036:112:1 (Use in Golden Device)             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:02:01:04:18:036:112:1:16000                   \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra0 set ATE=TXFRAMESKB                                                            \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  check Iqxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------------------------------------- \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For VHT160C mode                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  iwpriv ra0 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:03:036:112:1 (Use in Golden Device)             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:03:01:04:18:036:112:1:16000                   \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra0 set ATE=TXFRAMESKB                                                            \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  check Iqxel waveform                                                                     \n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------------------------------------- \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For VHT160NC mode                                                                             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  iwpriv ra0 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra0 set ATEConTxETxBfGdProc=04:00:06:036:112:1 (Use in Golden Device)             \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra0 set ATEConTxETxBfInitProc=04:00:06:01:04:18:036:112:1:16000                   \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra0 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra0 set ATE=TXFRAMESKB                                                            \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  iwpriv ra0 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra0 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  check Iqxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("--------------------------------------------------------------------------------------------- \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("For DBDC Band1 HT20 mode                                                                      \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                                                                                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  configure DBDC mode and Reboot system                                                    \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  iwpriv ra1 set ATEEBFCE=1                                                                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  iwpriv ra1 set ATEConTxETxBfGdProc=02:00:00:36:112:1 (Use in Golden Device)              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  iwpriv ra1 set ATEConTxETxBfInitProc=02:00:00:01:02:18:36:112:1:04000                    \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 5)  iwpriv ra1 set ATETXEBF=1 (Tx packet apply BF On)                                        \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 6)  iwpriv ra1 set ATE=TXFRAMESKB                                                            \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 7)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if apply then nonzero)                \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 8)  check IQxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 9)  iwpriv ra1 set ATETXEBF=0 (Tx packet apply BF Off)                                       \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("10)  iwpriv ra1 mac 820fa09c (check [15:0] eBF counter, if not apply then zero)               \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("11)  check Iqxel waveform                                                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================================= \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("                           Method for Dynamical Control Tx Power                              \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================================= \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 1)  Follow ETxBF Certification Procedure to enable TxBf packet at first                      \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 2)  Use command \"iwpriv ra0 set ATE=TXSTOP\" to stop Tx                                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 3)  Use command \"iwpriv ra0 set ATETXPOW0=XX\" to configure Tx Power DAC value for OFDM 54M \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, (" 4)  USe command \"ra0 set ATE=TXFRAMESKB\" to start continuous packet Tx                     \n"));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("============================================================================================= \n"));

	return status;
}


#endif /* TXBF_SUPPORT && MT_MAC */


INT32 SetATEHelp(RTMP_ADAPTER *pAd, RTMP_STRING *Arg)
{
	INT32 Ret = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATE=ATESTART, ATESTOP, TXCONT, TXCARR, TXCARS, TXFRAME, RXFRAME\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEDA\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATESA\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEBSSID\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATECHANNEL, range:0~14\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXPOW0, set power level of antenna 1.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXPOW1, set power level of antenna 2.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXANT, set TX antenna. 0:all, 1:antenna one, 2:antenna two.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATERXANT, set RX antenna.0:all, 1:antenna one, 2:antenna two, 3:antenna three.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXBW, set BandWidth, 0:20MHz, 1:40MHz\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXLEN, set Frame length, range 24~%d\n", (MAX_FRAME_SIZE - 34/* == 2312 */)));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXCNT, set how many frame going to transmit.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXMCS, set MCS, reference to rate table.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXMODE, set Mode 0:CCK, 1:OFDM, 2:HT-Mix, 3:GreenField, 4:VHT, reference to rate table.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATETXGI, set GI interval, 0:Long, 1:Short\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATERXFER, 0:disable Rx Frame error rate. 1:enable Rx Frame error rate.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATERRF, show all RF registers.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATELDE2P, load EEPROM from .bin file.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATERE2P, display all EEPROM content.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEAUTOALC, enable ATE auto Tx alc (Tx auto level control).\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEIPG, set ATE Tx frame IPG.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEPAYLOAD, set ATE payload pattern for TxFrame.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATESHOW, display all parameters of ATE.\n"));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("ATEHELP, online help.\n"));

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


INT32 ATESampleRssi(PRTMP_ADAPTER pAd, RXWI_STRUC *pRxWI)
{
	INT32 Ret = 0;

	if (!Ret)
		return TRUE;
	else
		return FALSE;
}


#ifdef RTMP_PCI_SUPPORT
PNDIS_PACKET ATEPayloadInit(RTMP_ADAPTER *pAd, UINT32 TxIdx)
{
	return NULL;
}
#endif /* RTMP_MAC_PCI */

#ifdef RTMP_PCI_SUPPORT
INT32 ATEPayloadAlloc(PRTMP_ADAPTER pAd, UINT32 Index)
{
	return (NDIS_STATUS_SUCCESS);
}
#endif /* RTMP_MAC_PCI */

INT32 ATEInit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);

	NdisZeroMemory(ATECtrl, sizeof(*ATECtrl));

	ATECtrl->Mode = ATE_STOP;

	ATECtrl->TxCount = 0xFFFFFFFF;
	ATECtrl->payload[0] = 0xAA;
	ATECtrl->FixedPayload = 1;
	ATECtrl->TxLength = 1058;/* 1058 : sync with QA */
	ATECtrl->BW = BW_20;
	ATECtrl->PhyMode = MODE_OFDM;
	ATECtrl->Mcs = 7;
	ATECtrl->Sgi = 0;/* LONG GI : 800 ns*/
	ATECtrl->Channel = 1;
	ATECtrl->TxAntennaSel = 1;
	ATECtrl->RxAntennaSel = 0;

	ATECtrl->QID = QID_AC_BE;
	ATECtrl->Addr1[0] = 0x00;
	ATECtrl->Addr1[1] = 0x11;
	ATECtrl->Addr1[2] = 0x22;
	ATECtrl->Addr1[3] = 0xAA;
	ATECtrl->Addr1[4] = 0xBB;
	ATECtrl->Addr1[5] = 0xCC;

	NdisMoveMemory(ATECtrl->Addr2, ATECtrl->Addr1, MAC_ADDR_LEN);
	NdisMoveMemory(ATECtrl->Addr3, ATECtrl->Addr1, MAC_ADDR_LEN);
	ATECtrl->bQAEnabled = FALSE;
	ATECtrl->bQATxStart = FALSE;
	ATECtrl->bQARxStart = FALSE;
	ATECtrl->TxDoneCount = 0;

	ATECtrl->duty_cycle= 0;
	ATECtrl->pkt_tx_time = 0;

	ATECtrl->ipg_param.ipg = 0;
	ATECtrl->ipg_param.sig_ext = SIG_EXTENSION;
	ATECtrl->ipg_param.slot_time = DEFAULT_SLOT_TIME;
	ATECtrl->ipg_param.sifs_time = DEFAULT_SIFS_TIME;
	ATECtrl->ipg_param.ac_num = QID_AC_BE;
    ATECtrl->ipg_param.aifsn = MIN_AIFSN;
    ATECtrl->ipg_param.cw = MIN_CW;
    ATECtrl->ipg_param.txop = 0;
#ifdef TXBF_SUPPORT
    ATECtrl->eTxBf = FALSE;
    ATECtrl->iTxBf = FALSE;
#endif

	ATECtrl->TemplateFrame = TemplateFrame;

#ifdef MT_MAC
	Ret = MT_ATEInit(pAd);
#endif

#ifdef CONFIG_QA
	ATECtrl->TxStatus = 0;
#endif

	return Ret;
}

INT32 ATEExit(RTMP_ADAPTER *pAd)
{
	INT32 Ret = 0;

#ifdef MT_MAC
    Ret = MT_ATEExit(pAd);
#endif

	return Ret;
}

VOID  ATEPeriodicExec(PVOID SystemSpecific1, PVOID FunctionContext,
						PVOID SystemSpecific2, PVOID SystemSpecific3)
{

}

INT32 SetATE(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;
	UINT32 mode = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Arg = %s\n", __FUNCTION__, Arg));

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;

	mode = TESTMODE_GET_PARAM(ATECtrl, band_idx, Mode);
	if (!strcmp(Arg, "ATESTART") && (mode != ATE_START)) /* support restart w/o ATESTOP */
	{

		if (mode & fATE_TXCONT_ENABLE) {
			/* TODO Get Correct TxfdMode*/
			UINT32 TxfdMode = 1;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("%s ,Stop Continuous Tx\n",__FUNCTION__));	
			Ret += ATEOp->StopContinousTx(pAd, TxfdMode, TESTMODE_BAND0);
		}

		if(mode & fATE_TXCARRSUPP_ENABLE){
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
					("%s ,Stop Carrier Suppression Test\n", __FUNCTION__));
    		Ret += ATEOp->StopTxTone(pAd);
		}
		/* MT76x6 Test Mode Freqency offset restore*/
		if(ATECtrl->en_man_set_freq){
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("MT76x6 Manual Set Frequency Restore\n"));
			MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_ENABLE);
			MtTestModeRestoreCr(pAd, FREQ_OFFSET_MANUAL_VALUE);
			ATECtrl->en_man_set_freq = 0;
		}

		if (Ret)
			goto err1;

#if (defined(MT_MAC) && (!defined(MT7636)))
#ifdef TXBF_SUPPORT
        /* Before going into ATE mode, stop sounding first */
		mt_Trigger_Sounding_Packet(pAd,
                                  FALSE,
	                              0,
	                              0,
	                              0,
	                              NULL);
#endif /* TXBF_SUPPORT */
#endif /* MAC && undefined MT7636 */
        
		Ret = ATEOp->ATEStart(pAd);
	}
	else if (!strcmp(Arg, "ATESTOP") && (mode & ATE_START))
	{
		Ret = ATEOp->ATEStop(pAd);
	}
	else if (!strcmp(Arg, "TRXENABLE") && (mode & ATE_START))
	{
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, TRUE, band_idx);
	}
	else if (!strcmp(Arg, "TRXDISABLE") && (mode & ATE_START))
	{
		MtATESetMacTxRx(pAd, ASIC_MAC_TXRX, FALSE, band_idx);
	}
	else if (!strcmp(Arg, "TXSTREAM") && (mode & ATE_START))
	{
		MtATESetTxStream(pAd, 3, band_idx);
	}
	else if (!strcmp(Arg, "RXSTREAM") && (mode & ATE_START))
	{
		MtATESetRxPath(pAd, 1, band_idx);
	}
	else if (!strcmp(Arg, "APSTOP") && (mode == ATE_STOP))
	{
		Ret = ATEOp->ATEStart(pAd);
	}
	else if (!strcmp(Arg, "APSTART") && (mode & ATE_START))
	{
		Ret = ATEOp->ATEStop(pAd);
	}
	else if (!strcmp(Arg, "TXFRAME") && (mode & ATE_START))
	{
		Ret = ATEOp->StartTx(pAd, band_idx);
	}
#if defined(TXBF_SUPPORT) && defined(MT_MAC)
	else if (!strcmp(Arg, "TXFRAMESKB") && (mode & ATE_START))
	{
		Ret = ATEOp->StartTxSKB(pAd, band_idx);
	}
#endif /* defined(TXBF_SUPPORT) && defined(MT_MAC) */	
	else if (!strcmp(Arg, "RXFRAME") && (mode & ATE_START))
	{
		Ret = ATEOp->StartRx(pAd, band_idx);
	}
	else if (!strcmp(Arg, "TXSTOP") && (mode & ATE_START))
	{
		Ret = ATEOp->StopTx(pAd, band_idx);
	}
	else if (!strcmp(Arg, "RXSTOP") && (mode & ATE_START))
	{
		Ret = ATEOp->StopRx(pAd, band_idx);
	}
	else if (!strcmp(Arg, "TXCONT") && (mode & ATE_START))
	{
    	/* 0: All 1:TX0 2:TX1 */
		/* TODO: Correct band selection */
		UINT32 ant = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
		/* TODO Get Correct TxfdMode*/
		UINT32 TxfdMode = 3; /* continuous payload OFDM/CCK */
		Ret = ATEOp->StartContinousTx(pAd, ant, TxfdMode, band_idx);
		mode |= ATE_TXCONT;
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
	}
	else if (!strcmp(Arg, "TXCONTSTOP") && (mode & ATE_START))
	{
		if (mode & fATE_TXCONT_ENABLE) {
			/* TODO Get Correct TxfdMode*/
			UINT32 TxfdMode = 3;
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s ,Stop Continuous Tx\n",__FUNCTION__));
			ATEOp->StopContinousTx(pAd, TxfdMode, band_idx);
		}
		mode &= ~ATE_TXCONT;
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
	}
	else if (!strcmp(Arg, "TXCARRSUPP") && (mode & ATE_START))
	{
    	INT32 pwr1 = 0xf;
		INT32 pwr2 = 0;
		UINT32 ant = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
		/* 0: All 1:TX0 2:TX1 */
		switch(ant){
		case 0:
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: not support two 2 TXCARR\n",__FUNCTION__));
			break;
		case 1:
			if(ATECtrl->TxPower0>30)
				pwr2 = (ATECtrl->TxPower0 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF0_TX_TWO_TONE_5M);
			break;
		case 2:
			if(ATECtrl->TxPower1>30)
				pwr2 = (ATECtrl->TxPower1 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF1_TX_TWO_TONE_5M);
			break;
		}
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
		mode |= ATE_TXCARRSUPP;
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
	}
	else if (!strcmp(Arg, "TXCARR") && (mode & ATE_START))
	{
		INT32 pwr1 = 0xf;
		INT32 pwr2 = 0;
		UINT32 ant = TESTMODE_GET_PARAM(ATECtrl, band_idx, TxAntennaSel);
		switch(ant){
		case 0:
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: not support two 2 TXCARR\n",__FUNCTION__));
			break;
		case 1:
			if(ATECtrl->TxPower0>30)
				pwr2 = (ATECtrl->TxPower0 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower0 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower0 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF0_TX_ONE_TONE_DC);
			break;
		case 2:
			if(ATECtrl->TxPower1>30)
				pwr2 = (ATECtrl->TxPower1 - 30)<<1;
			else{
				pwr1 = (ATECtrl->TxPower1 & 0x1e) >> 1;
				pwr2 = (ATECtrl->TxPower1 & 0x01) << 1;
			}
			ATEOp->StartTxTone(pAd, WF1_TX_ONE_TONE_DC);		
			break;
		}
		ATEOp->SetTxTonePower(pAd, pwr1, pwr2);
		mode |= ATE_TXCARR;
		TESTMODE_SET_PARAM(ATECtrl, band_idx, Mode, mode);
	}
	#ifdef TXBF_SUPPORT
	else if (!strcmp(Arg, "MUENABLE")){
		ATECtrl->mu_enable = TRUE;
		ATECtrl->mu_usrs = 4;
		TESTMODE_SET_PARAM(ATECtrl, 0, eTxBf, 1);
	}
	else if (!strcmp(Arg, "MUDISABLE")){
		ATECtrl->mu_enable = FALSE;
		ATECtrl->mu_usrs = 0;
		TESTMODE_SET_PARAM(ATECtrl, 0, eTxBf, 0);
		AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
	}
	else if (!strcmp(Arg, "BFENABLE")){
		TESTMODE_SET_PARAM(ATECtrl, 0, eTxBf, 1);
	}
	else if (!strcmp(Arg, "BFDISABLE")){
		TESTMODE_SET_PARAM(ATECtrl, 0, eTxBf, 0);
	}
	#endif
#ifdef PRE_CAL_TRX_SET1_SUPPORT
	else if (!strcmp(Arg, "RX2GSELFTEST") && (mode & ATE_START))
	{
		Ret = ATEOp->RxSelfTest(pAd, "0");
		Ret = 0;
	}
	else if (!strcmp(Arg, "RX5GSELFTEST") && (mode & ATE_START))
	{
		Ret = ATEOp->RxSelfTest(pAd, "1");
		Ret = 0;
	}
	else if (!strcmp(Arg, "RXSELFTEST") && (mode & ATE_START))
	{
		Ret = ATEOp->RxSelfTest(pAd, "2");
		Ret = 0;
	}
	else if (!strcmp(Arg, "TX2GDPD") && (mode & ATE_START))
	{
		Ret = ATEOp->TxDPDTest(pAd, "0");
		Ret = 0;
	}
	else if (!strcmp(Arg, "TX5GDPD") && (mode & ATE_START))
	{
		Ret = ATEOp->TxDPDTest(pAd, "1");
		Ret = 0;
	}
	else if (!strcmp(Arg, "TXDPD") && (mode & ATE_START))
	{
		Ret = ATEOp->TxDPDTest(pAd, "2");
		Ret = 0;
	}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
#ifdef PRE_CAL_TRX_SET2_SUPPORT
	else if ((strcmp(Arg, "PRECAL") > 0) && (mode & ATE_START))
	{
		UINT32 ChGrpId = 0;
		Ret = sscanf(Arg + strlen("PRECAL") + 1, "%d", &ChGrpId);
		if (Ret == 1)
		{
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ChGrpId %d\n",__FUNCTION__, ChGrpId));
		}
	
		Ret = ATEOp->PreCalTest(pAd, 0, ChGrpId);
		Ret = 0;
	}
	else if ((strcmp(Arg, "PRECALTX") > 0) && (mode & ATE_START))
	{
		UINT32 ChGrpId = 0;
	    
		Ret = sscanf(Arg + strlen("PRECALTX") + 1, "%d", &ChGrpId);
		if (Ret == 1)
		{
			MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: ChGrpId %d\n",__FUNCTION__, ChGrpId));
		}
		    
		Ret = ATEOp->PreCalTest(pAd, 1, ChGrpId);
		Ret = 0;
	}  
#endif /* PRE_CAL_TRX_SET2_SUPPORT */
#ifdef PA_TRIM_SUPPORT
    else if ((strcmp(Arg, "PATRIM") > 0) && (mode & ATE_START))
	{
		INT32 i;
        UINT32 Data[4] = {0};
        RTMP_STRING *value = NULL;
        
        for (i = 0, value = rstrtok(Arg + 7,"-"); value; value = rstrtok(NULL,"-"), i++)
        {            
            Data[i] = simple_strtol(value, 0, 16);
            MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("\x1b[32m%s: WF%d = 0x%08x \x1b[m\n", __FUNCTION__, i, Data[i]));
        }
        
		Ret = ATEOp->PATrim(pAd, &Data[0]);
		Ret = 0;
	}
#endif /* PA_TRIM_SUPPORT */
	else
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: do nothing(param = (%s), mode = (%d))\n",
										__FUNCTION__, Arg, ATECtrl->Mode));
	}

	if (!Ret)
		return TRUE;
	err1:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s RF-test stop fail, ret:%d\n", __func__, Ret));
	err0:
	return FALSE;
}


INT32 SetATEChannel(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	const INT idx_num = 4;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT8 band_idx = 0;
	UINT32 param[idx_num];
	INT i = 0;
	CHAR *value;

	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s, band:%x: Channel = %s\n"
			, __FUNCTION__, band_idx, Arg));

	for (i=0;i<idx_num;i++)
		param[i] = 0;

	for (i=0, value = rstrtok(Arg,":"); value; value = rstrtok(NULL,":")) {
		if (!value)
			break;
		if (i==idx_num)
			break;
		param[i++] = simple_strtol(value, 0, 10);
	}

	TESTMODE_SET_PARAM(ATECtrl, band_idx, Channel, param[0]);
#ifdef DOT11_VHT_AC
	TESTMODE_SET_PARAM(ATECtrl, band_idx, Channel_2nd, param[3]);
#endif
	Ret = ATEOp->SetChannel(pAd, param[0], band_idx, param[2], 0, param[1]);

	if (!Ret)
		return TRUE;
	err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s, format: ch:ch_band:pri_sel:ch_2\n"
			, __FUNCTION__));
		return FALSE;
}


#ifdef MT7615
INT32 set_ate_channel_ext(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
#define ATE_SET_CH_EXT_PARAM_CNT    8
    ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
    ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
    HQA_EXT_SET_CH param;
    INT32 ret = 0;
    UINT32 len = 0;
    UINT32 pri_ch = 0;
    UINT32 band_idx = 0;
    UINT32 bw = 0;
    UINT32 per_pkt_bw = 0;
    INT i = 0;
    CHAR *value;
    UINT32 data[ATE_SET_CH_EXT_PARAM_CNT] = {0};
    len = strlen(arg);

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Arg = %s\n", __FUNCTION__, arg));

	for (i=0, value=rstrtok(arg,":"); value; value=rstrtok(NULL,":")) {
		if (!value)
			break;
        if (i == ATE_SET_CH_EXT_PARAM_CNT)
			break;
		data[i] = simple_strtol(value, 0, 10);
        i++;
	}

    if (i == ATE_SET_CH_EXT_PARAM_CNT) {
        param.band_idx      = data[0];
        param.central_ch0   = data[1];
        param.central_ch1   = data[2];
        param.sys_bw        = data[3];
        param.perpkt_bw     = data[4];
        param.pri_sel       = data[5];
        param.reason        = data[6];
        param.ch_band       = data[7];
    }
    else {
        return ret;
    }
    
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

    ret = ate_ops->SetChannel(pAd, param.central_ch0, band_idx, param.pri_sel, param.reason, param.ch_band);
    if (ret == 0)
        ret = TRUE;
    
err0:
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("%s:len:%x, band_idx:%x, ch0:%u, ch1:%u, sys_bw:%x, bw_conver:%x, ",
			__FUNCTION__, len, param.band_idx,
			param.central_ch0, param.central_ch1, param.sys_bw, bw));
	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF,
			("perpkt_bw:%x, pri_sel:%x, pri_ch:%u\n",
			param.perpkt_bw, param.pri_sel, pri_ch));
    
    return ret;
}


INT32 set_ate_start_tx_ext(PRTMP_ADAPTER pAd, RTMP_STRING *arg)
{
#define ATE_START_TX_EXT_PARAM_CNT  14
    ATE_CTRL *ate_ctrl = &(pAd->ATECtrl);
    ATE_OPERATION *ate_ops = ate_ctrl->ATEOp;
    HQA_EXT_TXV param;
    ATE_TXPOWER TxPower;
    INT32 ret = 0;
    INT32 len = 0;
    UINT32 band_idx = 0;
    UINT32 Channel = 0, Ch_Band = 0, SysBw = 0, PktBw = 0;
    INT i = 0;
    CHAR *value;
    UINT32 data[ATE_START_TX_EXT_PARAM_CNT] = {0};
    len = strlen(arg);

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Arg = %s\n", __FUNCTION__, arg));

    for (i=0, value=rstrtok(arg,":"); value; value=rstrtok(NULL,":")) {
        if (!value)
            break;
        if (i == ATE_START_TX_EXT_PARAM_CNT)
            break;
        data[i] = simple_strtol(value, 0, 10);
        i++;
    }

    if (i == ATE_START_TX_EXT_PARAM_CNT) {
        param.band_idx  = data[0];
        param.pkt_cnt   = data[1];
        param.phymode   = data[2];
        param.rate      = data[3];
        param.pwr       = data[4];
        param.stbc      = data[5];
        param.ldpc      = data[6];
        param.ibf       = data[7];
        param.ebf       = data[8];
        param.wlan_id   = data[9];
        param.aifs      = data[10];
        param.gi        = data[11];
        param.tx_path   = data[12];
        param.nss       = data[13];
    }
    else { 
        return ret;
    }

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
    TESTMODE_SET_PARAM(ate_ctrl, band_idx, ipg_param.ipg, param.aifs);        //Fix me
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
    if (ret == 0)
        ret = TRUE;

err0:
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
#endif /* MT7615 */


INT32 SetATETxBw(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32 Ret = 0;
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	INT16 BW;
	UINT8 band_idx = 0;

	MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Bw = %s\n", __FUNCTION__, Arg));
	band_idx = MT_ATEGetBandIdxByIf(pAd);
	if (band_idx < 0)
		goto err0;
	BW = simple_strtol(Arg, 0, 10);

    if (BW == BAND_WIDTH_8080) {
        TESTMODE_SET_PARAM(ATECtrl, band_idx, PerPktBW, BAND_WIDTH_160);
    }
    else {
        TESTMODE_SET_PARAM(ATECtrl, band_idx, PerPktBW, BW);
    }

	Ret = ATEOp->SetBW(pAd, BW, band_idx);

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}


VOID rtmp_ate_init(RTMP_ADAPTER *pAd)
{
	
	if (ATEInit(pAd) != NDIS_STATUS_SUCCESS)
	{
		MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): ATE initialization failed !\n", __FUNCTION__));
		ATEExit(pAd);
		return;
	}
}

#ifdef SINGLE_SKU_V2
INT32 SetATESingleSKUEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32            Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */
	UINT8            BandIdx = 0;
    BOOLEAN          fgSKUEn = FALSE;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: SKUEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgSKUEn = simple_strtol(Arg, 0, 10);

#ifdef MT7615
    /* Update SKU Status in ATECTRL Structure */
    if (BAND0 == BandIdx)
        ATECtrl->fgTxPowerSKUEn = fgSKUEn;
#ifdef DBDC_MODE    
    else
        Info->fgTxPowerSKUEn = fgSKUEn;
#endif /* DBDC_MODE */
#endif

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_SINGLE_SKU, fgSKUEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}
#endif /* SINGLE_SKU_V2 */

INT32 SetATEBFBackoffMode(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{	
    g_BFBackOffMode = simple_strtol(Arg, 0, 10);

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: g_BFBackOffMode = %d\n", __FUNCTION__, g_BFBackOffMode));

    return TRUE;
}

INT32 SetATETempCompEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32           Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL        *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION   *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8           BandIdx = 0;
    BOOLEAN         fgTempCompEn = FALSE;

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TempCompEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgTempCompEn = simple_strtol(Arg, 0, 10);

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TEMP_COMP, fgTempCompEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATEPowerPercentEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32            Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8            BandIdx = 0;
    BOOLEAN          fgPowerPercentEn = FALSE;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: PowerPercentEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgPowerPercentEn = simple_strtol(Arg, 0, 10);

#ifdef MT7615
    /* Update Percentage Status in ATECTRL Structure */
    if (BAND0 == BandIdx)
        ATECtrl->fgTxPowerPercentageEn = fgPowerPercentEn;
#ifdef DBDC_MODE 
    else
        Info->fgTxPowerPercentageEn = fgPowerPercentEn;
#endif /* DBDC_MODE */
#endif

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_POWER_PERCENTAGE, fgPowerPercentEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATEPowerPercentCtrl(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32            Ret = 0;
	UINT32           PowerPercentLevel = 100;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8            BandIdx = 0;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

	BandIdx = MT_ATEGetBandIdxByIf(pAd);
	if (BandIdx < 0)
		goto err0;

	PowerPercentLevel = simple_strtol(Arg, 0, 10);

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: PowerPercentLevel = %d \n", __FUNCTION__, PowerPercentLevel));

    /* Sanity check */
    if ((PowerPercentLevel < 0) || ( PowerPercentLevel > 100))
    {
        MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: Please input X which is 0~100 \n", __FUNCTION__));
        goto err0;
    }

#ifdef MT7615
    /* Update TxPower Drop Status in ATECTRL Structure */
    if (BAND0 == BandIdx)
        ATECtrl->PercentageLevel = PowerPercentLevel;
#ifdef DBDC_MODE 
    else
        Info->PercentageLevel = PowerPercentLevel;
#endif /* DBDC_MODE */
#endif /* MT7615 */

#ifdef CONFIG_HW_HAL_OFFLOAD
	Ret = ATEOp->SetPowerDropLevel(pAd, PowerPercentLevel, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
    err0:
		return FALSE;
}

INT32 SetATEBFBackoffEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32            Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8            BandIdx = 0;
    BOOLEAN          fgBFBackoffEn = 0;
#ifdef DBDC_MODE
    BAND_INFO   *Info = &(ATECtrl->band_ext[0]);
#endif /* DBDC_MODE */

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: BFBackoffEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgBFBackoffEn = simple_strtol(Arg, 0, 10);

#ifdef MT7615
    /* Update BF Backoff Status in ATECTRL Structure */
    if (BAND0 == BandIdx)
        ATECtrl->fgTxPowerBFBackoffEn = fgBFBackoffEn;
#ifdef DBDC_MODE
    else
        Info->fgTxPowerBFBackoffEn = fgBFBackoffEn;
#endif /* DBDC_MODE */
#endif

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_BF_BACKOFF, fgBFBackoffEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATETSSIEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32             Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL          *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION     *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8             BandIdx = 0;
    BOOLEAN           fgTSSIEn = 0;

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TSSIEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgTSSIEn = simple_strtol(Arg, 0, 10);

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TSSI, fgTSSIEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

INT32 SetATETxPowerCtrlEn(
	PRTMP_ADAPTER pAd,
	RTMP_STRING *Arg)
{
	INT32            Ret = 0;
#ifdef CONFIG_HW_HAL_OFFLOAD	
	ATE_CTRL         *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION    *ATEOp = ATECtrl->ATEOp;
#endif /* CONFIG_HW_HAL_OFFLOAD */	
	UINT8            BandIdx = 0;
    BOOLEAN          fgTxPowerCtrlEn = 0;

    MTWF_LOG(DBG_CAT_TEST, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s: TxPowerCtrlEn = %s\n", __FUNCTION__, Arg));
	BandIdx = MT_ATEGetBandIdxByIf(pAd);

    if (BandIdx < 0)
		goto err0;

    fgTxPowerCtrlEn = simple_strtol(Arg, 0, 10);

#ifdef CONFIG_HW_HAL_OFFLOAD
    ATEOp->SetCfgOnOff(pAd, EXT_CFG_ONOFF_TXPOWER_CTRL, fgTxPowerCtrlEn, BandIdx);
#endif /* CONFIG_HW_HAL_OFFLOAD */

	if (!Ret)
		return TRUE;
	err0:
		return FALSE;
}

