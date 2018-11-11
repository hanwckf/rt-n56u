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
	hw_ctrl.c
*/
#include "rt_config.h"
#include  "hw_ctrl.h"
#include "hw_ctrl_basic.h"



static NTSTATUS HwCtrlUpdateRtsThreshold(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct rts_thld *rts = (struct rts_thld *)CMDQelmt->buffer;

	AsicUpdateRtsThld(pAd, rts->wdev, rts->pkt_thld, rts->len_thld);
	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlUpdateProtect(RTMP_ADAPTER *pAd)
{
    RTMP_ARCH_OP *arch_ops = &pAd->archOps;
    UINT32 mode = 0, wdev_idx = 0;
	struct wifi_dev *wdev = NULL;
    MT_PROTECT_CTRL_T  protect;
    MT_RTS_THRESHOLD_T rts_thld;
#ifdef DBDC_MODE
	MT_PROTECT_CTRL_T  protect_5g;
#endif /* DBDC_MODE */

    os_zero_mem(&protect, sizeof(MT_PROTECT_CTRL_T));

#ifdef DBDC_MODE
	os_zero_mem(&protect_5g, sizeof(MT_PROTECT_CTRL_T));
#endif /* DBDC_MODE */

    if(arch_ops->archUpdateProtect == NULL ) {
        AsicNotSupportFunc(pAd, __FUNCTION__);
        return NDIS_STATUS_FAILURE;
    }

    do {
		wdev = pAd->wdev_list[wdev_idx];

		if (wdev == NULL)
            break;
		if (!wdev->if_up_down_state) {
			/* skip inactive wdev */
			wdev_idx++;
			continue;
		}
        mode = wdev->protection;

#ifdef DBDC_MODE
		if ((pAd->CommonCfg.dbdc_mode == TRUE) && (HcGetBandByWdev(wdev) == DBDC_BAND1)) {
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
			if ((pAd->bApCliCertTest == TRUE) && (wdev->wdev_type == WDEV_TYPE_APCLI))
				os_zero_mem(&protect_5g, sizeof(MT_PROTECT_CTRL_T));
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

	        if (mode & SET_PROTECT(ERP)) {
	            protect_5g.erp_mask = ERP_OMAC_ALL;
	        }

	        if (mode & SET_PROTECT(NO_PROTECTION)) {
	            /* no need to do any setting */
	        }

	        if (mode & SET_PROTECT(NON_MEMBER_PROTECT)) {
	            protect_5g.mix_mode = 1;
				protect_5g.gf = 1;
				protect_5g.bw40 = 1;
				protect_5g.bw80 = 1;
	        }

	        if (mode & SET_PROTECT(HT20_PROTECT)) {
	            protect_5g.bw40 = 1;
	        }

	        if (mode & SET_PROTECT(NON_HT_MIXMODE_PROTECT)) {
	            protect_5g.mix_mode = 1;
	            protect_5g.gf = 1;
	            protect_5g.bw40 = 1;
				protect_5g.bw80 = 1;
	        }

	        if (mode & SET_PROTECT(GREEN_FIELD_PROTECT)) {
	            protect_5g.gf = 1;
				protect_5g.bw80 = 1;
	        }

	        if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE)) {
	            protect_5g.long_nav = 1;
        	}

			if (mode & SET_PROTECT(LONG_NAV_PROTECT)) {
				protect_5g.long_nav = 1;
			}
			
			if (mode & SET_PROTECT(RIFS_PROTECT)) {
				protect_5g.long_nav = 1;
				protect_5g.rifs = 1;
			}

	        if (mode & SET_PROTECT(FORCE_RTS_PROTECT)) {
	            rts_thld.pkt_num_thld = 0;
	            rts_thld.pkt_len_thld = 1;
				rts_thld.band_idx = DBDC_BAND1;

	            goto end;
	        }

			protect_5g.band_idx = DBDC_BAND1;
		}
		else
#endif /* DBDC_MODE */
		{
#ifdef CONFIG_AP_SUPPORT
#ifdef APCLI_SUPPORT
#ifdef APCLI_CERT_SUPPORT
			if ((pAd->bApCliCertTest == TRUE) && (wdev->wdev_type == WDEV_TYPE_APCLI))
				os_zero_mem(&protect, sizeof(MT_PROTECT_CTRL_T));
#endif /* APCLI_CERT_SUPPORT */
#endif /* APCLI_SUPPORT */
#endif /* CONFIG_AP_SUPPORT */

			if (mode & SET_PROTECT(ERP)) {
				protect.erp_mask = ERP_OMAC_ALL;
			}

			if (mode & SET_PROTECT(NO_PROTECTION)) {
				/* no need to do any setting */
			}

			if (mode & SET_PROTECT(NON_MEMBER_PROTECT)) {
				protect.mix_mode = 1;
				protect.gf = 1;
				protect.bw40 = 1;
			}

			if (mode & SET_PROTECT(HT20_PROTECT)) {
				protect.bw40 = 1;
			}

			if (mode & SET_PROTECT(NON_HT_MIXMODE_PROTECT)) {
				protect.mix_mode = 1;
				protect.gf = 1;
				protect.bw40 = 1;
			}

			if (mode & SET_PROTECT(GREEN_FIELD_PROTECT)) {
				protect.gf = 1;
			}

			//if (mode & SET_PROTECT(RDG)) {
			if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE)) {
				protect.long_nav = 1;
			}

			if (mode & SET_PROTECT(LONG_NAV_PROTECT)) {
				protect.long_nav = 1;
			}
			
			if (mode & SET_PROTECT(RIFS_PROTECT)) {
				protect.long_nav = 1;
				protect.rifs = 1;
			}

			if (mode & SET_PROTECT(FORCE_RTS_PROTECT)) {
				rts_thld.pkt_num_thld = 0;
				rts_thld.pkt_len_thld = 1;

				goto end;
			}
		}

	    if (mode & SET_PROTECT(_NOT_DEFINE_HT_PROTECT)) {
	        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
	                ("[ERROR] NOT Defined HT Protection!\n"));
	    }

        wdev_idx++;
    } while (wdev_idx < WDEV_NUM_MAX);

    arch_ops->archUpdateProtect(pAd, &protect);

#ifdef DBDC_MODE
	if (pAd->CommonCfg.dbdc_mode == TRUE)
		arch_ops->archUpdateProtect(pAd, &protect_5g);
#endif /* DBDC_MODE */

end:
    return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlSetClientMACEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PRT_SET_ASIC_WCID pInfo;

    pInfo = (PRT_SET_ASIC_WCID)CMDQelmt->buffer;
    AsicUpdateRxWCIDTable(pAd, pInfo->WCID, pInfo->Addr, pInfo->IsBMC, pInfo->IsReset);
    return NDIS_STATUS_SUCCESS;
}


#ifdef TXBF_SUPPORT
static NTSTATUS HwCtrlSetClientBfCap(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PMAC_TABLE_ENTRY pMacEntry;

    pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
    AsicUpdateClientBfCap(pAd, pMacEntry);
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetBfRepeater(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PMAC_TABLE_ENTRY pMacEntry;

    pMacEntry = (PMAC_TABLE_ENTRY)CMDQelmt->buffer;
#ifdef CONFIG_AP_SUPPORT    
    AsicTxBfReptClonedStaToNormalSta(pAd, pMacEntry->wcid, pMacEntry->MatchReptCliIdx);
#endif
    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAdjBfSounding(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PMT_STA_BF_ADJ prMtStaBfAdj = NULL;
    UCHAR ucConnState;
    struct wifi_dev *wdev = NULL;
	
    prMtStaBfAdj = (PMT_STA_BF_ADJ)CMDQelmt->buffer;

    if (prMtStaBfAdj)
    {
	ucConnState = prMtStaBfAdj->ConnectionState;
        wdev = prMtStaBfAdj->wdev;
    }

    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlTxBfTxApply(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PUCHAR pTxBfApply;

    pTxBfApply = (PUCHAR)CMDQelmt->buffer;

#ifdef BACKGROUND_SCAN_SUPPORT    
    BfSwitch(pAd, *pTxBfApply);    
#endif 

    return NDIS_STATUS_SUCCESS;
}

#endif /* TXBF_SUPPORT */


static NTSTATUS HwCtrlDelAsicWcid(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_WCID SetAsicWcid;
	SetAsicWcid = *((PRT_SET_ASIC_WCID)(CMDQelmt->buffer));

	if (!VALID_WCID(SetAsicWcid.WCID) &&(SetAsicWcid.WCID != WCID_ALL))
		return NDIS_STATUS_FAILURE;

	AsicDelWcidTab(pAd, SetAsicWcid.WCID);

        return NDIS_STATUS_SUCCESS;
}


#ifdef HTC_DECRYPT_IOT
static NTSTATUS HwCtrlSetAsicWcidAAD_OM(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	RT_SET_ASIC_AAD_OM SetAsicAAD_OM;

	SetAsicAAD_OM= *((PRT_SET_ASIC_AAD_OM)(CMDQelmt->buffer));
	
	AsicSetWcidAAD_OM(pAd, SetAsicAAD_OM.WCID, SetAsicAAD_OM.Value);

	return NDIS_STATUS_SUCCESS;
}
#endif /* HTC_DECRYPT_IOT */



static NTSTATUS HwCtrlSetWcidSecInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
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


static NTSTATUS HwCtrlSetAsicWcidIVEIV(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_ASIC_WCID_IVEIV_ENTRY pInfo;

	pInfo = (PRT_ASIC_WCID_IVEIV_ENTRY)CMDQelmt->buffer;
	AsicUpdateWCIDIVEIV(pAd,
						  pInfo->Wcid,
						  pInfo->Iv,
						  pInfo->Eiv);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAsicWcidAttr(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
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


static void update_txop_level(UINT16 *dst, UINT16 *src,
                             UINT32 bitmap, UINT32 len)
{
	UINT32 prio;

	for (prio=0; prio<len; prio++) {
		if (bitmap & (1<<prio)) {
			if (*(dst+prio) < *(src+prio))
				*(dst+prio) = *(src+prio);
		}
	}
}


static void tx_burst_arbiter(struct _RTMP_ADAPTER *pAd,
                                struct wifi_dev *curr_wdev,
                                             UCHAR bss_idx)
{
    struct wifi_dev **wdev = pAd->wdev_list;
    UINT32 idx = 0;
    UINT32 _prio_bitmap = 0;
    UINT16 txop_level = TXOP_0;
    UINT16 _txop_level[MAX_PRIO_NUM]={0};
    UINT8 prio;
    UINT8 curr_prio = PRIO_DEFAULT;
    EDCA_PARM *edca_param = NULL;
    UCHAR wmm_idx = 0;

    edca_param = HcGetEdca(pAd, curr_wdev);
    if (edca_param == NULL)
	    return;
    wmm_idx = edca_param->WmmSet;

    /* judge the final prio bitmap for specific BSS */
    do {
        if (wdev[idx] == NULL)
            break;

        if (wdev[idx]->bss_info_argument.ucBssIndex == bss_idx) {
            _prio_bitmap |= wdev[idx]->prio_bitmap;
            update_txop_level(_txop_level, wdev[idx]->txop_level,
                    _prio_bitmap, MAX_PRIO_NUM);
        }

        idx++;
    } while (idx < WDEV_NUM_MAX);

    /* update specific BSS's prio bitmap & txop_level array */
    curr_wdev->bss_info_argument.prio_bitmap = _prio_bitmap;
    memcpy(curr_wdev->bss_info_argument.txop_level, _txop_level,
            (sizeof(UINT16) * MAX_PRIO_NUM));

    /* find the highest prio module */
    for (prio=0; prio<MAX_PRIO_NUM; prio++) {
        if (_prio_bitmap & (1<<prio))
            curr_prio = prio;
    }
    txop_level = curr_wdev->bss_info_argument.txop_level[curr_prio];

    AsicSetWmmParam(pAd, wmm_idx, WMM_AC_BE, WMM_PARAM_TXOP, txop_level);
}

static void set_tx_burst(struct _RTMP_ADAPTER *pAd, struct _tx_burst_cfg *txop_cfg)
{
	struct _BSS_INFO_ARGUMENT_T *bss_info = NULL;
	UCHAR bss_idx = 0;

	if (txop_cfg->enable) {
		txop_cfg->wdev->prio_bitmap |= (1 << txop_cfg->prio);
		txop_cfg->wdev->txop_level[txop_cfg->prio] = txop_cfg->txop_level;
	} else {
		txop_cfg->wdev->prio_bitmap &= ~(1 << txop_cfg->prio);
	}

	bss_info = &txop_cfg->wdev->bss_info_argument;
	bss_idx = bss_info->ucBssIndex;

	tx_burst_arbiter(pAd, txop_cfg->wdev, bss_idx);
}

static void hw_set_tx_burst(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wdev,
			UINT8 ac_type, UINT8 prio, UINT16 level, UINT8 enable)
{
	struct _tx_burst_cfg txop_cfg;

	if (wdev == NULL)
		return;
	txop_cfg.wdev = wdev;
	txop_cfg.prio = prio;
	txop_cfg.ac_type = ac_type;
	txop_cfg.txop_level = level;
	txop_cfg.enable = enable;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
			("<caller: %pS>\n -%s: prio=%x, level=%x, enable=%x\n",
			 __builtin_return_address(0), __FUNCTION__,
			 prio, level, enable));

	set_tx_burst(pAd, &txop_cfg);
}


static NTSTATUS HwCtrlSetTxBurst(struct _RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	struct _tx_burst_cfg *txop_cfg = (struct _tx_burst_cfg *)CMDQelmt->buffer;

	if (txop_cfg == NULL)
		return NDIS_STATUS_FAILURE;

	set_tx_burst(pAd, txop_cfg);

	return NDIS_STATUS_SUCCESS;
}


#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlAPAdjustEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_ADJUST_EXP_ACK_TIME  \n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x005400ca);
	}

	return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlAPRecoverEXPAckTime(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("CmdThread::CMDTHREAD_AP_RECOVER_EXP_ACK_TIME  \n"));
		RTMP_IO_WRITE32(pAd, EXP_ACK_TIME, 0x002400ca);
	}

	return NDIS_STATUS_SUCCESS;
}
#endif /* CONFIG_AP_SUPPORT */


static NTSTATUS HwCtrlUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(line:%d)\n", __FUNCTION__, __LINE__));

	NICUpdateRawCounters(pAd);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlAddRemoveKeyTab(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    ASIC_SEC_INFO *pInfo;

    pInfo = (PASIC_SEC_INFO) CMDQelmt->buffer;

    AsicAddRemoveKeyTab(pAd, pInfo);

    return NDIS_STATUS_SUCCESS;
}

#ifdef MAC_REPEATER_SUPPORT
static NTSTATUS HwCtrlAddReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PADD_REPT_ENTRY_STRUC pInfo;
    struct wifi_dev *wdev = NULL;
    UCHAR *pAddr = NULL;
	APCLI_STRUCT *pApCliEntry = NULL;

    pInfo = (PADD_REPT_ENTRY_STRUC)CMDQelmt->buffer;
    wdev = pInfo->wdev;
    pAddr = pInfo->arAddr;
	pApCliEntry = wdev->func_dev;
	
	NdisAcquireSpinLock(&pAd->ApCfg.InsertReptCmdLock);
	pApCliEntry->InsRepCmdCount--;
	NdisReleaseSpinLock(&pAd->ApCfg.InsertReptCmdLock);
	
    RTMPInsertRepeaterEntry(pAd, wdev, pAddr);

    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRemoveReptEntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PREMOVE_REPT_ENTRY_STRUC pInfo;
    UCHAR func_tb_idx;
    UCHAR CliIdx;

    pInfo = (PREMOVE_REPT_ENTRY_STRUC)CMDQelmt->buffer;
    func_tb_idx = pInfo->func_tb_idx;
    CliIdx = pInfo->CliIdx;

    RTMPRemoveRepeaterEntry(pAd, func_tb_idx, CliIdx);

    return NDIS_STATUS_SUCCESS;
}
#endif /*MAC_REPEATER_SUPPORT*/

#ifdef MT_MAC
#ifdef BCN_OFFLOAD_SUPPORT
static NTSTATUS HwCtrlSetBcnOffload(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    PMT_SET_BCN_OFFLOAD pSetBcnOffload = (PMT_SET_BCN_OFFLOAD)CMDQelmt->buffer;
    struct wifi_dev *wdev = pAd->wdev_list[pSetBcnOffload->WdevIdx];
    CMD_BCN_OFFLOAD_T bcn_offload;

    BCN_BUF_STRUC *bcn_buf = NULL;
#ifdef CONFIG_AP_SUPPORT
    TIM_BUF_STRUC *tim_buf = NULL;
#endif
    UCHAR *buf;
    PNDIS_PACKET *pkt = NULL;

    NdisZeroMemory(&bcn_offload, sizeof(CMD_BCN_OFFLOAD_T));

    if (pSetBcnOffload->OffloadPktType == PKT_BCN)
    {
        bcn_buf = &wdev->bcn_buf;
        if (!bcn_buf)
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s(): bcn_buf is NULL!\n", __FUNCTION__));
            return NDIS_STATUS_FAILURE;
        }
        pkt = bcn_buf->BeaconPkt;
    }
#ifdef CONFIG_AP_SUPPORT
    else /* tim pkt case in AP mode. */
    {
        if (pAd->OpMode == OPMODE_AP)
        {
            tim_buf = &wdev->bcn_buf.tim_buf;
        }
        if (!tim_buf)
        {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s(): tim_buf is NULL!\n", __FUNCTION__));
            return NDIS_STATUS_FAILURE;
        }
        pkt = tim_buf->TimPkt;
    }
#endif /* CONFIG_AP_SUPPORT */

    bcn_offload.ucEnable = pSetBcnOffload->Enable;
    bcn_offload.ucWlanIdx = 0;//hardcode at present

    bcn_offload.ucOwnMacIdx = wdev->OmacIdx;
    bcn_offload.ucBandIdx = HcGetBandByWdev(wdev);
    bcn_offload.u2PktLength = pSetBcnOffload->WholeLength;
    bcn_offload.ucPktType = pSetBcnOffload->OffloadPktType;
#ifdef CONFIG_AP_SUPPORT
    bcn_offload.u2TimIePos = pSetBcnOffload->TimIePos;
    bcn_offload.u2CsaIePos = pSetBcnOffload->CsaIePos;
    bcn_offload.ucCsaCount = wdev->csa_count;
#endif
    buf = (UCHAR *)GET_OS_PKT_DATAPTR(pkt);
    NdisCopyMemory(bcn_offload.acPktContent, buf, pSetBcnOffload->WholeLength);
    MtCmdBcnOffloadSet(pAd, bcn_offload);

    return NDIS_STATUS_SUCCESS;
}
#endif /*BCN_OFFLOAD_SUPPORT*/

static NTSTATUS HwCtrlSetTREntry(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	PRT_SET_TR_ENTRY pInfo;
	MAC_TABLE_ENTRY *pEntry;
	pInfo = (PRT_SET_TR_ENTRY)CMDQelmt->buffer;
	pEntry = (MAC_TABLE_ENTRY *)pInfo->pEntry;

	TRTableInsertEntry(pAd, pInfo->WCID, pEntry);

        return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlUpdateBssInfo(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	BSS_INFO_ARGUMENT_T *pBssInfoArgs = (BSS_INFO_ARGUMENT_T*)CMDQelmt->buffer;
	UINT32 ret;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __FUNCTION__));
	ret = AsicBssInfoUpdate(pAd,*pBssInfoArgs);

	return ret;
}


static NTSTATUS HwCtrlSetBaRec(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	MT_BA_CTRL_T *pSetBaRec = (MT_BA_CTRL_T*)CMDQelmt->buffer;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s::CmdThread\n", __FUNCTION__));

	AsicUpdateBASession(pAd,pSetBaRec->Wcid,pSetBaRec->Tid,pSetBaRec->Sn,pSetBaRec->BaWinSize,pSetBaRec->isAdd ,pSetBaRec->BaSessionType);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlHandleUpdateBeacon(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    struct wifi_dev *wdev = NULL;
    MT_UPDATE_BEACON *prMtUpdateBeacon = (MT_UPDATE_BEACON *)CMDQelmt->buffer;
    UCHAR UpdateReason = 0xff;//init value;

    wdev = prMtUpdateBeacon->wdev;
    UpdateReason = prMtUpdateBeacon->UpdateReason;

    if (UpdateReason == IE_CHANGE)
    {
        if (wdev == NULL)
        {
            MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s, Reason is IE_CHANGE but no wdev!!!!\n", __FUNCTION__));
            return NDIS_STATUS_FAILURE;
        }
        MakeBeacon(pAd, wdev, FALSE);
    }
    else if (UpdateReason == PRETBTT_UPDATE)
    {
#ifdef CONFIG_AP_SUPPORT
#ifdef RT_CFG80211_P2P_SUPPORT
        if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#else
        IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
#endif /* RT_CFG80211_P2P_SUPPORT */
        {
#ifdef AP_QLOAD_SUPPORT
            ULONG UpTime;

            /* update channel utilization */
            NdisGetSystemUpTime(&UpTime);
            QBSS_LoadUpdate(pAd, UpTime);
#endif /* AP_QLOAD_SUPPORT */
#ifdef DOT11K_RRM_SUPPORT
            RRM_QuietUpdata(pAd);
#endif /* DOT11K_RRM_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
            RT_CFG80211_BEACON_TIM_UPDATE(pAd);
#else
            UpdateBeaconHandler(
                pAd,
                NULL,
                PRETBTT_UPDATE);
#endif /* RT_CFG80211_P2P_SUPPORT */
        }
#endif /* CONFIG_AP_SUPPORT */
    }

    return NDIS_STATUS_SUCCESS;
}

#ifdef MT_PS
static NTSTATUS HwCtrlClearPSRetrieveToken(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR *pWcid;
	pWcid = CMDQelmt->buffer;
	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("wcid=0x%x, Send Ps Clear CMD to MCU\n", *pWcid));
	CmdPsClearReq(pAd, *pWcid, TRUE);
	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlStartPSRetrieve(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR *pWcid;
	pWcid = CMDQelmt->buffer;
	MTWF_LOG(DBG_CAT_PS, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("wcid=%d CmdPsRetrieveStartReq CMD to MCU\n", *pWcid));
	CmdPsRetrieveStartReq(pAd, *pWcid);
	return NDIS_STATUS_SUCCESS;
}
#endif

#ifdef ERR_RECOVERY
static INT ErrRecoveryMcuIntEvent(RTMP_ADAPTER *pAd, UINT32 status)
{
    UINT32 IntStatus = 0;
    IntStatus |= status;
    RTMP_IO_WRITE32(pAd, MT_MCU_INT_EVENT, IntStatus); /* write 1 to clear */
    return TRUE;
}

static INT ErrRecoveryStopPdmaAccess(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return FALSE;

    pErrRecoveryCtl->errRecovState = ERR_RECOV_STOP_PDMA0;
    return TRUE;
}

static INT ErrRecoveryReinitPdma(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return FALSE;

    pErrRecoveryCtl->errRecovState = ERR_RECOV_RESET_PDMA0;
    return TRUE;
}

static INT ErrRecoveryWaitN9Normal(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return FALSE;

    pErrRecoveryCtl->errRecovState = ERR_RECOV_WAIT_N9_NORMAL;
    return TRUE;
}

static INT ErrRecoveryEventReentry(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return FALSE;

    pErrRecoveryCtl->errRecovState = ERR_RECOV_EVENT_REENTRY;
    return TRUE;
}

static INT ErrRecoveryDone(ERR_RECOVERY_CTRL_T *pErrRecoveryCtl)
{
    if (pErrRecoveryCtl == NULL)
        return FALSE;

    pErrRecoveryCtl->errRecovState = ERR_RECOV_STOP_IDLE;
    return TRUE;
}

static UINT32 ErrRecoveryTimeDiff(UINT32 time1, UINT32 time2)
{
	UINT32 timeDiff = 0;

	if (time1 > time2)
	{
		timeDiff = (0xFFFFFFFF - time1 + 1) + time2;
	}
	else
	{
		timeDiff = time2 - time1;
	}
	return timeDiff;
}

void SerTimeLogDump(RTMP_ADAPTER *pAd)
{
	UINT32 idx = 0;
	UINT32 *pSerTimes = NULL;

	if (pAd == NULL)
	    return;

	pSerTimes = &pAd->HwCtrl.ser_times[0];

	for (idx = SER_TIME_ID_T0; idx < SER_TIME_ID_END; idx++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("%s,::E  R  , Time[%d](us)=%u\n", __FUNCTION__, idx,
		pSerTimes[idx]));
	}

	for (idx = SER_TIME_ID_T0; idx < (SER_TIME_ID_END - 1); idx++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
		("%s,::E  R  , T%d - T%d(us)=%u\n", __FUNCTION__, 
			idx + 1, idx, ErrRecoveryTimeDiff(pSerTimes[idx], 
					    pSerTimes[idx + 1])));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
	("%s,::E  R  , Total Time(us)=%u\n", __FUNCTION__, 
	        ErrRecoveryTimeDiff(pSerTimes[SER_TIME_ID_T0], 
	                            pSerTimes[SER_TIME_ID_T7])));
}


VOID ser_sys_reset(RTMP_STRING *arg)
{
#ifdef SDK_TIMER_WDG 
	//kernel_restart(NULL);
	panic(arg); //trigger SDK WATCHDOG TIMER
#endif /* SDK_TIMER_WDG */
}

static void ErrRecoveryEndDriverRestore(RTMP_ADAPTER *pAd)
{
	POS_COOKIE pObj;

	pObj = (POS_COOKIE) pAd->OS_Cookie;

	RTMP_OS_TASKLET_SCHE(&pObj->tr_done_task);
}

NTSTATUS HwRecoveryFromError(RTMP_ADAPTER *pAd)
{
    UINT32 Status;
    UINT32 Stat;
    ERR_RECOVERY_CTRL_T *pErrRecoveryCtrl;

    UINT32 Highpart,Lowpart;
    UINT32 *pSerTimes = NULL;

    if (pAd == NULL)
        return NDIS_STATUS_INVALID_DATA;

#ifdef CONFIG_ATE
    if (ATE_ON(pAd))
    {
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		("Ser():The driver is in ATE mode now\n"));
        return NDIS_STATUS_SUCCESS;
    }
#endif /* CONFIG_ATE */

    pErrRecoveryCtrl = &pAd->ErrRecoveryCtl;
    Status = pAd->HwCtrl.ser_status;
    Stat = ErrRecoveryCurStat(pErrRecoveryCtrl);
    pSerTimes = &pAd->HwCtrl.ser_times[0];


    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Ser                       ,::E  R  , stat=0x%08X\n", Stat));

    switch (Stat)
    {
        case ERR_RECOV_STOP_IDLE:
	case ERR_RECOV_EVENT_REENTRY:		
            if ((Status & ERROR_DETECT_STOP_PDMA) == ERROR_DETECT_STOP_PDMA)
            {
		os_zero_mem(pSerTimes,
			(sizeof(pSerTimes[SER_TIME_ID_T0]) * SER_TIME_ID_END));

		MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
		pSerTimes[SER_TIME_ID_T0] = Lowpart;

                /* Stop access PDMA. */
                ErrRecoveryStopPdmaAccess(pErrRecoveryCtrl);
                /* send PDMA0 stop to N9 through interrupt. */
                ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_STOP_DONE);
                /* all mmio need to be stop till hw reset done. */
                pAd->bPCIclkOff = TRUE;
        		RtmpusecDelay(100*1000); /* delay for 100 ms to wait reset done. */
                pAd->bPCIclkOff = FALSE;
		MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
		pSerTimes[SER_TIME_ID_T1] = Lowpart;
            }
            else
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
            }
            break;

        case ERR_RECOV_STOP_PDMA0:
            if ((Status & ERROR_DETECT_RESET_DONE) == ERROR_DETECT_RESET_DONE)
            {
                WPDMA_GLO_CFG_STRUC GloCfg;
		MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
		pSerTimes[SER_TIME_ID_T2] = Lowpart;

                HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);

                /* clean Tx/Rx rings and cut-through queue. */
                RTMPResetTxRxRingMemory(pAd);
                WfHifInit(pAd);
		RT28XXDMAEnable(pAd);
                RTMP_IO_WRITE32(pAd, MT_WPDMA_MEM_RNG_ERR, 0);
                ErrRecoveryReinitPdma(pErrRecoveryCtrl);

                /* send PDMA0 reinit done to N9 through interrupt. */
                ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_INIT_DONE);
		MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
		pSerTimes[SER_TIME_ID_T3] = Lowpart;
            }
            else
            {
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
            }
            break;
	case ERR_RECOV_RESET_PDMA0:
		if ((Status & ERROR_DETECT_RECOVERY_DONE)
			== ERROR_DETECT_RECOVERY_DONE)
		{
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
	    		pSerTimes[SER_TIME_ID_T4] = Lowpart;

			ErrRecoveryWaitN9Normal(pErrRecoveryCtrl);

			ErrRecoveryMcuIntEvent(pAd, MCU_INT_PDMA0_RECOVERY_DONE);

			pSerTimes[SER_TIME_ID_T5] = Lowpart;

		}
		else
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("!!! SER CurStat=%u Event=%x!!!\n",
			ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}
		break;

        case ERR_RECOV_WAIT_N9_NORMAL:
		if ((Status & ERROR_DETECT_N9_NORMAL_STATE)
	    		== ERROR_DETECT_N9_NORMAL_STATE)
		{
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;

			ErrRecoveryDone(pErrRecoveryCtrl);

			/* update Beacon frame if operating in AP mode. */
			UpdateBeaconHandler(
			pAd,
			NULL,
			AP_RENEW);

			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;

			/*print out ser log timing*/
			SerTimeLogDump(pAd);
			ErrRecoveryEndDriverRestore(pAd);
		}
		else if ((Status & ERROR_DETECT_STOP_PDMA)
			== ERROR_DETECT_STOP_PDMA)
		{
			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T6] = Lowpart;

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("!!! ERROR SER re-entry  CurStat=%u Event=%x!!!\n",
			ErrRecoveryCurStat(pErrRecoveryCtrl), Status));

			MtAsicGetTsfTimeByDriver(pAd, &Highpart, &Lowpart, HW_BSSID_0);
			pSerTimes[SER_TIME_ID_T7] = Lowpart;

			/*print out ser log timing*/
			SerTimeLogDump(pAd);
	                ErrRecoveryEventReentry(pErrRecoveryCtrl);
	                HwRecoveryFromError(pAd);
		}
		else
		{
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
		    ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
		}
		break;

        default:
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("!!! SER CurStat=%u Event=%x!!!\n", ErrRecoveryCurStat(pErrRecoveryCtrl), Status));
            break;
    }

    return NDIS_STATUS_SUCCESS;
}
#endif /* ERR_RECOVERY */

#endif

/*STA part*/


static NTSTATUS HwCtrlNICUpdateRawCounters(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	NICUpdateRawCounters(pAd);
	return NDIS_STATUS_SUCCESS;
}

/*Pheripheral Handler*/
static NTSTATUS HwCtrlCheckGPIO(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	return NDIS_STATUS_SUCCESS;
}

#ifdef LED_CONTROL_SUPPORT
static NTSTATUS HwCtrlSetLEDStatus(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UCHAR LEDStatus = *((PUCHAR)(CMDQelmt->buffer));

	RTMPSetLEDStatus(pAd, LEDStatus);

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: CMDTHREAD_SET_LED_STATUS (LEDStatus = %d)\n",
								__FUNCTION__, LEDStatus));

	return NDIS_STATUS_SUCCESS;
}
#endif /* LED_CONTROL_SUPPORT */

#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
/*WPS LED MODE 10*/
static NTSTATUS HwCtrlLEDWPSMode10(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	UINT WPSLedMode10 = *((PUINT)(CMDQelmt->buffer));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10::ON or Flash or OFF : %x\n", WPSLedMode10));

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
			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("WPS LED mode 10:: No this status %d!!!\n", WPSLedMode10));
			break;
	}
	return NDIS_STATUS_SUCCESS;
}
#endif /* WSC_LED_SUPPORT */
#endif /* WSC_INCLUDED */

#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
static NTSTATUS HwCtrlSetStaDWRR(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    MT_VOW_STA_GROUP *pVoW  =  (MT_VOW_STA_GROUP *)(CMDQelmt->buffer);

    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s: group %d, staid %d\n", __FUNCTION__, pVoW->GroupIdx, pVoW->StaIdx));
    vow_set_client(pAd, pVoW->GroupIdx, pVoW->StaIdx);

    return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlSetStaDWRRQuantum(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	INT32 ret;
    MT_VOW_STA_QUANTUM *pVoW  =  (MT_VOW_STA_QUANTUM *)(CMDQelmt->buffer);

    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: restore %d, quantum %d\x1b[m\n", __FUNCTION__, pVoW->restore, pVoW->quantum));

	if (pVoW->restore) {
		if(vow_watf_is_enabled(pAd)) {
			pAd->vow_cfg.vow_sta_dwrr_quantum[0] = pAd->vow_watf_q_lv0;
			pAd->vow_cfg.vow_sta_dwrr_quantum[1] = pAd->vow_watf_q_lv1;
			pAd->vow_cfg.vow_sta_dwrr_quantum[2] = pAd->vow_watf_q_lv2;
			pAd->vow_cfg.vow_sta_dwrr_quantum[3] = pAd->vow_watf_q_lv3;
		}
		else {
		        pAd->vow_cfg.vow_sta_dwrr_quantum[0] = VOW_STA_DWRR_QUANTUM0;
	    	        pAd->vow_cfg.vow_sta_dwrr_quantum[1] = VOW_STA_DWRR_QUANTUM1;
	    	        pAd->vow_cfg.vow_sta_dwrr_quantum[2] = VOW_STA_DWRR_QUANTUM2;
	    	        pAd->vow_cfg.vow_sta_dwrr_quantum[3] = VOW_STA_DWRR_QUANTUM3;
		}				
	}
	else {
		UINT8 ac;
		/* 4 ac with the same quantum */
		for (ac = 0; ac < WMM_NUM_OF_AC; ac++)
	    	pAd->vow_cfg.vow_sta_dwrr_quantum[ac] = pVoW->quantum;
 
	}

	ret = vow_set_sta(pAd, 0xff, ENUM_VOW_DRR_CTRL_FIELD_AIRTIME_QUANTUM_ALL);

    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("\x1b[31m%s: ret %d\x1b[m\n", __FUNCTION__, ret));
	
    return NDIS_STATUS_SUCCESS;
}

#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */

#ifdef THERMAL_PROTECT_SUPPORT
static NTSTATUS HwCtrlThermalProtRadioOff(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:\n", __FUNCTION__));
    /* Set Radio off Process*/
    Set_RadioOn_Proc(pAd, "0");

    return NDIS_STATUS_SUCCESS;
}
#endif /* THERMAL_PROTECT_SUPPORT */

static NTSTATUS HwCtrlUpdateRssi(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
    RssiUpdate(pAd);
    return NDIS_STATUS_SUCCESS;
}


static NTSTATUS HwCtrlGetTemperature(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
	UINT32 temperature=0;
	/*ActionIdx 0 means get temperature*/
	MtCmdGetThermalSensorResult(pAd,0,&temperature);
	os_move_mem(CMDQelmt->RspBuffer,&temperature,CMDQelmt->RspBufferLen);

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlGetTxStatistic(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
	TX_STAT_STRUC *pTxStat = (PTX_STAT_STRUC)CMDQelmt->buffer;
	PMAC_TABLE_ENTRY pEntry = &pAd->MacTab.Content[pTxStat->Wcid];
	EXT_EVENT_TX_STATISTIC_RESULT_T TxStatResult;

	MtCmdGetTxStatistic(pAd, pTxStat->Field, pTxStat->Band, pTxStat->Wcid, &TxStatResult);

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
    if (((TxStatResult.u4Field & GET_TX_STAT_ENTRY_TX_CNT) != 0) && pEntry->TxStatRspCnt)
        pEntry->TotalTxSuccessCnt += TxStatResult.u4EntryTxCount - TxStatResult.u4EntryTxFailCount;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): wcid(%d), TotalTxCnt(%u) - TotalTxFail(%u) = %u (%s)\n", 
		__FUNCTION__, pTxStat->Wcid, 
		TxStatResult.u4EntryTxCount, 
		TxStatResult.u4EntryTxFailCount, 
		pEntry->TotalTxSuccessCnt, 
		(pEntry->TxStatRspCnt)? "Vaild":"Invalid"));

	pEntry->TxStatRspCnt++;
#endif

	return NDIS_STATUS_SUCCESS;
}

static NTSTATUS HwCtrlRadioOnOff(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
    PRADIO_ON_OFF_T pRadioOnOff = (PRADIO_ON_OFF_T)CMDQelmt->buffer;
    AsicRadioOnOffCtrl(pAd, pRadioOnOff->ucDbdcIdx, pRadioOnOff->ucRadio);    
    return NDIS_STATUS_SUCCESS;
}

#ifdef LINK_TEST_SUPPORT
static NTSTATUS HwCtrlAutoLinkTest(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
	/* Link Test Time Slot Handler */
	LinkTestTimeSlotHandler(pAd);
	
    return NDIS_STATUS_SUCCESS;
}
#endif /* LINK_TEST_SUPPORT */

#ifdef GREENAP_SUPPORT
static NTSTATUS HwCtrlGreenAPOnOff(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
    PGREENAP_ON_OFF_T pGreenAP = (PGREENAP_ON_OFF_T)CMDQelmt->buffer;    
    AsicGreenAPOnOffCtrl(pAd, pGreenAP->ucDbdcIdx, pGreenAP->ucGreenAPOn);    
    return NDIS_STATUS_SUCCESS;
}
#endif /* GREENAP_SUPPORT */

static NTSTATUS HwCtrlSetSlotTime(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
	SLOT_CFG *pSlotCfg = (SLOT_CFG*)CMDQelmt->buffer;
	AsicSetSlotTime(pAd,pSlotCfg->bUseShortSlotTime,pSlotCfg->Channel,pSlotCfg->wdev);
	return NDIS_STATUS_SUCCESS;
}

#ifdef PKT_BUDGET_CTRL_SUPPORT
/*
*
*/
static NTSTATUS HwCtrlSetPbc(RTMP_ADAPTER *pAd,HwCmdQElmt *CMDQelmt)
{
	struct pbc_ctrl *pbc = (struct pbc_ctrl*)CMDQelmt->buffer;
	INT32 ret=0;
	UINT8 bssid = (pbc->wdev) ?  (pbc->wdev->bss_info_argument.ucBssIndex) : PBC_BSS_IDX_FOR_ALL;
	UINT16 wcid = (pbc->entry) ? (pbc->entry->wcid) : PBC_WLAN_IDX_FOR_ALL;
	ret = MtCmdPktBudgetCtrl(pAd,bssid,wcid,pbc->type);
	return ret;
}
#endif /*PKT_BUDGET_CTRL_SUPPORT*/

/*
*
*/
static VOID hwRunWifiSysCtrl(RTMP_ADAPTER *pAd,WIFI_SYS_CTRL *wifi_sys_ctrl)
{
	STA_REC_CTRL_T *sta_rec_ctrl = &wifi_sys_ctrl->StaRecCtrl;
	DEV_INFO_CTRL_T *dev_ctrl=&wifi_sys_ctrl->DevInfoCtrl;

	if(dev_ctrl->EnableFeature)
	{
		AsicDevInfoUpdate(
			pAd,
			dev_ctrl->OwnMacIdx,
			dev_ctrl->OwnMacAddr,
			dev_ctrl->BandIdx,
			dev_ctrl->Active,
			dev_ctrl->EnableFeature
		);
	}

	if(wifi_sys_ctrl->BssInfoCtrl.u4BssInfoFeature)
	{
		AsicBssInfoUpdate(pAd,wifi_sys_ctrl->BssInfoCtrl);
	}

	if(sta_rec_ctrl->EnableFeature)
	{
		AsicStaRecUpdate(pAd,
			wifi_sys_ctrl->wdev,
			sta_rec_ctrl->BssIndex,
			sta_rec_ctrl->WlanIdx,
			sta_rec_ctrl->ConnectionType,
			sta_rec_ctrl->ConnectionState,
			sta_rec_ctrl->EnableFeature,
			sta_rec_ctrl->IsNewSTARec
		);
	}
}

static NTSTATUS HwCtrlWifiSysOpen(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	
	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysClose(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	struct wifi_dev *wdev = wifi_sys_ctrl->wdev;

	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	if(HcIsRadioAcq(wdev))
	{	
		HcReleaseRadioForWdev(pAd,wdev);
	}
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	struct wifi_dev *wdev = wifi_sys_ctrl->wdev;
	UINT16 txop_level = TXOP_0;

	
	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	if(wifi_sys_ctrl->BssInfoCtrl.u4BssInfoFeature)
	{
		HcSetEdca(wdev);
	}

	if (pAd->CommonCfg.bEnableTxBurst) {
		txop_level = TXOP_60;
		if (pAd->CommonCfg.bRdg)
			txop_level = TXOP_80;
	} else {
		txop_level = TXOP_0;
	}
	hw_set_tx_burst(pAd, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);
            
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	struct wifi_dev *wdev = wifi_sys_ctrl->wdev;

	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	if (!wifi_sys_ctrl->skip_set_txop)
		hw_set_tx_burst(pAd, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);

	if(wifi_sys_ctrl->BssInfoCtrl.u4BssInfoFeature)
	{
		HcReleaseEdca(pAd,wdev);
	}

	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkDown(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	struct wifi_dev *wdev = wifi_sys_ctrl->wdev;
	STA_REC_CTRL_T *sta_rec = &wifi_sys_ctrl->StaRecCtrl;

	HcReleaseUcastWcid(pAd,wdev,sta_rec->WlanIdx);

	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	/* Delete this entry from ASIC on-chip WCID Table*/
	if (!((sta_rec->WlanIdx >= GET_MAX_UCAST_NUM(pAd))&&(sta_rec->WlanIdx != WCID_ALL)))
	{
		AsicDelWcidTab(pAd,sta_rec->WlanIdx);
	}
	if (!wifi_sys_ctrl->skip_set_txop)
		hw_set_tx_burst(pAd, wdev, AC_BE, PRIO_DEFAULT, TXOP_0, 0);

	switch(wdev->wdev_type){
#ifdef CONFIG_AP_SUPPORT
	case WDEV_TYPE_AP:
	{		
		ADD_HT_INFO_IE *addht = wlan_operate_get_addht(wdev);
		/* back to default protection */
	    wdev->protection = 0;
		addht->AddHtInfo2.OperaionMode=0;
		UpdateBeaconHandler(pAd,wdev,IE_CHANGE);
	    AsicUpdateProtect(pAd, 0, ALLN_SETPROTECT, TRUE, FALSE);
	}
	break;
#endif /*CONFIG_AP_SUPPORT*/
	}
	
	return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerLinkUp(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	UCHAR wcid = wifi_sys_ctrl->StaRecCtrl.WlanIdx;
	struct wifi_dev *wdev = wifi_sys_ctrl->wdev;
	PEER_LINKUP_HWCTRL *lu_ctrl = (PEER_LINKUP_HWCTRL*)wifi_sys_ctrl->priv;
	UINT16 txop_level=TXOP_0;
	
	hwRunWifiSysCtrl(pAd,wifi_sys_ctrl);

	if (pAd->CommonCfg.bEnableTxBurst) {
		txop_level = TXOP_60;
		if (pAd->CommonCfg.bRdg) 
			txop_level = TXOP_80;
	} else {
		txop_level = TXOP_0;
	}
	hw_set_tx_burst(pAd, wdev, AC_BE, PRIO_DEFAULT, txop_level, 1);

	if(wdev->wdev_type == WDEV_TYPE_AP)
	{
#ifdef DOT11_N_SUPPORT
		if(lu_ctrl)
		{
		    if (lu_ctrl->bRdgCap)
		    {
		        AsicSetRDG(pAd, wcid, HcGetBandByWdev(wdev), 1, 1);
		    }
		}
#endif
	}

	
	if(lu_ctrl)
	{
		os_free_mem(lu_ctrl);
	}
	
    return NDIS_STATUS_SUCCESS;
}


/*
*
*/
static NTSTATUS HwCtrlWifiSysPeerUpdate(RTMP_ADAPTER *pAd, HwCmdQElmt *CMDQelmt)
{
	WIFI_SYS_CTRL *wifi_sys_ctrl =(WIFI_SYS_CTRL*)CMDQelmt->buffer;
	STA_REC_CTRL_T *sta_rec_ctrl = &wifi_sys_ctrl->StaRecCtrl;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	UINT32 featues=0;
	/*update ra rate*/
	if((sta_rec_ctrl->EnableFeature & STA_REC_RA_UPDATE_FEATURE) && wifi_sys_ctrl->priv)
	{
		AsicRaParamStaRecUpdate(pAd,
		sta_rec_ctrl->WlanIdx,
		(CMD_STAREC_AUTO_RATE_UPDATE_T*)wifi_sys_ctrl->priv,
		STA_REC_RA_UPDATE_FEATURE);

		if(wifi_sys_ctrl->priv)
		{
			os_free_mem(wifi_sys_ctrl->priv);
		}
		return NDIS_STATUS_SUCCESS;
	}

	if(sta_rec_ctrl->EnableFeature & STA_REC_RA_FEATURE)
	{

		featues = STA_REC_RA_FEATURE;
		AsicStaRecUpdate(pAd,
			wifi_sys_ctrl->wdev,
			sta_rec_ctrl->BssIndex,
			sta_rec_ctrl->WlanIdx,
			sta_rec_ctrl->ConnectionType,
			sta_rec_ctrl->ConnectionState,
			featues,
			sta_rec_ctrl->IsNewSTARec
		);
		
		sta_rec_ctrl->EnableFeature &= (~STA_REC_RA_FEATURE);
	}
#endif /*RACTRL_FW_OFFLOAD_SUPPORT*/

	/*normal update*/
	if(sta_rec_ctrl->EnableFeature)
	{
		AsicStaRecUpdate(pAd,
			wifi_sys_ctrl->wdev,
			sta_rec_ctrl->BssIndex,
			sta_rec_ctrl->WlanIdx,
			sta_rec_ctrl->ConnectionType,
			sta_rec_ctrl->ConnectionState,
			sta_rec_ctrl->EnableFeature,
			sta_rec_ctrl->IsNewSTARec
		);
	}

	
    return NDIS_STATUS_SUCCESS;
}



/*HWCMD_TYPE_RADIO*/
static HW_CMD_TABLE_T HwCmdRadioTable[] = {
	{HWCMD_ID_UPDATE_DAW_COUNTER,HwCtrlUpdateRawCounters,0},
#ifdef MT_MAC
	{HWCMD_ID_SET_CLIENT_MAC_ENTRY,HwCtrlSetClientMACEntry,0},
#ifdef TXBF_SUPPORT	
    {HWCMD_ID_SET_APCLI_BF_CAP,HwCtrlSetClientBfCap,0},
    {HWCMD_ID_SET_APCLI_BF_REPEATER, HwCtrlSetBfRepeater, 0},
    {HWCMD_ID_ADJUST_STA_BF_SOUNDING, HwCtrlAdjBfSounding, 0},
    {HWCMD_ID_TXBF_TX_APPLY_CTRL, HwCtrlTxBfTxApply, 0}, 
#endif    
	{HWCMD_ID_SET_TR_ENTRY,HwCtrlSetTREntry,0},
	{HWCMD_ID_SET_BA_REC, HwCtrlSetBaRec,0},
	{HWCMD_ID_UPDATE_BSSINFO,HwCtrlUpdateBssInfo,0},
	{HWCMD_ID_UPDATE_BEACON,HwCtrlHandleUpdateBeacon,0},
	{HWCMD_ID_SET_TX_BURST,HwCtrlSetTxBurst,0},
#endif /*MT_MAC*/
#ifdef CONFIG_AP_SUPPORT
	{HWCMD_ID_AP_ADJUST_EXP_ACK_TIME,HwCtrlAPAdjustEXPAckTime,0},
	{HWCMD_ID_AP_RECOVER_EXP_ACK_TIME,	HwCtrlAPRecoverEXPAckTime,0},
#endif
#ifdef VOW_SUPPORT
#ifdef CONFIG_AP_SUPPORT
    {HWCMD_ID_SET_STA_DWRR, HwCtrlSetStaDWRR,0},
	{HWCMD_ID_SET_STA_DWRR_QUANTUM, HwCtrlSetStaDWRRQuantum,0},
#endif /* CONFIG_AP_SUPPORT */
#endif /* VOW_SUPPORT */
	{HWCMD_ID_UPDATE_RSSI, HwCtrlUpdateRssi,0},
	{HWCMD_ID_GET_TEMPERATURE,HwCtrlGetTemperature,0},
	{HWCMD_ID_SET_SLOTTIME,HwCtrlSetSlotTime,0},
#ifdef BCN_OFFLOAD_SUPPORT
    {HWCMD_ID_SET_BCN_OFFLOAD, HwCtrlSetBcnOffload,0},
#endif
#ifdef MAC_REPEATER_SUPPORT
    {HWCMD_ID_ADD_REPT_ENTRY, HwCtrlAddReptEntry,0},
    {HWCMD_ID_REMOVE_REPT_ENTRY, HwCtrlRemoveReptEntry,0},
#endif
#ifdef THERMAL_PROTECT_SUPPORT
    {HWCMD_ID_THERMAL_PROTECTION_RADIOOFF, HwCtrlThermalProtRadioOff,0},
#endif /* THERMAL_PROTECT_SUPPORT */
	{HWCMD_ID_RADIO_ON_OFF, HwCtrlRadioOnOff, 0}, 
#ifdef LINK_TEST_SUPPORT
	{HWCMD_ID_AUTO_LINK_TEST, HwCtrlAutoLinkTest, 0}, 
#endif /* LINK_TEST_SUPPORT */ 	
#ifdef GREENAP_SUPPORT	
	{HWCMD_ID_GREENAP_ON_OFF, HwCtrlGreenAPOnOff, 0}, 
#endif /* GREENAP_SUPPORT */	
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_SECURITY*/
static HW_CMD_TABLE_T HwCmdSecurityTable[] = {
	{HWCMD_ID_ADDREMOVE_ASIC_KEY,HwCtrlAddRemoveKeyTab,0},
	{HWCMD_ID_SET_WCID_SEC_INFO,HwCtrlSetWcidSecInfo,0},
	{HWCMD_ID_SET_ASIC_WCID_ATTR,HwCtrlAsicWcidAttr,0},
	{HWCMD_ID_SET_ASIC_WCID_IVEIV,HwCtrlSetAsicWcidIVEIV,0},
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_PERIPHERAL*/
static HW_CMD_TABLE_T HwCmdPeripheralTable[]={
	{HWCMD_ID_GPIO_CHECK,HwCtrlCheckGPIO,0},
#ifdef WSC_INCLUDED
#ifdef WSC_LED_SUPPORT
	{HWCMD_ID_LED_WPS_MODE10,HwCtrlLEDWPSMode10,0},
#endif
#endif
#ifdef LED_CONTROL_SUPPORT
	{HWCMD_ID_SET_LED_STATUS,HwCtrlSetLEDStatus,0},
#endif
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_HT_CAP*/
static HW_CMD_TABLE_T HwCmdHtCapTable[]={
	{HWCMD_ID_DEL_ASIC_WCID,HwCtrlDelAsicWcid,0},
#ifdef HTC_DECRYPT_IOT
	{HWCMD_ID_SET_ASIC_AAD_OM,HwCtrlSetAsicWcidAAD_OM,0},
#endif /* HTC_DECRYPT_IOT */
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_PS*/
static HW_CMD_TABLE_T HwCmdPsTable[]={
#ifdef MT_MAC
#ifdef MT_PS
	{HWCMD_ID_PS_CLEAR,HwCtrlClearPSRetrieveToken,0},
	{HWCMD_ID_PS_RETRIEVE_START,HwCtrlStartPSRetrieve,0},
#endif
#endif
#ifdef MLME_BY_CMDTHREAD
	{HWCMD_ID_MLME_BY_CMDTHREAD, MlmePeriodicExec,0},
#endif
	{HWCMD_ID_PERODIC_CR_ACCESS_NIC_UPDATE_RAW_COUNTERS, HwCtrlNICUpdateRawCounters,0},
	{HWCMD_ID_END,NULL,0}
};


/*HWCMD_TYPE_WIFISYS*/
static HW_CMD_TABLE_T HwCmdWifiSysTable[]={
	{HWCMD_ID_WIFISYS_LINKDOWN, HwCtrlWifiSysLinkDown,0},
	{HWCMD_ID_WIFISYS_LINKUP,HwCtrlWifiSysLinkUp,0},
	{HWCMD_ID_WIFISYS_OPEN,HwCtrlWifiSysOpen,0},
	{HWCMD_ID_WIFISYS_CLOSE,HwCtrlWifiSysClose,0},
	{HWCMD_ID_WIFISYS_PEER_LINKDOWN,HwCtrlWifiSysPeerLinkDown,0},	
	{HWCMD_ID_WIFISYS_PEER_LINKUP,HwCtrlWifiSysPeerLinkUp,0},
	{HWCMD_ID_WIFISYS_PEER_UPDATE,HwCtrlWifiSysPeerUpdate,0},
	{HWCMD_ID_GET_TX_STATISTIC, HwCtrlGetTxStatistic, 0},
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_WMM*/
static HW_CMD_TABLE_T HwCmdWmmTable[]={
#ifdef PKT_BUDGET_CTRL_SUPPORT
	{HWCMD_ID_PBC_CTRL,HwCtrlSetPbc,0},
#endif /*PKT_BUDGET_CTRL_SUPPORT*/
	{HWCMD_ID_END,NULL,0}
};

/*HWCMD_TYPE_PROTECT*/
static HW_CMD_TABLE_T HwCmdProtectTable[] = {
	{HWCMD_ID_RTS_THLD, HwCtrlUpdateRtsThreshold, 0},
	{HWCMD_ID_END, NULL, 0}
};

/*Order can't be changed, follow HW_CMD_TYPE order definition*/
HW_CMD_TABLE_T *HwCmdTable[] = {
	HwCmdRadioTable,
	HwCmdSecurityTable,
	HwCmdPeripheralTable,
	HwCmdHtCapTable,
	HwCmdPsTable,
	HwCmdWifiSysTable,
	HwCmdWmmTable,
	HwCmdProtectTable,
	NULL
};



HW_FLAG_TABLE_T HwFlagTable[]={
	{HWFLAG_ID_UPDATE_PROTECT, HwCtrlUpdateProtect,0},
	{HWFLAG_ID_END,NULL,0}
};

