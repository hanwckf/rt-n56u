/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_asic_mt.c

	Abstract:
	Functions used to communicate with ASIC

	Revision History:
	Who			When			What
	--------	----------		----------------------------------------------
*/
#ifdef COMPOS_WIN
#include "MtConfig.h"
#elif defined (COMPOS_TESTMODE_WIN)
#include "config.h"
#else
#include "rt_config.h"
#endif

#include "hdev/hdev.h"

extern const UCHAR wmm_aci_2_hw_ac_queue[18];

#if defined(MT7615)
VOID PciResetPDMAToMakeSurePreFetchIndexCorrect(RTMP_ADAPTER *pAd)
{
    if (IS_MT7615(pAd))
    {
        UINT32 value = 0;

        /* <1> disable the clock gating(bit 30) */
        HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &value);
        value |= CLK_GATE_DIS; 
        HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, value);

        /* <2> do pdma0 hw reset(bit 24) */
        HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &value);
        value |= SW_RST;
        HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, value);

        /* delay 1ms */
    	RtmpOsMsDelay(1);

        /* <3> start to filter pkt and init the drop pkt count. */
    	pAd->RxRest = 1;
    	pAd->RxResetDropCount = 0;
    }    
}

VOID EnhancedPDMAInit(RTMP_ADAPTER *pAd)
{
    UINT32 value = 0;

    /* PDMA related features for MT7615 E3 */
    value = TXP_ACTIVE_MODE(0x1);
    HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG_1, value);

    value = TX_RING0_DMAD_PRE_HI_PRI_NUM(0x3) | TX_RING1_DMAD_PRE_HI_PRI_NUM(0x3);
    HIF_IO_WRITE32(pAd, MT_WPDMA_TX_PRE_CFG, value);

    value = RX_RING0_DMAD_PRE_HI_PRI_NUM(0x7f) | RX_RING1_DMAD_PRE_HI_PRI_NUM(0xf);
    HIF_IO_WRITE32(pAd, MT_WPDMA_RX_PRE_CFG, value);

    value = TRX_DFET_ABT_TYPE(0x2) |
            TRX_PFET_ABT_TYPE(0x1) |
            PDMA_DFET_PFET_ABT_TYPE(0x2) |
            WRR_TIME_SLOT_DURATION(0x400);
    HIF_IO_WRITE32(pAd, MT_WPDMA_ABT_CFG, value);

    value = WRR_TIME_SLOT_PRIORITY_0(0x1)  |
            WRR_TIME_SLOT_PRIORITY_1(0x0)  |
            WRR_TIME_SLOT_PRIORITY_2(0x0)  |
            WRR_TIME_SLOT_PRIORITY_3(0x2)  |
            WRR_TIME_SLOT_PRIORITY_4(0x0)  |
            WRR_TIME_SLOT_PRIORITY_5(0x2)  |
            WRR_TIME_SLOT_PRIORITY_6(0x1)  |
            WRR_TIME_SLOT_PRIORITY_7(0x0)  |
            WRR_TIME_SLOT_PRIORITY_8(0x1)  |
            WRR_TIME_SLOT_PRIORITY_9(0x0)  |
            WRR_TIME_SLOT_PRIORITY_10(0x0) |
            WRR_TIME_SLOT_PRIORITY_11(0x2) |
            WRR_TIME_SLOT_PRIORITY_12(0x0) |
            WRR_TIME_SLOT_PRIORITY_13(0x2) |
            WRR_TIME_SLOT_PRIORITY_14(0x1) |
            WRR_TIME_SLOT_PRIORITY_15(0x0);
    HIF_IO_WRITE32(pAd, MT_WPDMA_ABT_CFG1, value);
}

#endif


#ifdef CONFIG_AP_SUPPORT
/* because of the CR arrangement in HW are not in sequence, wrape the table to search fast.*/
static UINT32 LPON_TT0SBOR_CR_MAPPING_TABLE[] =
{
    LPON_TT0TPCR,
    LPON_TT0SBOR1,
    LPON_TT0SBOR2,
    LPON_TT0SBOR3,
    LPON_TT0SBOR4,
    LPON_TT0SBOR5,
    LPON_TT0SBOR6,
    LPON_TT0SBOR7,
    LPON_TT0SBOR8,
    LPON_TT0SBOR9,
    LPON_TT0SBOR10,
    LPON_TT0SBOR11,
    LPON_TT0SBOR12,
    LPON_TT0SBOR13,
    LPON_TT0SBOR14,
    LPON_TT0SBOR15,
};
#endif /*CONFIG_AP_SUPPORT*/

static BOOLEAN WtblWaitIdle(RTMP_ADAPTER *pAd,UINT32 WaitCnt, UINT32 WaitDelay)
{
	UINT32 Value,CurCnt=0;

	do {
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
		if ((Value & WTBL_IU_BUSY)  == 0)
			break;
		CurCnt++;
		RtmpusecDelay(WaitDelay);
	} while (CurCnt < WaitCnt);

	if (CurCnt == WaitCnt) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
				("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
				__FUNCTION__, Value));
		return FALSE;
	}

	return TRUE;
}


UINT32 MtAsicGetCrcErrCnt(RTMP_ADAPTER *pAd)
{
	return 0;
}


UINT32 MtAsicGetPhyErrCnt(RTMP_ADAPTER *pAd)
{
	return 0;
}


UINT32 MtAsicGetCCACnt(RTMP_ADAPTER *pAd)
{
	return 0;
}

UINT32 MtAsicGetChBusyCnt(RTMP_ADAPTER *pAd, UCHAR ch_idx)
{

#if defined(MT7615) || defined(MT7622)
#ifdef CONFIG_AP_SUPPORT
        UINT32	msdr16;

// TODO: shiang-MT7615, fix me!
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s----------------->\n",__FUNCTION__));

        if (pAd->CommonCfg.dbdc_mode == 0)
        {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: SB Band0\n",__FUNCTION__)); 
	        MAC_IO_READ32(pAd, MIB_M0SDR16, &msdr16);
        }
        else if(pAd->AutoChSelCtrl.IsABand == TRUE)
        {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: DBDC Band1\n",__FUNCTION__));
	        MAC_IO_READ32(pAd, MIB_M1SDR16, &msdr16);                       
        }
        else
        {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: DBDC Band0\n",__FUNCTION__));        
	        MAC_IO_READ32(pAd, MIB_M0SDR16, &msdr16);                
        }
	msdr16 &= 0x00ffffff;
        
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s<-----------------\n",__FUNCTION__));    
	return msdr16;
 #endif/*CONFIG_AP_SUPPORT*/   
#endif /* defined(MT7615) || defined(MT7622) */
return 0;
}




INT MtAsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


INT32 MtAsicAutoFallbackInit(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, AGG_ARUCR, &Value);
	Value &= ~RATE1_UP_MPDU_LIMIT_MASK;
	Value |= RATE1_UP_MPDU_LINIT(2);
	Value &= ~RATE2_UP_MPDU_LIMIT_MASK;
	Value |= RATE2_UP_MPDU_LIMIT(2);
	Value &= ~RATE3_UP_MPDU_LIMIT_MASK;
	Value |= RATE3_UP_MPDU_LIMIT(2);
	Value &= ~RATE4_UP_MPDU_LIMIT_MASK;
	Value |= RATE4_UP_MPDU_LIMIT(2);
	Value &= ~RATE5_UP_MPDU_LIMIT_MASK;
	Value |= RATE5_UP_MPDU_LIMIT(1);
	Value &= ~RATE6_UP_MPDU_LIMIT_MASK;
	Value |= RATE6_UP_MPDU_LIMIT(1);
	Value &= ~RATE7_UP_MPDU_LIMIT_MASK;
	Value |= RATE7_UP_MPDU_LIMIT(1);
	Value &= ~RATE8_UP_MPDU_LIMIT_MASK;
	Value |= RATE8_UP_MPDU_LIMIT(1);
	MAC_IO_WRITE32(pAd, AGG_ARUCR, Value);

	MAC_IO_READ32(pAd, AGG_ARDCR, &Value);
	Value &= ~RATE1_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE1_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE2_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE2_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE3_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE3_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE4_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE4_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE5_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE5_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE6_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE6_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE7_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE7_DOWN_MPDU_LIMIT(0);
	Value &= ~RATE8_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE8_DOWN_MPDU_LIMIT(0);
	MAC_IO_WRITE32(pAd, AGG_ARDCR, Value);

	MAC_IO_READ32(pAd, AGG_ARCR, &Value);
	Value |= INI_RATE1;
	Value |= FB_SGI_DIS;
	Value &= ~RTS_RATE_DOWN_TH_MASK;
	Value &= ~RATE_DOWN_EXTRA_RATIO_MASK;
	Value |= RATE_DOWN_EXTRA_RATIO(2);
	Value |= RATE_DOWN_EXTRA_RATIO_EN;
	Value &= ~RATE_UP_EXTRA_TH_MASK;
	Value |= RATE_UP_EXTRA_TH(4);
	MAC_IO_WRITE32(pAd, AGG_ARCR, Value);

	Value = tx_rate_to_tmi_rate(MODE_HTMIX, MCS_1, 1, FALSE, 0);
	MAC_IO_WRITE32(pAd, AGG_ARCR1, Value);

	return TRUE;
}


/*
	========================================================================

	Routine Description:
		Set MAC register value according operation mode.
		OperationMode AND bNonGFExist are for MM and GF Proteciton.
		If MM or GF mask is not set, those passing argument doesn't not take effect.

		Operation mode meaning:
		= 0 : Pure HT, no preotection.
		= 0x01; there may be non-HT devices in both the control and extension channel, protection is optional in BSS.
		= 0x10: No Transmission in 40M is protected.
		= 0x11: Transmission in both 40M and 20M shall be protected
		if (bNonGFExist)
			we should choose not to use GF. But still set correct ASIC registers.
	========================================================================
*/
VOID MtAsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN MT_PROTECT_CTRL_T *Protect)
{
	UINT32 val = 0;

	MAC_IO_READ32(pAd, AGG_PCR, &val);

    /* B/G Protection */
    val &= ~ERP_PROTECTION_MASK;
    val |= ERP_PROTECTION(Protect->erp_mask);

    /* RIFS Protection */
    val &= ~RIFS_PROTECTION;
    if (Protect->rifs) {
        /* TX_RIFS_EN of RCR is enable,
         * HW will switch to Long NAV protection,
         * ignoring PROTECTION_MODE bit */
        val |= RIFS_PROTECTION;
    }

    /* NAV Protection */
    if (Protect->long_nav) {
		val &= ~PROTECTION_MODE;
    }
    else {
		val |= PROTECTION_MODE;
    }

    /* Green Field Protectio */
    val &= ~GF_PROTECTION;
    if (Protect->gf) {
        val |= GF_PROTECTION;
    }

    /* Other HT Protection */
    val &= ~(MM_PROTECTION|BW40_PROTECTION
            |BW80_PROTECTION|BW160_PROTECTION);
    if (Protect->mix_mode) {
        val |= MM_PROTECTION;
    }

    if (Protect->bw40) {
        val |= BW40_PROTECTION;
    }

    if (Protect->bw80) {
        val |= BW80_PROTECTION;
    }

    if (Protect->bw160) {
        val |= BW160_PROTECTION;
    }

	MAC_IO_WRITE32(pAd, AGG_PCR, val);

	return ;
}




/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */

VOID MtAsicSwitchChannel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	UINT32 val,reg;
	// TODO: Need to fix
	// TODO: shiang-usw, unify the pAd->chipOps
	if (pAd->chipOps.ChipSwitchChannel) {
		pAd->chipOps.ChipSwitchChannel(pAd, SwChCfg);
    } else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("For this chip, no specified channel switch function!\n"));
	}

	// TODO: shiang-7615
	if (SwChCfg.BandIdx) {
		reg = RMAC_CHFREQ1;
	} else {
		reg = RMAC_CHFREQ0;
	}
	MAC_IO_READ32(pAd, reg, &val);
	val &= (~0xff);
	val |= SwChCfg.ControlChannel;
	MAC_IO_WRITE32(pAd, reg, val);
}

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
#ifdef ANT_DIVERSITY_SUPPORT
VOID MtAsicAntennaSelect(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));

	return;
}
#endif /* ANT_DIVERSITY_SUPPORT */


VOID MtAsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
	/* Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	*/
	/* IF chipOps.AsicResetBbpAgent == NULL, run "else" part */
#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me!
	return;
#endif /* defined(MT7615) || defined(MT7622) */

	// TODO: shiang-usw, unify the pAd->chipOps
	if (pAd->chipOps.AsicResetBbpAgent != NULL)
	{
		pAd->chipOps.AsicResetBbpAgent(pAd);
	}
}

/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
 /* CFG_TODO */
#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfMacs)
{
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): No Function for HIF_MT!\n",
                __FUNCTION__, __LINE__));
    return;
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
// TODO: Carter/Star for Repeater can support DBDC, after define STA/APCLI/Repeater
INT MtAsicSetReptFuncEnableByDriver(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	RMAC_MORE_STRUC rmac_more;

	MAC_IO_READ32(pAd, RMAC_MORE, &rmac_more.word);
	if (bEnable == 0)
	{
		rmac_more.field.muar_mode_sel = 0;
	}
	else
	{
		rmac_more.field.muar_mode_sel = 1;
	}
    /* configure band 0/band 1 into repeater mode concurrently. */
	MAC_IO_WRITE32(pAd, RMAC_MORE, rmac_more.word);
    MAC_IO_WRITE32(pAd, RMAC_MORE_BAND_1, rmac_more.word);

	return TRUE;
}


VOID MtAsicInsertRepeaterEntryByDriver(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR tempMAC[MAC_ADDR_LEN];
    RMAC_MAR0_STRUC rmac_mcbcs0;
    RMAC_MAR1_STRUC rmac_mcbcs1;

	COPY_MAC_ADDR(tempMAC, pAddr);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n%s %02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
			__FUNCTION__, tempMAC[0], tempMAC[1], tempMAC[2], tempMAC[3], tempMAC[4], tempMAC[5] , CliIdx));

	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = tempMAC[4];
	rmac_mcbcs1.field.addr_47_40 = tempMAC[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	rmac_mcbcs1.field.multicast_addr_index = (CliIdx * 2);
	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);

	return;
}


VOID MtAsicRemoveRepeaterEntryByDriver(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
    RMAC_MAR0_STRUC rmac_mcbcs0;
    RMAC_MAR1_STRUC rmac_mcbcs1;

    NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
    MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

    NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
    rmac_mcbcs1.field.access_start = 1;
    rmac_mcbcs1.field.readwrite = 1;

    rmac_mcbcs1.field.multicast_addr_index = (CliIdx * 2);//start from idx 0

    MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);//clear client entry first.

    NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
    MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

    NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
    rmac_mcbcs1.field.access_start = 1;
    rmac_mcbcs1.field.readwrite = 1;

    rmac_mcbcs1.field.multicast_addr_index = ((CliIdx * 2) + 1);
    MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);//clear rootap entry.
}



VOID MtAsicInsertRepeaterRootEntryByDriver(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR Wcid,
	IN UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = pAddr[4];
	rmac_mcbcs1.field.addr_47_40 = pAddr[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;

	rmac_mcbcs1.field.multicast_addr_index = (ReptCliIdx * 2) + 1;

	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);

}

#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


static VOID RxFilterCfg2Row(UINT32 FilterMask,UINT32 *RowFilterMask)
{

	*RowFilterMask = FilterMask & RX_STBC_BCN_BC_MC ?
					(*RowFilterMask | DROP_STBC_BCN_BC_MC) : (*RowFilterMask & ~(DROP_STBC_BCN_BC_MC));

	*RowFilterMask = FilterMask & RX_FCS_ERROR ?
					(*RowFilterMask | DROP_FCS_ERROR_FRAME) : (*RowFilterMask & ~(DROP_FCS_ERROR_FRAME));

	*RowFilterMask = FilterMask & RX_PROTOCOL_VERSION ?
					(*RowFilterMask | DROP_VERSION_NO_0) : (*RowFilterMask & ~(DROP_VERSION_NO_0));

	*RowFilterMask = FilterMask & RX_PROB_REQ ?
					(*RowFilterMask | DROP_PROBE_REQ) : (*RowFilterMask & ~(DROP_PROBE_REQ));

	*RowFilterMask = FilterMask & RX_MC_ALL ?
					(*RowFilterMask | DROP_MC_FRAME) : (*RowFilterMask & ~(DROP_MC_FRAME));

	*RowFilterMask = FilterMask & RX_BC_ALL ?
					(*RowFilterMask | DROP_BC_FRAME) : (*RowFilterMask & ~(DROP_BC_FRAME));

	*RowFilterMask = FilterMask & RX_MC_TABLE ?
					(*RowFilterMask | DROP_NOT_IN_MC_TABLE) : (*RowFilterMask & ~(DROP_NOT_IN_MC_TABLE));

	*RowFilterMask = FilterMask & RX_BC_MC_OWN_MAC_A3 ?
					(*RowFilterMask | DROP_ADDR3_OWN_MAC) : (*RowFilterMask & ~(DROP_ADDR3_OWN_MAC));

	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A3 ?
					(*RowFilterMask | DROP_DIFF_BSSID_A3) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A3));

	*RowFilterMask = FilterMask & RX_BC_MC_DIFF_BSSID_A2 ?
					(*RowFilterMask | DROP_DIFF_BSSID_A2) : (*RowFilterMask & ~(DROP_DIFF_BSSID_A2));

	*RowFilterMask = FilterMask & RX_BCN_DIFF_BSSID ?
					(*RowFilterMask | DROP_DIFF_BSSID_BCN) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BCN));

	*RowFilterMask = FilterMask & RX_CTRL_RSV ?
					(*RowFilterMask | DROP_CTRL_RSV) : (*RowFilterMask & ~(DROP_CTRL_RSV));

	*RowFilterMask = FilterMask & RX_CTS ?
					(*RowFilterMask | DROP_CTS) : (*RowFilterMask & ~(DROP_CTS));

	*RowFilterMask = FilterMask & RX_RTS ?
					(*RowFilterMask | DROP_RTS) : (*RowFilterMask & ~(DROP_RTS));

	*RowFilterMask = FilterMask & RX_DUPLICATE ?
					(*RowFilterMask | DROP_DUPLICATE) : (*RowFilterMask & ~(DROP_DUPLICATE));

	*RowFilterMask = FilterMask & RX_NOT_OWN_BSSID ?
					(*RowFilterMask | DROP_NOT_MY_BSSID) : (*RowFilterMask & ~(DROP_NOT_MY_BSSID));

	*RowFilterMask = FilterMask & RX_NOT_OWN_UCAST?
					(*RowFilterMask | DROP_NOT_UC2ME) : (*RowFilterMask & ~(DROP_NOT_UC2ME));

	*RowFilterMask = FilterMask & RX_NOT_OWN_BTIM ?
					(*RowFilterMask | DROP_DIFF_BSSID_BTIM) : (*RowFilterMask & ~(DROP_DIFF_BSSID_BTIM));

	*RowFilterMask = FilterMask & RX_NDPA ?
					(*RowFilterMask | DROP_NDPA) : (*RowFilterMask & ~(DROP_NDPA));

}


INT MtAsicSetRxFilter(RTMP_ADAPTER *pAd,MT_RX_FILTER_CTRL_T RxFilter)
{
	UINT32 Value=0;


	if(RxFilter.bPromiscuous)
	{
#ifdef SNIFFER_MT7615
		if (IS_MT7615(pAd))
		{
			Value = DROP_FCS_ERROR_FRAME | RM_FRAME_REPORT_EN | RX_UNWANTED_CTL_FRM;
		}
		else
#endif
		Value = RX_PROMISCUOUS_MODE;
	}else
	{
		/*disable frame report & monitor mode*/
		Value &= ~RX_PROMISCUOUS_MODE;
		if(RxFilter.bFrameReport)
		{
			Value |= RM_FRAME_REPORT_EN;
		}else
		{
			Value &=~(RM_FRAME_REPORT_EN);
		}
		/*enable drop filter by RxfilterMask*/
		RxFilterCfg2Row(RxFilter.filterMask,&Value);
	}

	if(RxFilter.BandIdx)
	{
		MAC_IO_WRITE32(pAd, RMAC_RFCR_BAND_1, Value);
	}else
	{
		MAC_IO_WRITE32(pAd, RMAC_RFCR_BAND_0, Value);
	}

	return TRUE;
}


#ifdef DOT11_N_SUPPORT
INT MtAsicWtblSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable,UINT8 Wcid)
{
	struct wtbl_entry tb_entry;
	UINT32 dw;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));

#if defined(MT7615) || defined(MT7622)
	// TODO: shiang-MT7615, fix me!
	return 0;
#endif /* defined(MT7615) || defined(MT7622) */

	/* Search for the Ralink STA */
	for (Wcid = 1; VALID_UCAST_ENTRY_WCID(pAd, Wcid); Wcid++)
	{

		if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
						__FUNCTION__, Wcid));
			return FALSE;
		}

		HW_IO_READ32(pAd, tb_entry.wtbl_addr + (2 * 4), &dw);
		if (bEnable)
		{
			DMAC_WTBL_DW3_SET_R(dw, 1);
			DMAC_WTBL_DW3_SET_RDGBA(dw, 1);
		}
		else
		{
			DMAC_WTBL_DW3_SET_R(dw, 0);
			DMAC_WTBL_DW3_SET_RDGBA(dw, 0);
		}
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + (2 * 4), dw);
	}

	return TRUE;
}


INT MtAsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR BandIdx)
//INT MtAsicSetRDG(RTMP_ADAPTER *pAd, MT_RDG_CTRL_T *Rdg)
{
#if defined(MT7615) || defined(MT7622)
	return TRUE;
#else
	UINT32 tmac_tcr, agg_pcr, tmac_trcr;
	UINT32 RegTCR = 0,RegTRCR;

#ifdef DBDC_MODE
	if(BandIdx == 1)
	{
		RegTCR = TMAC_TCR1;
		RegTRCR = TMAC_TRCR1;
	}
	else
#endif /* DBDC_MODE */
	{
		RegTCR = TMAC_TCR;
		RegTRCR = TMAC_TRCR0;
	}

	MAC_IO_READ32(pAd, RegTCR, &tmac_tcr);
	MAC_IO_READ32(pAd, AGG_PCR, &agg_pcr);
	MAC_IO_READ32(pAd, RegTRCR, &tmac_trcr);

	if (bEnable)
	{
		/* enable RDG Rsp. also w/ Ralink Mode is necessary */
		tmac_tcr |= (RDG_RA_MODE | RDG_RESP_EN);
		/* LongNAV protect */
		agg_pcr &= ~(PROTECTION_MODE);
			 tmac_trcr = tmac_trcr & ~I2T_CHK_EN;
	}
	else
	{
		/* disable RDG Rsp. also w/ Ralink Mode is necessary */
		tmac_tcr &= ~(RDG_RA_MODE | RDG_RESP_EN);
		/* single frame protect */
		agg_pcr |= PROTECTION_MODE;
			 tmac_trcr = tmac_trcr | I2T_CHK_EN;
	}

	MAC_IO_WRITE32(pAd, RegTCR, tmac_tcr);
	MAC_IO_WRITE32(pAd, AGG_PCR, agg_pcr);
	MAC_IO_WRITE32(pAd, RegTRCR, tmac_trcr);

	return TRUE;
#endif /* defined(MT7615) || defined(MT7622) */
}
#endif /* DOT11_N_SUPPORT */


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
		pAd 		- Adapter pointer
		bPiggyBack	- Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID MtAsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}


#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2
static INT SetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
//	UINT32 mask, time_mask;
//	UINT32 Value;

return 0;
}


INT MtAsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN bEnable, UCHAR HwBssidIdx)
{
	UINT32 timeout = 0, bitmask = 0;

	ASSERT(HwBssidIdx <= 3);
	bitmask = 0xff << (HwBssidIdx * 8);

	if (bEnable == TRUE)
	{
		/*
		   each HW BSSID has its own PreTBTT interval,
		   unit is 64us, 0x00~0xff is configurable.
		   Base on RTMP chip experience,
		   Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		*/

		MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		timeout |= (0x50 << (HwBssidIdx * 8));
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): HwBssidIdx=%d, PreTBTT timeout = 0x%x\n",
					__FUNCTION__, HwBssidIdx, timeout));
	}
	else
	{
        MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): HwBssidIdx=%d, PreTBTT timeout = 0x%x\n",
					__FUNCTION__, HwBssidIdx, timeout));
    }

	return TRUE;
}


INT MtAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return SetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


INT MtAsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));

	return TRUE;
}

INT MtAsicGetTsfTimeByDriver(
        RTMP_ADAPTER *pAd,
        UINT32 *high_part,
        UINT32 *low_part,
        UCHAR HwBssidIdx)
{
    UINT32 Value = 0;

    MAC_IO_READ32(pAd, LPON_T0CR, &Value);
    Value = (Value & TSF_TIMER_HW_MODE_MASK) | TSF_TIMER_VALUE_READ;//keep HW mode value.
    MAC_IO_WRITE32(pAd, LPON_T0CR, Value);

    MAC_IO_READ32(pAd, LPON_UTTR0, low_part);
    MAC_IO_READ32(pAd, LPON_UTTR1, high_part);

    return TRUE;
}

typedef struct _SYNC_MODE_CR_TABLE_T
{
    UINT32              u4ArbOpModeCR;
    UINT32              u4ArbBcnWmmCR;
    UINT32              u4LponMacTimerCr;
    UINT32              u4LponTbttCtrlCR;
    UINT32              u4LponPreTbttTime;/* set pretbtt time */
    UINT32              u4LponSyncModeCR;/* sync mode CR*/
    UINT32              u4IntEnableCR;
} SYNC_MODE_CR_TABLE_T, *PSYNC_MODE_CR_TABLE_T;

static SYNC_MODE_CR_TABLE_T g_arDisableSyncModeMapTable[HW_BSSID_MAX] =
{   /*WMM cr set band0 first, change it when is checked it link to Band1. */
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR1, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR1, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR3, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR3, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicDisableSyncByDriver(RTMP_ADAPTER *pAd, UCHAR HWBssidIdx)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
	UINT32 value=0;

    if (HWBssidIdx >= HW_BSSID_MAX)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("--->%s(): HW OmacIdx:%x passed in is not correct\n",
                __func__, HWBssidIdx));
        return;
    }

    cr_set = g_arDisableSyncModeMapTable[HWBssidIdx];

    /*1. disable hif interrupt pin*/
    if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
    {
        MAC_IO_READ32(pAd, cr_set.u4IntEnableCR, &value);
        switch (HWBssidIdx)
        {
            case HW_BSSID_0:
                value &= ~HWIER3_TBTT0;
                value &= ~HWIER3_PRETBTT0;
                break;
            case HW_BSSID_1:
                value &= ~HWIER0_TBTT1;
                value &= ~HWIER0_PRETBTT1;
                break;
            case HW_BSSID_2:
                value &= ~HWIER0_TBTT2;
                value &= ~HWIER0_PRETBTT2;
                break;
            case HW_BSSID_3:
                value &= ~HWIER0_TBTT3;
                value &= ~HWIER0_PRETBTT3;
                break;
            default:
                ASSERT(HWBssidIdx < HW_BSSID_MAX);
                break;
        }
        MAC_IO_WRITE32(pAd, cr_set.u4IntEnableCR, value);
    }

    /*2. disable BeaconPeriodEn */
    MAC_IO_READ32(pAd, cr_set.u4LponTbttCtrlCR, &value);
    value &= ~TBTTn_CAL_EN;
    MAC_IO_WRITE32(pAd, cr_set.u4LponTbttCtrlCR, value);

    /*3. disable MPTCR pin*/
    /*NOTE: disable is write another CR at the same bit to disable. */
    value = 0;
    value |= (TBTT_TIMEUP_EN |
            TBTT_PERIOD_TIMER_EN |
            PRETBTT_TIMEUP_EN |
            PRETBTT_INT_EN);

    if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
        value = (value << 8);
    MAC_IO_WRITE32(pAd, cr_set.u4LponMacTimerCr, value);

    /*4. recover BCN AIFS, CWmin, and HW TSF sync mode.*/
    MAC_IO_READ32(pAd, cr_set.u4LponSyncModeCR, &value);
    value = value & TSF_TIMER_HW_MODE_FULL;
    MAC_IO_WRITE32(pAd, cr_set.u4LponSyncModeCR, value);


    value = 0;
    value |= ARB_WMMBCN_AIFS_DEFAULT_VALUE;
    value |= ARB_WMMBCN_CWMIN_DEFAULT_VALUE;
    MAC_IO_WRITE32(pAd, cr_set.u4ArbBcnWmmCR, value);

    /*5. set ARB OPMODE */
    MAC_IO_READ32(pAd, cr_set.u4ArbOpModeCR, &value);
    value &= ~(MT_ARB_SCR_OPMODE_MASK << (HWBssidIdx * 2));//clean opmode
    MAC_IO_WRITE32(pAd, cr_set.u4ArbOpModeCR, value);
}

static SYNC_MODE_CR_TABLE_T g_arEnableSyncModeMapTable[HW_BSSID_MAX] =
{   /*WMM cr set band0 first, change it when is checked it link to Band1. */
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR0, LPON_T0TPCR, LPON_PISR, LPON_T0CR, HWIER3},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR0, LPON_T1TPCR, LPON_PISR, LPON_T1CR, HWIER0},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR2, LPON_T2TPCR, LPON_PISR, LPON_T2CR, HWIER0},
    {ARB_SCR, ARB_WMMBCN0, LPON_MPTCR2, LPON_T3TPCR, LPON_PISR, LPON_T3CR, HWIER0},
};

VOID MtAsicEnableBssSyncByDriver(
        RTMP_ADAPTER *pAd,
        USHORT BeaconPeriod,
        UCHAR HWBssidIdx,
        UCHAR OPMode)
{
	SYNC_MODE_CR_TABLE_T cr_set = {0};
	UINT32 value=0;

    if (HWBssidIdx >= HW_BSSID_MAX)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("--->%s(): HW OmacIdx:%x passed in is not correct\n",
                __func__, HWBssidIdx));
        return;
    }

    cr_set = g_arEnableSyncModeMapTable[HWBssidIdx];

    /*1. set ARB OPMODE */
    MAC_IO_READ32(pAd, cr_set.u4ArbOpModeCR, &value);
    if(OPMode == OPMODE_AP)
    {
        value |=  (MT_ARB_SCR_BM_CTRL |
                    MT_ARB_SCR_BCN_CTRL |
                    MT_ARB_SCR_BCN_EMPTY);
    }
    value |= (OPMode << (HWBssidIdx * 2));
    MAC_IO_WRITE32(pAd, cr_set.u4ArbOpModeCR, value);

    if (OPMode == OPMODE_AP)
    {
        MAC_IO_READ32(pAd, cr_set.u4LponSyncModeCR, &value);
        value = value | TSF_TIMER_HW_MODE_TICK_ONLY;
        MAC_IO_WRITE32(pAd, cr_set.u4LponSyncModeCR, value);
    }

    /*3. set Pretbtt time. */
    MAC_IO_READ32(pAd, cr_set.u4LponPreTbttTime, &value);
    value |= DEFAULT_PRETBTT_INTERVAL_IN_MS << (HWBssidIdx * 8);
    MAC_IO_WRITE32(pAd, cr_set.u4LponPreTbttTime, value);

    /*4. set MPTCR */
    value = 0;
    value |= (TBTT_TIMEUP_EN |
            TBTT_PERIOD_TIMER_EN |
            PRETBTT_TIMEUP_EN |
            PRETBTT_INT_EN);

    if ((HWBssidIdx == HW_BSSID_1) || (HWBssidIdx == HW_BSSID_3))
        value = (value << 8);

    MAC_IO_WRITE32(pAd, cr_set.u4LponMacTimerCr, value);

    /*5. set BeaconPeriod */
    value = 0;
	{
        value |= BEACONPERIODn(BeaconPeriod);
    }
    value |= TBTTn_CAL_EN;
    MAC_IO_WRITE32(pAd, cr_set.u4LponTbttCtrlCR, value);

#ifndef BCN_OFFLOAD_SUPPORT
    /*6. enable HOST interrupt pin.*/
    if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
    {
        MAC_IO_READ32(pAd, cr_set.u4IntEnableCR, &value);
        switch (HWBssidIdx)
        {
            case HW_BSSID_0:
                value |= HWIER3_TBTT0;
                value |= HWIER3_PRETBTT0;
                break;
            case HW_BSSID_1:
                value |= HWIER0_TBTT1;
                value |= HWIER0_PRETBTT1;
                break;
            case HW_BSSID_2:
                value |= HWIER0_TBTT2;
                value |= HWIER0_PRETBTT2;
                break;
            case HW_BSSID_3:
                value |= HWIER0_TBTT3;
                value |= HWIER0_PRETBTT3;
                break;
            default:
                ASSERT(HWBssidIdx < HW_BSSID_MAX);
                break;
        }
        MAC_IO_WRITE32(pAd, cr_set.u4IntEnableCR, value);
    }
#endif
}



typedef struct _RTMP_WMM_PAIR {
	UINT32 Address;
	UINT32 Mask;
	UINT32 Shift;
} RTMP_WMM_PAIR, *PRTMP_WMM_PAIR;


static RTMP_WMM_PAIR wmm_txop_mask[] = {
	{TMAC_ACTXOPLR1, 0x0000ffff, 0}, /* AC0 - BK */
	{TMAC_ACTXOPLR1, 0xffff0000, 16}, /* AC1 - BE */
	{TMAC_ACTXOPLR0, 0x0000ffff, 0}, /* AC2 - VI */
	{TMAC_ACTXOPLR0, 0xffff0000, 16}, /* AC3 - VO */
};


static RTMP_WMM_PAIR wmm_aifsn_mask[] = {
	{ARB_WMMAC00, 0x0000000f, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x0000000f, 4}, /* AC1 - BE */
	{ARB_WMMAC02, 0x0000000f, 8}, /* AC2  - VI */
	{ARB_WMMAC03, 0x0000000f, 12}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmin_mask[] = {
	{ARB_WMMAC00, 0x00001f00, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x00001f00, 8}, /* AC1 - BE */
	{ARB_WMMAC02, 0x00001f00, 16}, /* AC2  - VI */
	{ARB_WMMAC03, 0x00001f00, 24}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmax_mask[] = {
	{ARB_WMMAC00, 0x001f0000, 0}, /* AC0 - BK */
	{ARB_WMMAC01, 0x001f0000, 16}, /* AC1 - BE */
	{ARB_WMMAC02, 0x001f0000, 0}, /* AC2  - VI */
	{ARB_WMMAC03, 0x001f0000, 16}, /* AC3 - VO */
};


 UINT32 MtAsicGetWmmParam(RTMP_ADAPTER *pAd, UINT32 AcNum, UINT32 EdcaType)
 {
	 UINT32 addr = 0, cr_val, mask = 0, shift = 0;

	 if (AcNum <= WMM_PARAM_AC_3)
	 {
		 switch (EdcaType)
		 {
			 case WMM_PARAM_TXOP:
				 addr = wmm_txop_mask[AcNum].Address;
				 mask = wmm_txop_mask[AcNum].Mask;
				 shift = wmm_txop_mask[AcNum].Shift;
				 break;
			 case WMM_PARAM_AIFSN:
				 addr = wmm_aifsn_mask[AcNum].Address;
				 mask = wmm_aifsn_mask[AcNum].Mask;
				 shift = wmm_aifsn_mask[AcNum].Shift;
				 break;
			 case WMM_PARAM_CWMIN:
				 addr = wmm_cwmin_mask[AcNum].Address;
				 mask = wmm_cwmin_mask[AcNum].Mask;
				 shift = wmm_cwmin_mask[AcNum].Shift;
				 break;
			 case WMM_PARAM_CWMAX:
				 addr = wmm_cwmax_mask[AcNum].Address;
				 mask = wmm_cwmax_mask[AcNum].Mask;
				 shift = wmm_cwmax_mask[AcNum].Shift;
				 break;
			 default:
				 MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __FUNCTION__, __LINE__, EdcaType));
				 break;
		 }
	 }

	 if (addr && mask)
	 {
		 MAC_IO_READ32(pAd, addr, &cr_val);
		 cr_val = (cr_val & mask) >> shift;

		 return cr_val;
	 }

	 return 0xdeadbeef;
 }


INT MtAsicSetWmmParam(RTMP_ADAPTER *pAd,UCHAR idx, UINT32 AcNum, UINT32 EdcaType, UINT32 EdcaValue)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
    UCHAR index = 0;


    /* Could write any queue by FW */
    if ((AcNum < 4) && (idx < 4)) {
        index = (idx * 4) + AcNum;
    }
    else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                ("%s(): Non-WMM Queue, WmmIdx/QueIdx=%d/%d!\n",
                __FUNCTION__, idx, AcNum));
        index = AcNum;
    }

	os_zero_mem(&EdcaParam,sizeof(MT_EDCA_CTRL_T));
	EdcaParam.ucTotalNum = 1;
	EdcaParam.ucAction = EDCA_ACT_SET;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)index;

	switch (EdcaType) {
	case WMM_PARAM_TXOP:
	pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
	pAcParam->u2Txop= (UINT16)EdcaValue;
	break;
	case WMM_PARAM_AIFSN:
	pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
	pAcParam->ucAifs = (UINT8)EdcaValue;
	break;
	case WMM_PARAM_CWMIN:
	pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
	pAcParam->ucWinMin= (UINT8)EdcaValue;
	break;
	case WMM_PARAM_CWMAX:
	pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
	pAcParam->u2WinMax= (UINT16)EdcaValue;
	break;
	default:
        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __FUNCTION__, __LINE__, EdcaType));
	break;
	}
#if defined(COMPOS_TESTMODE_WIN)
	;//no function
#else
	MtCmdEdcaParameterSet(pAd,EdcaParam);
#endif
	return NDIS_STATUS_SUCCESS;
}



/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicSetEdcaParm(RTMP_ADAPTER *pAd, PEDCA_PARM pEdcaParm)
{
	MT_EDCA_CTRL_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac=0,index=0;

	os_zero_mem(&EdcaParam, sizeof(MT_EDCA_CTRL_T));

	if ((pEdcaParm != NULL) && (pEdcaParm->bValid != FALSE))
	{
		EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;

		for ( ac=0; ac < CMD_EDCA_AC_MAX;  ac++)
		{
			index =(pEdcaParm->WmmSet)*4+ac;
			pAcParam = &EdcaParam.rAcParam[ac];
			pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
			pAcParam->ucAcNum =  wmm_aci_2_hw_ac_queue[index];
			pAcParam->ucAifs = pEdcaParm->Aifsn[ac];
			pAcParam->ucWinMin= pEdcaParm->Cwmin[ac];
			pAcParam->u2WinMax= pEdcaParm->Cwmax[ac];
			pAcParam->u2Txop= pEdcaParm->Txop[ac];
		}
	}

#if defined(COMPOS_TESTMODE_WIN)
	;//no function
#else
	MtCmdEdcaParameterSet(pAd,EdcaParam);
#endif

}


INT MtAsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


UINT32 MtAsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
							__FUNCTION__, __LINE__));

	return 0;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime, UCHAR BandIdx)
{
	UINT32 ret;

	UINT32 RifsTime = RIFS_TIME;
	UINT32 EifsTime = EIFS_TIME;
	ret = MtCmdSlotTimeSet(pAd,(UINT8)SlotTime,(UINT8)SifsTime,(UINT8)RifsTime,(UINT16)EifsTime,BandIdx);
}


#define RX_PKT_MAX_LENGTH   0x400 /* WORD(4 Bytes) unit */
INT MtAsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	UINT32 val;

#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not finish Yet!\n",
				__FUNCTION__, __LINE__));

return 0;
#endif /* defined(MT7615) || defined(MT7622) */

	// Rx max packet length
	MAC_IO_READ32(pAd, DMA_DCR0, &val);
	val &= (~0xfffc);
	val |= (RX_PKT_MAX_LENGTH << 2);
	MAC_IO_WRITE32(pAd, DMA_DCR0, val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Set the Max RxPktLen=%d!\n",
				__FUNCTION__, __LINE__, RX_PKT_MAX_LENGTH));

	return TRUE;
}

/*
	========================================================================
	Description:
		Add Shared key information into ASIC.
		Update shared key, TxMic and RxMic to Asic Shared key table
		Update its cipherAlg to Asic Shared key Mode.

    Return:
	========================================================================
*/
VOID MtAsicAddSharedKeyEntry(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			BssIndex,
	IN UCHAR			KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	//Do nothing, replace by MtAsicAddRemoveKeyTabByFw
}


/*	IRQL = DISPATCH_LEVEL*/
VOID MtAsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	//Do nothing, replace by MtAsicAddRemoveKeyTabByFw
}


VOID MtAsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG		uIV,
	IN ULONG		uEIV)
{
	// TODO: shiang-7603
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
}


static UCHAR *phy_bw_str[] = {"20M", "40M", "80M", "160M", "10M", "5M", "80+80"};
char* get_bw_str(int bandwidth)
{
	if (bandwidth >= BW_20 && bandwidth <= BW_8080)
		return phy_bw_str[bandwidth];
	else
		return "N/A";
}


/**
 * Wtbl2RateTableUpdate
 *
 *
 *
 */
VOID MtAsicTxCapAndRateTableUpdate(RTMP_ADAPTER *pAd, UCHAR ucWcid, RA_PHY_CFG_T *prTxPhyCfg, UINT32* Rate, BOOL fgSpeEn)
{
	struct wtbl_entry tb_entry;
	union WTBL_DW3 wtbl_wd3;
	union WTBL_DW5 wtbl_wd5;
	UINT32 u4RegVal;
	UCHAR bw;

	if(WtblWaitIdle(pAd,100,50) != TRUE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): WaitIdle failed\n", __FUNCTION__));
		return;
	}

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, ucWcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, ucWcid));
		return;
	}

	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 12, &(wtbl_wd3.word));
	if (fgSpeEn)
	{
		wtbl_wd3.field.spe_idx = 24;
	}
	else
	{
		wtbl_wd3.field.spe_idx = 0;
	}
	MAC_IO_WRITE32(pAd, tb_entry.wtbl_addr + 12, wtbl_wd3.word);

	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 20, &(wtbl_wd5.word));

	if (prTxPhyCfg->ldpc & HT_LDPC)
	{
		wtbl_wd5.field.ldpc = 1;
	}
	else
	{
		wtbl_wd5.field.ldpc = 0;
    }

	if (prTxPhyCfg->ldpc & VHT_LDPC)
	{
		wtbl_wd5.field.ldpc_vht = 1;
	}
	else
	{
		wtbl_wd5.field.ldpc_vht = 0;
	}

    switch (prTxPhyCfg->BW)
    {
        case 4:
        case 3:
            bw = 3;
            break;
        case BW_80:
            bw = 2;
            break;
        case BW_40:
            bw = 1;
            break;
        case BW_20:
        //case BW_10:
        default:
            bw = 0;
            break;
    }
    wtbl_wd5.field.fcap = bw;
    wtbl_wd5.field.cbrn = 7; // change bw as (fcap/2) if rate_idx > 7, temporary code

    if (prTxPhyCfg->ShortGI)
    {
        wtbl_wd5.field.g2 = 1;
        wtbl_wd5.field.g4 = 1;
        wtbl_wd5.field.g8 = 1;
        wtbl_wd5.field.g16 = 1;
    }
    else
    {
        wtbl_wd5.field.g2 = 0;
        wtbl_wd5.field.g4 = 0;
        wtbl_wd5.field.g8 = 0;
        wtbl_wd5.field.g16 = 0;
    }
    wtbl_wd5.field.rate_idx = 0;
    wtbl_wd5.field.txpwr_offset = 0;

	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR0, wtbl_wd5.word);
	u4RegVal = (Rate[0] | (Rate[1] << 12) | (Rate[2] << 24));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR1, u4RegVal);
	u4RegVal = ((Rate[2] >> 8) | (Rate[3] << 4) | (Rate[4] << 16) | (Rate[5] << 28));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR2, u4RegVal);
	u4RegVal = ((Rate[5] >> 4) | (Rate[6] << 8) | (Rate[7] << 20));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR3, u4RegVal);

	// TODO: shiang-MT7615, shall we also clear TxCnt/RxCnt/AdmCnt here??
	u4RegVal = (ucWcid | (1 << 13) | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}


/**
 * Wtbl2TxRateCounterGet
 *
 *
 *
 */
static VOID WtblTxRateCounterGet(RTMP_ADAPTER *pAd, UCHAR ucWcid, DMAC_TX_CNT_INFO *tx_cnt_info)
{
	struct wtbl_entry tb_entry;
	UINT32 u4RegVal;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, ucWcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, ucWcid));
		return;
	}
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 56, &(tx_cnt_info->wtbl_wd14.word));
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 60, &(tx_cnt_info->wtbl_wd15.word));
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 64, &(tx_cnt_info->wtbl_wd16.word));
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 68, &(tx_cnt_info->wtbl_wd17.word));

	if(WtblWaitIdle(pAd,100,50)!=TRUE)
	{
		return;
	}

	u4RegVal = (ucWcid | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);

}


/*
 * Wtbl2RcpiGet
 *
 *
 *
 */
 
VOID MtAsicTxCntUpdate(RTMP_ADAPTER *pAd, UCHAR Wcid, MT_TX_COUNTER *pTxInfo)
{
	DMAC_TX_CNT_INFO tx_cnt_info;

	WtblTxRateCounterGet(pAd, (UINT8)Wcid, &tx_cnt_info);
	pTxInfo->TxCount = tx_cnt_info.wtbl_wd16.field.current_bw_tx_cnt ;
	pTxInfo->TxCount += tx_cnt_info.wtbl_wd17.field.other_bw_tx_cnt;
	pTxInfo->TxFailCount = tx_cnt_info.wtbl_wd16.field.current_bw_fail_cnt ;
	pTxInfo->TxFailCount += tx_cnt_info.wtbl_wd17.field.other_bw_fail_cnt;

	pTxInfo->Rate1TxCnt = (UINT16)tx_cnt_info.wtbl_wd14.field.rate_1_tx_cnt;
	pTxInfo->Rate1FailCnt = (UINT16)tx_cnt_info.wtbl_wd14.field.rate_1_fail_cnt;
	pTxInfo->Rate2OkCnt = (UINT16)tx_cnt_info.wtbl_wd15.field.rate_2_ok_cnt;
	pTxInfo->Rate3OkCnt = (UINT16)tx_cnt_info.wtbl_wd15.field.rate_3_ok_cnt;
}


VOID MtAsicRssiGet(RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR *RssiSet )
{
	struct wtbl_entry tb_entry;
	union WTBL_DW28 wtbl_wd28;
	//UINT32 u4RegVal;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, Wcid));
		return;
	}
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 112, &wtbl_wd28.word);     // WTBL DW28

	RssiSet[0] = (wtbl_wd28.field.resp_rcpi_0 - 220) / 2;
	RssiSet[1] = (wtbl_wd28.field.resp_rcpi_1 - 220) / 2;
	RssiSet[2] = (wtbl_wd28.field.resp_rcpi_2 - 220) / 2;

    printk("RssiSet[0] = %d\n", RssiSet[0]);
    printk("RssiSet[1] = %d\n", RssiSet[1]);
    printk("RssiSet[2] = %d\n", RssiSet[2]);

/*
	if(WtblWaitIdle(pAd,100,50)!=TRUE)
	{
		return;
	}

	u4RegVal = (Wcid | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
*/

    return;
}

VOID MtRssiGet(RTMP_ADAPTER *pAd, UCHAR Wcid, CHAR *RssiSet )
{
	struct wtbl_entry tb_entry;
	union WTBL_DW28 wtbl_wd28;
	UINT i;
	//UINT32 u4RegVal;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, Wcid));
		return;
	}
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 112, &wtbl_wd28.word);     // WTBL DW28

	RssiSet[0] = (wtbl_wd28.field.resp_rcpi_0 - 220) / 2;
	RssiSet[1] = (wtbl_wd28.field.resp_rcpi_1 - 220) / 2;
	RssiSet[2] = (wtbl_wd28.field.resp_rcpi_2 - 220) / 2;
	RssiSet[3] = (wtbl_wd28.field.resp_rcpi_3 - 220) / 2;

    //printk("RssiSet[0] = %d\n", RssiSet[0]);
    //printk("RssiSet[1] = %d\n", RssiSet[1]);
    //printk("RssiSet[2] = %d\n", RssiSet[2]);
	//printk("RssiSet[3] = %d\n", RssiSet[3]);

	/* santiy check rssi value */
	for (i = 0; i < 4; i++)
	{       
		if (RssiSet[i] > 0)         
			RssiSet[i] = -127;
	}
    
/*
	if(WtblWaitIdle(pAd,100,50)!=TRUE)
	{
		return;
	}

	u4RegVal = (Wcid | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
*/

    return;
}




VOID MtAsicRcpiReset(RTMP_ADAPTER *pAd, UCHAR ucWcid)
{
    UINT32 u4RegVal;

	if(WtblWaitIdle(pAd,100,50)!=TRUE)
	{
		return ;
    }

    u4RegVal = (ucWcid | (1 << 15));
    MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}


static UINT16 get_tid_ssn(UCHAR tid, UINT32 *content)
{
    UINT16 ssn = 0;

    switch (tid)
    {
        case TID0_SN:
            ssn = *content & BA_SN_MSK;
            break;

        case TID1_SN:
            ssn = (*content >> 12) & BA_SN_MSK;
            break;

        case TID2_SN:
            ssn = (((*(content + 1) & 0xf) << 8) | (*content >> 24)) & BA_SN_MSK;
            break;

        case TID3_SN:
            ssn = (*(content + 1) >> 4) & BA_SN_MSK;
            break;

        case TID4_SN:
            ssn = (*(content + 1) >> 16) & BA_SN_MSK;
            break;

        case TID5_SN:
            ssn = (((*(content + 2) & 0xff) << 4) | (*(content + 1) >> 28)) & BA_SN_MSK;
            break;

        case TID6_SN:
            ssn = (*(content + 2) >> 8) & BA_SN_MSK;
            break;

        case TID7_SN:
            ssn = (*(content + 2) >> 20) & BA_SN_MSK;
            break;

        default:
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                    ("%s: Unknown TID=%x\n",
                     __FUNCTION__, tid));
            break;
    }

    return ssn;
}

UINT16 MtAsicGetTidSnByDriver(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid)
{
	struct wtbl_entry tb_entry;
	UINT16 ssn = 0xffff; //invalid number
	UINT32 serial_no[3] = {0x0, 0x0, 0x0};

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                ("%s: Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, wcid));
		goto end;
	}

	HW_IO_READ32(pAd, tb_entry.wtbl_addr + (4 * 11), &serial_no[0]);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + (4 * 12), &serial_no[1]);
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + (4 * 13), &serial_no[2]);

	ssn = get_tid_ssn(tid, serial_no);

end:
    return ssn;
}


// TODO: shiang-MT7615, use MMPS_DYNAMIC instead for "smps"
VOID MtAsicSetSMPSByDriver(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Smps)
{
	struct wtbl_entry tb_entry;
	UINT32 dw;

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, Wcid));

		return ;
	}

	// WTBL.dw3 bit 23, support Dynamic SMPS
	HW_IO_READ32(pAd, tb_entry.wtbl_addr + 12, &dw);
	if (Smps)
		dw |= (0x1 << 23);
	else
		dw &= (~(0x1<< 23));
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + 12, dw);
}

VOID MtAsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Smps)
{
	MtAsicSetSMPSByDriver(pAd, Wcid, Smps);
	return;
}


BOOLEAN static IsMtAsicWtblIndirectAccessIdle(RTMP_ADAPTER *pAd)
{
	UINT32 wiucr_val, cnt;
	BOOLEAN is_idle = FALSE;

	wiucr_val = (1<<31);
	cnt = 0;
	do {
		HW_IO_READ32(pAd, WTBL_OFF_WIUCR, &wiucr_val);
		if ((wiucr_val & (1<<31)) == 0)
			break;
		else
			cnt++;
	}while(cnt < 100);

	if ((wiucr_val & (1<<31)) == 0)
	{
		is_idle = TRUE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): WIUCR is idle!wiucr_val=0x%x\n",
					__FUNCTION__, wiucr_val));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): WIUCR is busy!wiucr_val=0x%x\n",
					__FUNCTION__, wiucr_val));
	}

	return is_idle;
}


/*
	WTBL Access for DW0~DW1, Perr unique key
*/
INT static MtAsicWtblWriteRxInfo(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 *val)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE)
	{
		HW_IO_WRITE32(pAd, WTBL_ON_RICR0, val[0]);	// DW 0
		HW_IO_WRITE32(pAd, WTBL_ON_RICR1, val[1]);	// DW 1
		wiucr_val = (WTBL_RXINFO_UPDATE |WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	}
	else
		return FALSE;
};


INT static MtAsicWtblWriteRateInfo(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 *val)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE)
	{
		HW_IO_WRITE32(pAd, WTBL_ON_RIUCR0, val[0]); // DW 5
		HW_IO_WRITE32(pAd, WTBL_ON_RIUCR1, val[1]); // DW 6
		HW_IO_WRITE32(pAd, WTBL_ON_RIUCR2, val[2]); // DW 7
		HW_IO_WRITE32(pAd, WTBL_ON_RIUCR3, val[3]); // DW 8
		wiucr_val = (WTBL_RATE_UPDATE |WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	}
	else
		return FALSE;
};


static INT MtAsicWtblWritePsm(RTMP_ADAPTER *pAd, UINT8 wcid, UINT32 addr, UINT8 psm)
{
	UINT32 val;

	MAC_IO_WRITE32(pAd, WTBL_ON_WTBLOR, WTBL_OR_PSM_W_FLAG);
	HW_IO_READ32(pAd, addr, &val);
	val &= (~((1 << 30)));
	val |= ((psm << 30));
	HW_IO_WRITE32(pAd, addr, val);
	MAC_IO_WRITE32(pAd, WTBL_ON_WTBLOR, 0);

	return TRUE;
};


static INT MtAsicWtblDwWrite(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 base, UINT8 dw, UINT32 val)
{
	switch (dw)
	{
		case 0:
		case 1:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s(): Err!Please call MtAsicWtblWriteRxInfo() for this!\n", __FUNCTION__));
			break;
		case 2:
		case 3:
		case 4:
			HW_IO_WRITE32(pAd, base + dw * 4, val);
			break;
		case 5:
		case 6:
		case 7:
		case 8:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s(): Err!Please call MtAsicWtblWriteRateInfo() for this!\n", __FUNCTION__));
			break;
		default:
			HW_IO_WRITE32(pAd, base + dw * 4, val);
			break;
	}

	return TRUE;
}


/*
	WTBL Access for Rx Counter clear
*/
INT static MtAsicWtblRxCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE)
	{
		wiucr_val = (WTBL_RX_CNT_CLEAR |WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	}
	else
		return FALSE;
};


INT static MtAsicWtblTxCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE)
	{
		wiucr_val = (WTBL_TX_CNT_CLEAR |WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	}
	else
		return FALSE;
};


INT static MtAsicWtblAdmCounterClear(RTMP_ADAPTER *pAd, UINT8 wlan_idx)
{
	UINT32 wiucr_val;

	if (IsMtAsicWtblIndirectAccessIdle(pAd) == TRUE)
	{
		wiucr_val = (WTBL_ADM_CNT_CLEAR |WLAN_IDX(wlan_idx));
		HW_IO_WRITE32(pAd, WTBL_OFF_WIUCR, wiucr_val);
		return TRUE;
	}
	else
		return FALSE;
};


INT static MtAsicWtblFieldsReset(RTMP_ADAPTER *pAd, UINT8 wlan_idx, UINT32 base)
{
	UINT32 Index;

	/* Clear BA Information */
	MtAsicWtblDwWrite(pAd, wlan_idx, base, 4, 0);

	/* RX Counter Clear */
	MtAsicWtblRxCounterClear(pAd, wlan_idx);

	/* TX Counter Clear */
	MtAsicWtblTxCounterClear(pAd, wlan_idx);

	/* Clear Cipher Key */
	for (Index = 0; Index < 16; Index++)
		HW_IO_WRITE32(pAd, base + (4 * (Index + 30)), 0x0);

	/* Admission Control Counter Clear */
	MtAsicWtblAdmCounterClear(pAd, wlan_idx);

	return TRUE;
}


#define SN_MASK 0xfff

#ifndef  MAC_INIT_OFFLOAD
static UCHAR ba_mib_range[MT_DMAC_BA_STAT_RANGE]={0, 4, 14, 21, 32, 48, 56, 64};
#endif
static UCHAR ba_range[8/*MT_DMAC_BA_AGG_RANGE*/] = {4, 8, 12, 24, 36, 48, 54, 64};

INT32 MtAsicSetDevMacByDriver(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)
{
	UINT32 val;
	UINT32 own_mac_reg_base = RMAC_OMA0R0;

	// We only need to consider BandIdx in FW offload case
	if (Active)
	{
		val = (OwnMacAddr[0]) | (OwnMacAddr[1]<<8) |  (OwnMacAddr[2]<<16) | (OwnMacAddr[3]<<24);
		MAC_IO_WRITE32(pAd, own_mac_reg_base + (OwnMacIdx * 8), val);

		val = OwnMacAddr[4] | (OwnMacAddr[5]<<8) | (1 <<16);
		MAC_IO_WRITE32(pAd, (own_mac_reg_base + 4) + (OwnMacIdx * 8), val);
	}
	return 0;
}

INT32 MtAsicSetBssidByDriver(
	RTMP_ADAPTER *pAd,
    BSS_INFO_ARGUMENT_T bss_info_argument)
{
    UINT32 val;
    ULONG curr_bssid_reg_base = RMAC_CB0R0;
#ifdef CONFIG_AP_SUPPORT
    UINT8 BssIdx = 0;
#endif
    UCHAR OwnMacIdx = bss_info_argument.OwnMacIdx;
    UINT8 Active = FALSE;
    UCHAR *Bssid = bss_info_argument.Bssid;

	if (bss_info_argument.bss_state >= BSS_ACTIVE)
		Active = TRUE;

    if (OwnMacIdx < HW_BSSID_MAX)
    {
        if (Active)
        {
            val = (UINT32)((Bssid[0]) |
                (UINT32)(Bssid[1] << 8) |
                (UINT32)(Bssid[2] << 16) |
                (UINT32)(Bssid[3] << 24));
            MAC_IO_WRITE32(pAd, curr_bssid_reg_base + (OwnMacIdx * 8), val);

            val = (UINT32)(Bssid[4]) | (UINT32)(Bssid[5] << 8) | (1 <<16);
            MAC_IO_WRITE32(pAd, (curr_bssid_reg_base + 4) + (OwnMacIdx * 8), val);
        }
    }
#ifdef CONFIG_AP_SUPPORT
    else
    {
        BssIdx = (OwnMacIdx & ~pAd->chipCap.ExtMbssOmacStartIdx);
        MtDmacSetExtMbssEnableCR(pAd, BssIdx, Active);//enable rmac 0_1~0_15 bit
        MtDmacSetMbssHwCRSetting(pAd, BssIdx, Active);//enable lp timing setting for 0_1~0_15
    }
#endif
	return NDIS_STATUS_SUCCESS;
}

INT32 MtAsicSetStaRecByDriver(
	RTMP_ADAPTER *pAd,
	STA_REC_CFG_T StaRecCfg)
{
	/* Not supported this fucntion in driver */
	return 0;
}

INT32 MtAsicUpdateBASessionByDriver(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	INT32 Status = 0;
	struct wtbl_entry ent;
	UINT32 range_mask = 0x7 << (BaCtrl.Tid * 3);
	UINT32 reg, reg_mask, reg_bit, reg2, reg2_bit, value;


	if (BaCtrl.BaSessionType == BA_SESSION_RECP)
	{
		/* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (BaCtrl.isAdd)
		{
			UINT32 band_offset, band = BaCtrl.band_idx;

			// TODO: shiang-MT7615, check band!!
			if (band == 0)
				band_offset = 0;
			else
				band_offset = 0x100;
			value = (BaCtrl.PeerAddr[0] | (BaCtrl.PeerAddr[1] << 8) |
					(BaCtrl.PeerAddr[2] << 16) | (BaCtrl.PeerAddr[3] << 24));
			MAC_IO_WRITE32(pAd, (BSCR0_BAND_0 + band_offset), value);

			MAC_IO_READ32(pAd, (BSCR1_BAND_0 + band_offset), &value);
			value &= ~(BA_MAC_ADDR_47_32_MASK | RST_BA_TID_MASK | RST_BA_SEL_MASK);
			value |= BA_MAC_ADDR_47_32((BaCtrl.PeerAddr[4] | (BaCtrl.PeerAddr[5] << 8)));
			value |= (RST_BA_SEL(RST_BA_MAC_TID_MATCH) | RST_BA_TID(BaCtrl.Tid) | START_RST_BA_SB);
			MAC_IO_WRITE32(pAd, (BSCR1_BAND_0 + band_offset), value);
		}
	}
	else
	{
		NdisZeroMemory((UCHAR *)(&ent), sizeof(struct wtbl_entry));
		mt_wtbl_get_entry234(pAd, BaCtrl.Wcid, &ent);

		if (BaCtrl.isAdd)
		{
			INT idx = 0;

			// TODO: shiang-MT7615, shall we stop tx for this?
			MtAsicSetMacTxRx(pAd, ASIC_MAC_TX, FALSE,BaCtrl.band_idx);

			reg = reg2 = reg_mask = reg_bit = 0;

			/* Clear WTBL2. SN: Direct Updating */
			switch (BaCtrl.Tid)
			{
				case 0:
					reg = ent.wtbl_addr + (4 * 11); //DW11
					reg_bit = 0;
					break;
				case 1:
					reg = ent.wtbl_addr + (4 * 11); //DW11
					reg_bit = 12;
					break;
				case 2:
					reg = ent.wtbl_addr + (4 * 11); //DW11
					reg_bit = 24;

					reg2 = ent.wtbl_addr + (4 * 12); //DW12
					reg2_bit = 0;
					break;
				case 3:
        				reg = ent.wtbl_addr + (4 * 12); //DW12
        				reg_bit = 4;
					break;
				case 4:
					reg = ent.wtbl_addr + (4 * 12); //DW12
					reg_bit = 16;
					break;
				case 5:
					reg = ent.wtbl_addr + (4 * 12); //DW12
					reg_bit = 28;

					reg2 = ent.wtbl_addr + (4 * 13); //DW13
					reg2_bit = 0;
					break;
				case 6:
					reg = ent.wtbl_addr + (4 * 13); //DW13
					reg_bit = 8;
					break;
				case 7:
					reg = ent.wtbl_addr + (4 * 13); //DW13
					reg_bit = 20;
					break;
				default:
					MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
								("%s: unknown tid(%d) \n", __FUNCTION__, BaCtrl.Tid));
					goto skip_update_sn;
			}

			if (32 - reg_bit >= 12) {
				HW_IO_READ32(pAd, reg, &value);
				value &= (~(SN_MASK << reg_bit));
				value |= ((BaCtrl.Sn & SN_MASK) << reg_bit);
				HW_IO_WRITE32(pAd, reg, value);
			} else {
				ASSERT(reg2 != 0);

				// low bits
				HW_IO_READ32(pAd, reg, &value);
				value &= (~(SN_MASK << reg_bit));
				value |= ((BaCtrl.Sn & SN_MASK) << reg_bit);
				HW_IO_WRITE32(pAd, reg, value);

				// high bits
				HW_IO_READ32(pAd, reg2, &value);
				reg_mask = ((SN_MASK >> (32 - reg_bit)) << reg2_bit);
				value &= (~reg_mask);
				value |= (((BaCtrl.Sn & SN_MASK) >> (32 - reg_bit)) << reg2_bit);
				HW_IO_WRITE32(pAd, reg2, value);
			}

skip_update_sn:
			MtAsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE,BaCtrl.band_idx);

			while (BaCtrl.BaWinSize > ba_range[idx]) {
				if (idx == (MT_DMAC_BA_AGG_RANGE - 1))
					break;
				idx++;
			};
			if ((idx > 0) && (ba_range[idx] > BaCtrl.BaWinSize))
				idx--;

			reg = ent.wtbl_addr + (4 * 4);
			HW_IO_READ32(pAd, reg, &value);
			value &= (~range_mask);
			value |= ((1 << (BaCtrl.Tid + 24)) | (idx << (BaCtrl.Tid * 3)));
			HW_IO_WRITE32(pAd, reg, value);
		}
		else
		{
			reg = ent.wtbl_addr + (4 * 4);
			HW_IO_READ32(pAd, reg, &value);
			value &= (~(1 << (BaCtrl.Tid + 24)));
			value &= (~range_mask);
			HW_IO_WRITE32(pAd, reg, value);
		}
	}

	return Status;
}

INT32 MtAsicUpdateBASession(RTMP_ADAPTER *pAd, MT_BA_CTRL_T BaCtrl)
{
	return MtAsicUpdateBASessionByDriver(pAd, BaCtrl);
}

VOID MtAsicUpdateRxWCIDTableByDriver(RTMP_ADAPTER *pAd,MT_WCID_TABLE_INFO_T WtblInfo)
{
	struct wtbl_entry tb_entry;
	//UINT32 Value, Index;
	UINT32 dw[5] = {0};
	UINT32 rate_info[4] = {0};

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, (UCHAR)WtblInfo.Wcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, WtblInfo.Wcid));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
				("%s(): tb_entry->wtbl_idx/wtbl_addr=%d/0x%x\n",
				__FUNCTION__, tb_entry.wtbl_idx, tb_entry.wtbl_addr));

	if(WtblInfo.WcidType == MT_WCID_TYPE_BMCAST)
	{
		WTBL_SET_MUARIDX(dw[0], 0xe);
		WTBL_SET_CIPHER_SUIT(dw[2], WTBL_CIPHER_NONE);
#ifdef HDR_TRANS_TX_SUPPORT
		if (pAd->OpMode == OPMODE_AP)
		{
			WTBL_SET_FD(rate_info[0], 1);
			WTBL_SET_TD(rate_info[0], 0);
		}
#endif /* HDR_TRANS_TX_SUPPORT */
	}
	else if(WtblInfo.WcidType == MT_WCID_TYPE_APCLI_MCAST)
	{
		WTBL_SET_MUARIDX(dw[0], 0xe);
		WTBL_SET_RV(dw[0], 1);
		WTBL_SET_CIPHER_SUIT(dw[2], WTBL_CIPHER_NONE);
		WTBL_SET_RC_A2(dw[0], 1);
	}
	else
	{
		WTBL_SET_MUARIDX(dw[0], WtblInfo.MacAddrIdx);
		if((WtblInfo.WcidType == MT_WCID_TYPE_APCLI) ||
            (WtblInfo.WcidType == MT_WCID_TYPE_REPEATER))
		{
			WTBL_SET_RC_A1(dw[0], 1);
		}

		WTBL_SET_RV(dw[0], 1);
		WTBL_SET_RC_A2(dw[0], 1);
		// TODO: shiang-MT7603, in which case we need to check A1???
		//WTBL_SET_RC_A1(dw[0], 1);
		WTBL_SET_CIPHER_SUIT(dw[2], WtblInfo.CipherSuit);
		WTBL_SET_RKV(dw[0], ((WtblInfo.CipherSuit != WTBL_CIPHER_NONE) ? 1:0));

		WTBL_SET_QOS(rate_info[0], WtblInfo.SupportQoS);
#ifdef HDR_TRANS_TX_SUPPORT
		switch (WtblInfo.WcidType)
		{
			case MT_WCID_TYPE_AP:
				WTBL_SET_FD(rate_info[0], 0);
				WTBL_SET_TD(rate_info[0], 1);
				break;
			case MT_WCID_TYPE_CLI:
				WTBL_SET_FD(rate_info[0], 1);
				WTBL_SET_TD(rate_info[0], 0);
				break;
			case MT_WCID_TYPE_APCLI:
            case MT_WCID_TYPE_REPEATER:
				WTBL_SET_FD(rate_info[0], 0);
				WTBL_SET_TD(rate_info[0], 1);
				break;
			case MT_WCID_TYPE_WDS:
				WTBL_SET_FD(rate_info[0], 1);
				WTBL_SET_TD(rate_info[0], 1);
				break;
			default:
			 	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
							("%s: Unknown entry type(%d) do not support header translation\n",
							__FUNCTION__, WtblInfo.WcidType));
				break;
		}
#endif /* HDR_TRANS_TX_SUPPORT */

#ifdef HDR_TRANS_RX_SUPPORT
		if ((pAd->chipCap.asic_caps | fASIC_CAP_RX_HDR_TRANS) == 0)
			WTBL_SET_DIS_RHTR(dw[2], 1);
		else
			WTBL_SET_DIS_RHTR(dw[2], 0);
#endif /* HDR_TRANS_RX_SUPPORT */

#ifdef DOT11_N_SUPPORT
		if(WtblInfo.SupportHT)
		{
			WTBL_SET_HT(dw[2], 1);
			WTBL_SET_MM(rate_info[0], WtblInfo.MpduDensity);
			WTBL_SET_QOS(rate_info[0], 1);
			DMAC_WTBL_DW3_SET_AF(dw[3], WtblInfo.MaxRAmpduFactor);
			DMAC_WTBL_DW3_SET_BAF(dw[3], 1);
			if(WtblInfo.SupportRDG)
			{
				DMAC_WTBL_DW3_SET_R(dw[3], 1);
				DMAC_WTBL_DW3_SET_RDGBA(dw[3], 1);
			}

			if (WtblInfo.SmpsMode == MMPS_DYNAMIC)
				DMAC_WTBL_DW3_SET_SMPS(dw[3], 1);
			else
				DMAC_WTBL_DW3_SET_SMPS(dw[3], 0);

#ifdef DOT11_VHT_AC
			if (WtblInfo.SupportVHT)
			{
				WTBL_SET_VHT(dw[2], 1);
			}
#endif /* DOT11_VHT_AC */
		}
#endif /* DOT11_N_SUPPORT */

#ifdef TXBF_SUPPORT
        WTBL_SET_PFMU_IDX(dw[2], (UCHAR)WtblInfo.PfmuId);
#endif
		WTBL_SET_PARTIAL_AID(dw[2], (UCHAR)WtblInfo.Aid);
	}

	WTBL_SET_PEER_ADDR4(dw[0], WtblInfo.Addr[4]);
	WTBL_SET_PEER_ADDR5(dw[0], WtblInfo.Addr[5]);
	WTBL_SET_PEER_ADDR0_3(dw[1], WtblInfo.Addr[0] + (WtblInfo.Addr[1] << 8) +(WtblInfo.Addr[2] << 16) +(WtblInfo.Addr[3] << 24));

printk("%s(%d):Before Write to WTBL, dump raw data:0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x\n",
		__FUNCTION__, WtblInfo.Wcid, dw[0], dw[1], dw[2], dw[3], dw[4], rate_info[0]);

	MtAsicWtblWritePsm(pAd, WtblInfo.Wcid, tb_entry.wtbl_addr+12, 0); // set psm as 0

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
					("%s(%d):Write WTBL Addr:0x%x\n",
					__FUNCTION__, WtblInfo.Wcid, tb_entry.wtbl_addr));

	// DW 0~1
	MtAsicWtblWriteRxInfo(pAd, WtblInfo.Wcid, &dw[0]); // dw 0~1
	MtAsicWtblDwWrite(pAd, WtblInfo.Wcid, tb_entry.wtbl_addr, 2, dw[2]);
	MtAsicWtblDwWrite(pAd, WtblInfo.Wcid, tb_entry.wtbl_addr, 3, dw[3]);

	// DW 5~8
	MtAsicWtblWriteRateInfo(pAd, WtblInfo.Wcid, &rate_info[0]);

	MtAsicWtblFieldsReset(pAd, WtblInfo.Wcid, tb_entry.wtbl_addr);

	if (1) {
		// dump WTBL when in-direct access
		INT i;
		UINT32 wtbl_addr, wtbl_val[29] = {0};

		for (i = 0; i < 29; i++)
		{
			wtbl_addr = WTBL_BASE_ADDR + WtblInfo.Wcid * WTBL_PER_ENTRY_SIZE + (i * 4);
			HW_IO_READ32(pAd, wtbl_addr, &wtbl_val[i]);
		}
		hex_dump("After write, dump Raw WTBL Entry", (UCHAR *)wtbl_val, 29 *4);
	}
}

VOID MtAsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, MT_WCID_TABLE_INFO_T WtblInfo)
{
	MtAsicUpdateRxWCIDTableByDriver(pAd, WtblInfo);

	return;
}

/*
   ========================================================================
Description:
Add Client security information into ASIC WCID table and IVEIV table.
Return:

Note :
The key table selection rule :
1.	Wds-links and Mesh-links always use Pair-wise key table.
2.	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled,
it needs to set key into Pair-wise Key Table.
3.	The pair-wise key security mode is set NONE, it means as no security.
4.	In STA Adhoc mode, it always use shared key table.
5.	Otherwise, use shared key table

========================================================================
 */
VOID MtAsicUpdateWcidAttributeEntry(
        IN	PRTMP_ADAPTER	pAd,
        IN	UCHAR			BssIdx,
		IN	UCHAR			KeyIdx,
		IN	UCHAR			CipherAlg,
        IN	UINT8			Wcid,
        IN	UINT8			KeyTabFlag)
{
    //	WCID_ATTRIBUTE_STRUC WCIDAttri;
    //USHORT offset;
    //UINT32 wcid_attr_base = 0, wcid_attr_size = 0;

	// TODO: shiang-7603
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): , Wcid=%d, BssIdx=%d, KeyIdx=%d, Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__, Wcid, BssIdx, KeyIdx));
	return;
}


/*
   ==========================================================================
Description:

IRQL = DISPATCH_LEVEL

==========================================================================
 */
VOID MtAsicDelWcidTabByDriver(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
	UCHAR cnt, cnt_s, cnt_e;
	struct wtbl_entry tb_entry;
	UINT32 dw[5] = {0};

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():wcid_idx=0x%x\n",
					__FUNCTION__, wcid_idx));

	if (wcid_idx == WCID_ALL) {
		cnt_s = 0;
		cnt_e = (WCID_ALL - 1);
	} else {
		cnt_s = cnt_e = wcid_idx;
	}

	for (cnt = cnt_s; cnt_s <= cnt_e; cnt_s++)
	{
		cnt = cnt_s;
		NdisZeroMemory(&tb_entry, sizeof(tb_entry));
		if (mt_wtbl_get_entry234(pAd, cnt, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
						__FUNCTION__, cnt));
			return;
		}

		// TODO: shiang-MT7615, why we need to set rc_a2/rv as 1 even for delTab case??
		//dw0->field.rc_a2 = 1;
		//dw0->field.rv = 1;
#ifdef MT_PS
		//if (cnt == MCAST_WCID) {
		//	dw3->field.i_psm=1;
		//	dw3->field.du_i_psm=1;
		//}
#endif /*leonardo workaround*/

		MtAsicWtblWriteRxInfo(pAd, wcid_idx, &dw[0]); // DW0~1
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + 4 * 2, dw[2]); // DW 2
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + 4 * 3, dw[3]); // DW 3
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr + 4 * 4, dw[4]); // DW 4
		MtAsicWtblWritePsm(pAd, wcid_idx, tb_entry.wtbl_addr + 4 * 3, 0);

		MtAsicWtblFieldsReset(pAd, wcid_idx, tb_entry.wtbl_addr);
	}
}

VOID MtAsicAddRemoveKey(RTMP_ADAPTER *pAd,MT_SECURITY_CTRL SecurityCtrl)
{
#ifdef CONFIG_WTBL_TLV_MODE
    MtAsicAddRemoveKeyByFw(pAd, SecurityCtrl);
#endif
}


/*
	========================================================================
	Description:
		Add Pair-wise key material into ASIC.
		Update pairwise key, TxMic and RxMic to Asic Pair-wise key table

    Return:
	========================================================================
*/
VOID MtAsicAddPairwiseKeyEntry(RTMP_ADAPTER *pAd, UCHAR WCID, CIPHER_KEY *pKey)
{
	// TODO: shiang-7603
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}


/*
	========================================================================
	Description:
		Remove Pair-wise key material from ASIC.

    Return:
	========================================================================
*/
VOID MtAsicRemovePairwiseKeyEntry(RTMP_ADAPTER *pAd, UCHAR Wcid)
{
	/* Set the specific WCID attribute entry as OPEN-NONE */
	MtAsicUpdateWcidAttributeEntry(pAd,
							  BSS0,
							  0,
							  CIPHER_NONE,
							  Wcid,
							  PAIRWISEKEYTABLE);
}


INT MtAsicSendCommandToMcu(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	// TODO: shiang-7603
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


BOOLEAN MtAsicSendCmdToMcuAndWait(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Command,
	IN UCHAR Token,
	IN UCHAR Arg0,
	IN UCHAR Arg1,
	IN BOOLEAN in_atomic)
{
	BOOLEAN cmd_done = TRUE;

	MtAsicSendCommandToMcu(pAd, Command, Token, Arg0, Arg1, in_atomic);

#ifndef COMPOS_WIN
    // TODO: shiang-usw, revise this!!!
#ifdef RTMP_MAC_PCI
	cmd_done = AsicCheckCommanOk(pAd, Token);
#endif /* RTMP_MAC_PCI */
#endif /* COMPOS_WIN */
	return cmd_done;
}


BOOLEAN MtAsicSendCommandToMcuBBP(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 Command,
	IN UCHAR		 Token,
	IN UCHAR		 Arg0,
	IN UCHAR		 Arg1,
	IN BOOLEAN		FlgIsNeedLocked)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


VOID MtAsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
}


#ifdef WAPI_SUPPORT
VOID MtAsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG		 pn_low,
	IN ULONG		 pn_high)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}
#endif /* WAPI_SUPPORT */





#ifdef STREAM_MODE_SUPPORT
UINT32 MtStreamModeRegVal(RTMP_ADAPTER *pAd)
{
	return 0x0;
}


/*
	========================================================================
	Description:
		configure the stream mode of specific MAC or all MAC and set to ASIC.

	Prameters:
		pAd 	 ---
		pMacAddr ---
		bClear	 --- disable the stream mode for specific macAddr when
						(pMacAddr!=NULL)

    Return:
	========================================================================
*/
VOID MtAsicSetStreamMode(
	IN RTMP_ADAPTER *pAd,
	IN PUCHAR pMacAddr,
	IN INT chainIdx,
	IN BOOLEAN bEnabled)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}


VOID MtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}
#endif // STREAM_MODE_SUPPORT //


VOID MtAsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	//AUTO_RSP_CFG_STRUC csr4;

	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
						__FUNCTION__, __LINE__));
	return;
}


#ifdef DOT11_N_SUPPORT
INT MtAsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	NdisZeroMemory(aggCnt, cnt_len * sizeof(ULONG));

	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


INT MtAsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


#endif // DOT11_N_SUPPORT //


INT MtAsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
	return TRUE;
}



static UINT32 reg_wmm_cr[4]={ARB_TQSW0, ARB_TQSW1, ARB_TQSW2, ARB_TQSW3};
static UINT32 reg_mgmt_cr[2] = {ARB_TQSM0, ARB_TQSM1};

INT32 MtAsicSetTxQ(RTMP_ADAPTER *pAd, INT WmmSet, INT BandIdx, BOOLEAN Enable)
{
	UINT32 reg_w = 0, mask_w = 0, val_w;
	UINT32 reg_mgmt = 0, mask_m = 0, val_m;

	if ((BandIdx < 2) && (WmmSet < 4) )
	{
		reg_mgmt = reg_mgmt_cr[BandIdx];
		mask_w = 0x0f0f0f0f;
		reg_w = reg_wmm_cr[WmmSet];
		mask_m = 0x0f000f0f;

		MAC_IO_READ32(pAd, reg_w, &val_w);
		val_w = ((Enable) ? (val_w | mask_w) : (val_w & (~mask_w)));
		MAC_IO_WRITE32(pAd, reg_w, val_w);

		MAC_IO_READ32(pAd, reg_mgmt, &val_m);
		val_m = ((Enable) ? (val_m | mask_m) : (val_m & (~mask_m)));
		MAC_IO_WRITE32(pAd, reg_mgmt, val_m);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): Set WmmSet=%d, band=%d, Enable=%d with CR[0x%x = 0x%08x, 0x%x=0x%08x]\n",
					__FUNCTION__, WmmSet, BandIdx, Enable, reg_w, val_w, reg_mgmt, val_m));

		return TRUE;
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
					("%s(): Invalid Input paramter!WmmSet=%d, BssIdx=%d, band=%d, Enable=%d\n",
					__FUNCTION__, WmmSet, BandIdx, BandIdx, Enable));
		return FALSE;
	}
}

INT32 MtAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable,UCHAR BandIdx)
{
	UINT32 Value, Value2;
	UINT32 i;

	MAC_IO_READ32(pAd, ARB_SCR, &Value);
	MAC_IO_READ32(pAd, ARB_RQCR, &Value2);

	switch (TxRx)
	{
		case ASIC_MAC_TX:
			if (Enable)
			{
				Value &= (BandIdx) ? (~MT_ARB_SCR_TX1DIS) : (~MT_ARB_SCR_TXDIS);
			}
			else
			{
				Value |= (BandIdx) ? MT_ARB_SCR_TX1DIS : MT_ARB_SCR_TXDIS;
			}
			break;
		case ASIC_MAC_RX:
			if (Enable)
			{
				Value &= (BandIdx) ?  (~MT_ARB_SCR_RX1DIS) : (~MT_ARB_SCR_RXDIS);
				Value2 |= (BandIdx) ? ARB_RQCR_RX1_START : ARB_RQCR_RX_START;
			}
			else
			{
				Value |= (BandIdx) ? MT_ARB_SCR_RX1DIS : MT_ARB_SCR_RXDIS;
				Value2 &= (BandIdx) ?  (~ARB_RQCR_RX1_START) : (~ARB_RQCR_RX_START) ;
			}
			break;
		case ASIC_MAC_TXRX:
			if (Enable)
			{
				Value &= (BandIdx) ? (~(MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS)) : (~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS));
				Value2 |= (BandIdx) ? ARB_RQCR_RX1_START : ARB_RQCR_RX_START;
			}
			else
			{
				Value |= (BandIdx) ? (MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS) : (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value2 &= (BandIdx) ?  (~ARB_RQCR_RX1_START) : (~ARB_RQCR_RX_START);
			}
			break;
		case ASIC_MAC_TXRX_RXV:
			if (Enable)
			{
				Value &= (BandIdx) ?  (~(MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS)) : (~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS));
				Value2 |= (BandIdx) ?
							((ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN)) :
							((ARB_RQCR_RX_START | ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN));
			}
			else
			{
				Value |= (BandIdx) ? (MT_ARB_SCR_TX1DIS | MT_ARB_SCR_RX1DIS) : (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value2 &= (BandIdx) ?
							~(ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN) :
							~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
			}
			break;
		case ASIC_MAC_RXV:
			if (Enable)
			{
				Value &= (BandIdx) ? ~MT_ARB_SCR_RX1DIS : ~MT_ARB_SCR_RXDIS;
				Value2 |= (BandIdx) ?
						(ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN):
						(ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
			}
			else
			{
				Value2 &= (BandIdx) ?
					~(ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN):
					~(ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV1_T_EN);
			}
			break;
		case ASIC_MAC_RX_RXV:
			if (Enable)
			{

				Value &= (BandIdx) ? ~MT_ARB_SCR_RX1DIS : ~MT_ARB_SCR_RXDIS;
				Value2 |= (BandIdx) ?
						(ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN):
						(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);
			}
			else
			{
				Value |= (BandIdx) ?  MT_ARB_SCR_RX1DIS : MT_ARB_SCR_RXDIS;
				Value2 &=(BandIdx) ?
					~(ARB_RQCR_RX1_START | ARB_RQCR_RXV1_START |ARB_RQCR_RXV1_R_EN | ARB_RQCR_RXV1_T_EN):
				 	~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN) ;
			}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d)\n", __FUNCTION__,TxRx));
			break;
	}

	MAC_IO_WRITE32(pAd, ARB_SCR, Value);
	MAC_IO_WRITE32(pAd, ARB_RQCR, Value2);

	/*Set TX Queue*/
	for(i=0;i<pAd->chipCap.WmmHwNum;i++)
	{
		MtAsicSetTxQ(pAd,i,BandIdx,Enable);
	}

	return TRUE;
}

INT MtAsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable, UINT8 WPDMABurstSIZE)
{
#ifdef RTMP_MAC_PCI
	WPDMA_GLO_CFG_STRUC GloCfg;
#ifndef MT7622
	UINT32 Value1;
	UINT32 Value2;
#endif
#ifdef CONFIG_DELAY_INT
	UINT32 Value;
#endif

	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
#ifndef MT7622
	HW_IO_READ32(pAd, 0x7158, &Value1);
	HW_IO_READ32(pAd, 0x7000, &Value2);
#endif /* MT7622 */

#ifdef CONFIG_DELAY_INT
	HIF_IO_READ32(pAd, MT_DELAY_INT_CFG, &Value);
#endif


	switch (TxRx)
	{
		case PDMA_TX:
			if (enable == TRUE)
			{
				GloCfg.MT7615_field.EnableTxDMA = 1;
				GloCfg.MT7615_field.EnTXWriteBackDDONE = 1;
				GloCfg.MT7615_field.WPDMABurstSIZE = WPDMABurstSIZE;
				GloCfg.MT7615_field.multi_dma_en = 3;
#ifndef MT7622
				Value1 |= (1 << 16);
#endif
#ifdef CONFIG_DELAY_INT
				Value |= TX_DLY_INT_EN;
				Value &= ~TX_MAX_PINT_MASK;
				Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
				Value &= ~TX_MAX_PTIME_MASK;
				Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
#endif


			} else {
				GloCfg.MT7615_field.EnableTxDMA = 0;
#ifdef MT7615
				/* disable pdma pre-fetch */
				GloCfg.MT7615_field.multi_dma_en = 2;
#endif
#ifdef CONFIG_DELAY_INT
				Value &= ~TX_DLY_INT_EN;
				Value &= ~TX_MAX_PINT_MASK;
				Value &= ~TX_MAX_PTIME_MASK;
#endif
			}
#ifndef MT7622
			Value2 &= ~BIT23;
#endif
			break;
		case PDMA_RX:
			if (enable == TRUE)
			{
				GloCfg.MT7615_field.EnableRxDMA = 1;
				GloCfg.MT7615_field.WPDMABurstSIZE = WPDMABurstSIZE;
				GloCfg.MT7615_field.multi_dma_en = 3;
#ifndef MT7622
				Value1 |= (1 << 16);
#endif

#ifdef CONFIG_DELAY_INT
				Value |= RX_DLY_INT_EN;
				Value &= ~RX_MAX_PINT_MASK;
				Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
				Value &= ~RX_MAX_PTIME_MASK;
				Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif
			} else {
				GloCfg.MT7615_field.EnableRxDMA = 0;
#ifdef MT7615
				/* disable pdma pre-fetch */
				GloCfg.MT7615_field.multi_dma_en = 2;
#endif
#ifdef CONFIG_DELAY_INT
				Value &= ~RX_DLY_INT_EN;
				Value &= ~RX_MAX_PINT_MASK;
				Value &= ~RX_MAX_PTIME_MASK;
#endif
			}
#ifndef MT7622
			Value2 &= ~BIT23;
#endif
			break;
		case PDMA_TX_RX:
			if (enable == TRUE)
			{
				GloCfg.MT7615_field.EnableTxDMA = 1;
				GloCfg.MT7615_field.EnableRxDMA = 1;
				GloCfg.MT7615_field.EnTXWriteBackDDONE = 1;
				GloCfg.MT7615_field.WPDMABurstSIZE = WPDMABurstSIZE;
				GloCfg.MT7615_field.multi_dma_en = 3;

				if (MTK_REV_GTE(pAd, MT7615, MT7615E3))
				{
					#ifdef CONFIG_RALINK_MT7621
					/*
					 * Tx Burst Size: three bits are used to specify
					 * 256 bytes, bit0:0, bit21:10
					 */
					GloCfg.MT7615_E3_field.tx_bt_size_bit0 = 0;
					GloCfg.MT7615_E3_field.tx_bt_size_bit21 = 2;
					#else /* CONFIG_RALINK_MT7621 */
					/*
					 * Tx Burst Size: three bits are used to specify
					 * 128 bytes, bit0:1, bit21:1
					 */
					GloCfg.MT7615_E3_field.tx_bt_size_bit0 = 1;
					GloCfg.MT7615_E3_field.tx_bt_size_bit21 = 1;
					#endif /* CONFIG_RALINK_MT7621 */
					GloCfg.MT7615_E3_field.first_token_only = 1;
				}
#ifndef MT7622
				Value1 |= (1 << 16);
#endif

#ifdef CONFIG_DELAY_INT
				Value |= TX_DLY_INT_EN;
				Value &= ~TX_MAX_PINT_MASK;
				Value |= TX_MAX_PINT(TX_PENDING_INT_NUMS);
				Value &= ~TX_MAX_PTIME_MASK;
				Value |= TX_MAX_PTIME(TX_PENDING_INT_TIME);
				Value |= RX_DLY_INT_EN;
				Value &= ~RX_MAX_PINT_MASK;
				Value |= RX_MAX_PINT(RX_PENDING_INT_NUMS);
				Value &= ~RX_MAX_PTIME_MASK;
				Value |= RX_MAX_PTIME(RX_PENDING_INT_TIME);
#endif


			} else {
				GloCfg.MT7615_field.EnableRxDMA = 0;
				GloCfg.MT7615_field.EnableTxDMA = 0;
#ifdef MT7615
				/* disable pdma pre-fetch */
				GloCfg.MT7615_field.multi_dma_en = 2;
#endif
#ifdef CONFIG_DELAY_INT
				Value &= ~TX_DLY_INT_EN;
				Value &= ~TX_MAX_PINT_MASK;
				Value &= ~TX_MAX_PTIME_MASK;
				Value &= ~RX_DLY_INT_EN;
				Value &= ~RX_MAX_PINT_MASK;
				Value &= ~RX_MAX_PTIME_MASK;
#endif
			}
#ifndef MT7622
			Value2 &= ~BIT23;
#endif
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __FUNCTION__, TxRx));
			break;
	}
	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, GloCfg.word);
#ifndef MT7622
	HW_IO_WRITE32(pAd, 0x7158, Value1);
	HW_IO_WRITE32(pAd, 0x7000, Value2);
#endif

#ifdef CONFIG_DELAY_INT
	HIF_IO_WRITE32(pAd, MT_DELAY_INT_CFG, Value);
#endif

#endif

	return TRUE;
}


BOOLEAN MtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#ifdef RTMP_MAC_PCI
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;

	// TODO: shiang-MT7615
	do {
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.MT7615_field.TxDMABusy == 0)  && (GloCfg.MT7615_field.RxDMABusy == 0)) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMAIdle, GloCfg=0x%x\n", GloCfg.word));
			return TRUE;
		}
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return FALSE;
		RtmpusecDelay(wait_us);
	}while ((i++) < round);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMABusy, GloCfg=0x%x\n", GloCfg.word));

	return FALSE;
#endif
	return TRUE;
}


/*Dir- 0:TX, 1:RX*/
#define USB_DMA_TX_BUSY 1 <<31
#define USB_DMA_RX_BUSY 1 <<30

BOOLEAN MtAsicCheckDMAIdle(RTMP_ADAPTER *pAd,UINT8 Dir)
{
	UINT32 MacReg = 0;

#ifdef RTMP_MAC_PCI
	WPDMA_GLO_CFG_STRUC GloCfg;
	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
	if ((!Dir &&  !GloCfg.field.TxDMABusy) || (Dir && !GloCfg.field.RxDMABusy))
	{
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMA Tx Idle, MacReg=0x%x\n", MacReg));
		return TRUE;
	}
	return FALSE;
#endif


	return TRUE;
}


INT MtAsicSetMacWD(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT MtAsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums,UCHAR BandIdx)
{
	UINT32 Value;
	UINT32 Reg;

	Reg  = (BandIdx) ? TMAC_TCR1 : TMAC_TCR;

	MAC_IO_READ32(pAd, Reg, &Value);
	Value &= ~TMAC_TCR_TX_STREAM_NUM_MASK;
	Value |= TMAC_TCR_TX_STREAM_NUM(StreamNums - 1);
	MAC_IO_WRITE32(pAd, Reg, Value);

	return TRUE;
}

#ifdef NEW_SET_RX_STREAM

INT MtAsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                    ("%s(): StreamNums(%d) BandIdx(%d)\n",
                    __FUNCTION__, StreamNums, BandIdx));

    if (pAd->chipOps.ChipSetRxStream)
    {
        return pAd->chipOps.ChipSetRxStream(pAd, StreamNums, BandIdx);
    }
    else
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("For this chip, no specified set RxStream function!\n"));
    }

    return TRUE;
}

#else

INT MtAsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
	UINT32 Value, Mask = 0;
	UINT32 Reg;
	INT Ret = TRUE;

#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not finish Yet!\n",
				__FUNCTION__, __LINE__));

return 0;
#endif /* defined(MT7615) || defined(MT7622) */

	Reg = (BandIdx) ? RMAC_RMCR_BAND_1 : RMAC_RMCR;
	MAC_IO_READ32(pAd, Reg, &Value);

	Value &= ~(RMAC_RMCR_RX_STREAM_0 |
				RMAC_RMCR_RX_STREAM_1 |
				RMAC_RMCR_RX_STREAM_2);

	switch (StreamNums) {
		case 3:
			Mask |= RMAC_RMCR_RX_STREAM_2;
		case 2:
			Mask |= RMAC_RMCR_RX_STREAM_1;
		case 1:
			Mask |= RMAC_RMCR_RX_STREAM_0;
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal StreamNums(%d\n", StreamNums));
			Ret = FALSE;
			break;
	}

	if (Ret)
	{
		Value |= Mask;

		Value &= ~RMAC_RMCR_SMPS_MODE_MASK;
		Value |= RMAC_RMCR_SMPS_MODE(DISABLE_SMPS_RX_BYSETTING);

		MAC_IO_WRITE32(pAd, Reg, Value);
	}

	return Ret;
}

#endif

INT MtAsicSetBW(RTMP_ADAPTER *pAd, INT bw, UCHAR BandIdx)
{
	UINT32 val, offset;

#ifndef COMPOS_WIN
	// TODO: shiang-usw, some CR setting in bbp_set_bw() need to take care!!
	bbp_set_bw(pAd, bw, BandIdx);
#endif /* COMPOS_WIN */

	offset = (BandIdx == 0) ? 2 : 18;
	MAC_IO_READ32(pAd, AGG_BWCR, &val);
	val &= ~(3 << offset);
	switch (bw)
	{
		case BW_20:
			val |= (0 << offset);
			break;
		case BW_40:
			val |= (0x1 << offset);
			break;
		case BW_80:
			val |= (0x2 << offset);
			break;
		case BW_160:
		case BW_8080:
			val |= (0x3 << offset);
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF,
						("%s():Invalid BW(%d)!\n", __FUNCTION__, bw));
	}
	MAC_IO_WRITE32(pAd, AGG_BWCR, val);

	return TRUE;
}


INT MtAsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel,UCHAR BandIdx)
{
	UINT32 Value = 0, Mask = 0;
	INT Ret = TRUE;
	UINT32 Reg = 0;

#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not finish Yet!\n",
				__FUNCTION__, __LINE__));

return 0;
#endif /* defined(MT7615) || defined(MT7622) */

	Reg = (BandIdx) ? RMAC_RMCR_BAND_1 : RMAC_RMCR;

	MAC_IO_READ32(pAd, Reg, &Value);

	Value &= ~(RMAC_RMCR_RX_STREAM_0 |
				RMAC_RMCR_RX_STREAM_1 |
				RMAC_RMCR_RX_STREAM_2);

	switch (RxPathSel) {
	case 0: /* ALL */
	Mask = (RMAC_RMCR_RX_STREAM_0 | RMAC_RMCR_RX_STREAM_1 | RMAC_RMCR_RX_STREAM_2);
	break;
	case 1: /* RX0 */
	Mask = RMAC_RMCR_RX_STREAM_0;
	break;
	case 2: /* RX1 */
	Mask = RMAC_RMCR_RX_STREAM_1;
	break;
	case 3: /* RX2 */
	    Mask = RMAC_RMCR_RX_STREAM_2;
	    break;
	default:
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("illegal RxPath(%d\n", RxPathSel));
	    Ret = FALSE;
	    break;
	}

	if (Ret)
	{
		Value |= Mask;
		Value &= ~RMAC_RMCR_SMPS_MODE_MASK;
		Value |= RMAC_RMCR_SMPS_MODE(DISABLE_SMPS_RX_BYSETTING);
		MAC_IO_WRITE32(pAd, Reg, Value);
	}
	return Ret;
}



UINT32 MtAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
	UINT32 value = 0;
	switch (type) {
	case HQA_RX_STAT_MACFCSERRCNT:
		MAC_IO_READ32(pAd,MIB_M0SDR3,&value);
#if defined(MT7615) || defined(MT7622)
		value = value & 0xFFFF; /* [15:0] FCS ERR */
#else
		value = (value >> 16) & 0xFFFF; /* [31:16] FCS ERR */
#endif /* defined(MT7615) || defined(MT7622) */
		break;
	case HQA_RX_STAT_MAC_MDRDYCNT:
		MAC_IO_READ32(pAd,MIB_M0SDR10,&value);
#if defined(MT7615) || defined(MT7622)
		value = value & 0x3FFFFFF; /* [15:0] Mac Mdrdy*/
#endif /* defined(MT7615) || defined(MT7622) */
		break;
	case HQA_RX_STAT_MAC_RXLENMISMATCH:
		MAC_IO_READ32(pAd,MIB_M0SDR11,&value);
		value = value & 0xFFFF;
		break;
	case HQA_RX_STAT_PHY_MDRDYCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS5, &value);
		break;
	case HQA_RX_STAT_PHY_FCSERRCNT:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS4, &value);
		break;
	case HQA_RX_STAT_PD:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS0, &value);
		break;
	case HQA_RX_STAT_CCK_SIG_SFD:
		/* [31:16] SIG [15:0] SFD */
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS1, &value);
		break;
	case HQA_RX_STAT_OFDM_SIG_TAG:
		/* [31:16] SIG [15:0] TAG */
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS2, &value);
		break;
	case HQA_RX_STAT_RSSI:
		PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_2, &value);
		break;
	case  HQA_RX_STAT_RSSI_RX23:
		PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_2, &value);
		break;
	case HQA_RX_STAT_ACI_HITL:
		PHY_IO_READ32(pAd, RO_BAND0_AGC_DEBUG_4, &value);
		break;
	case HQA_RX_STAT_ACI_HITH:
		PHY_IO_READ32(pAd, RO_BAND1_AGC_DEBUG_4, &value);
		break;
	case HQA_RX_FIFO_FULL_COUNT:
		MAC_IO_READ32(pAd, MIB_M0SDR4, &value);
		value = (value >> 16 ) & 0xffff;
		break;
	case HQA_RX_FIFO_FULL_COUNT_BAND1:
		MAC_IO_READ32(pAd, MIB_M1SDR4, &value);
		value = (value >> 16 ) & 0xffff;
		break;
	case HQA_RX_RESET_PHY_COUNT:
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS5, &value);
		PHY_IO_READ32(pAd, RO_BAND0_PHYCTRL_STS4, &value);
		break;
	case HQA_RX_STAT_MACFCSERRCNT_BAND1:
		MAC_IO_READ32(pAd,MIB_M1SDR3,&value);
		break;
	case HQA_RX_STAT_MAC_MDRDYCNT_BAND1:
		MAC_IO_READ32(pAd,MIB_M1SDR10,&value);
		break;
	case HQA_RX_STAT_MAC_RXLENMISMATCH_BAND1:
		MAC_IO_READ32(pAd,MIB_M1SDR11,&value);
		break;
	case HQA_RX_RESET_MAC_COUNT:
		MAC_IO_READ32(pAd,MIB_M0SDR3,&value);
		MAC_IO_READ32(pAd,MIB_M0SDR10,&value);
		MAC_IO_READ32(pAd,MIB_M1SDR3,&value);
		MAC_IO_READ32(pAd,MIB_M1SDR10,&value);
		break;
	case HQA_RX_STAT_PHY_MDRDYCNT_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND1_PHYCTRL_STS5, &value);
		break;
	case HQA_RX_STAT_PHY_FCSERRCNT_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND1_PHYCTRL_STS4, &value);
		break;
	case HQA_RX_STAT_PD_BAND1:
		/* [31:16] OFDM [15:0] CCK */
		PHY_IO_READ32(pAd, RO_BAND1_PHYCTRL_STS0, &value);
		break;
	case HQA_RX_STAT_CCK_SIG_SFD_BAND1:
		/* [31:16] SIG [15:0] SFD */
		PHY_IO_READ32(pAd, RO_BAND1_PHYCTRL_STS1, &value);
		break;
	case HQA_RX_STAT_OFDM_SIG_TAG_BAND1:
		/* [31:16] SIG [15:0] TAG */
		PHY_IO_READ32(pAd, RO_BAND1_PHYCTRL_STS2, &value);
		break;
	default:
		break;
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type(%d):%x\n", __FUNCTION__, type, value));
	return value;
}

#ifdef CONFIG_ATE
INT MtAsicSetRfFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
	UINT32 Value = 0;
	INT Ret = TRUE;

#if defined(MT7615) || defined(MT7622)
// TODO: shiang-MT7615, fix me!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Not finish Yet!\n",
				__FUNCTION__, __LINE__));

	return 0;
#endif /* defined(MT7615) || defined(MT7622) */

    if (FreqOffset > 127)
        FreqOffset = 127;

    HW_IO_READ32(pAd, XTAL_CTL13, &Value);
    Value &= ~DA_XO_C2_MASK; /* [14:8] (DA_XO_C2) */
    Value |= DA_XO_C2(0x3C); //set 60(DEC)
    HW_IO_WRITE32(pAd,XTAL_CTL13,Value);

    HW_IO_READ32(pAd, XTAL_CTL14, &Value);
    Value &= ~DA_XO_C2_MASK;
    Value |= DA_XO_C2(0x7F);
    HW_IO_WRITE32(pAd,XTAL_CTL14,Value);

    HW_IO_READ32(pAd, XTAL_CTL13, &Value);
    Value &= ~DA_XO_C2_MASK; /* [14:8] (DA_XO_C2) */
    Value |= DA_XO_C2(FreqOffset); //set 60(DEC)
    HW_IO_WRITE32(pAd,XTAL_CTL13,Value);

    return Ret;
}


INT MtAsicSetTSSI(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	UINT32 WF0Offset = 0x10D04; /* WF_PHY_CR_FRONT CR_WF0_TSSI_1 */
	UINT32 WF1Offset = 0x11D04; /* WF_PHY_CR_FRONT CR_WF1_TSSI_1 */
    INT Ret = TRUE;
    /* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
    /* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (FALSE == bOnOff)
		CRValue = 0xE3F3F800;
	else
		CRValue = 0xE1010800;

	if ((0 == WFSelect) || (2 == WFSelect)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Set WF#%d TSSI off\n",__FUNCTION__, WFSelect));
		PHY_IO_WRITE32(pAd,WF0Offset,CRValue);//3
		if(FALSE == bOnOff)
		{//off
			PHY_IO_WRITE32(pAd,0x10D18,0x0);// 3
		}
	}

	if ((1 == WFSelect) || (2 == WFSelect)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Set WF#%d TSSI on\n",__FUNCTION__, WFSelect));
		PHY_IO_WRITE32(pAd,WF1Offset,CRValue);//3
		if(FALSE == bOnOff)
		{//off
			PHY_IO_WRITE32(pAd,0x11D18,0x0);// 3
		}
	}

    return Ret;
}


INT MtAsicSetDPD(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	UINT32 CRValue = 0x0;
	ULONG WF0Offset = 0x10A08;
	ULONG WF1Offset = 0x11A08;
    INT Ret = TRUE;
    /* !!TEST MODE ONLY!! Normal Mode control by FW and Never disable */
    /* WF0 = 0, WF1 = 1, WF ALL = 2 */

	if (FALSE == bOnOff) {
		//WF0
		if((0 == WFSelect) || (2 == WFSelect)) {
			PHY_IO_READ32(pAd, WF0Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd,WF0Offset,CRValue);//3
		}
		//WF1
		if((1 == WFSelect) || (2 == WFSelect)) {
			PHY_IO_READ32(pAd, WF1Offset, &CRValue);
			CRValue |= 0xF0000000;
			PHY_IO_WRITE32(pAd,WF1Offset,CRValue);//3
		}
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Set WFSelect: %d DPD off\n",__FUNCTION__, WFSelect));
	} else {
		if ((0 == WFSelect) || (2 == WFSelect)) {
			PHY_IO_READ32(pAd, WF0Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd,WF0Offset,CRValue);//3
		}
		if ((1 == WFSelect) || (2 == WFSelect)) {
			PHY_IO_READ32(pAd, WF1Offset, &CRValue);
			CRValue &= (~0xF0000000);
			PHY_IO_WRITE32(pAd,WF1Offset,CRValue);//3
		}
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Set WFSelect: %d DPD on\n",__FUNCTION__, WFSelect));
	}

    return Ret;
}



#ifdef COMPOS_TESTMODE_WIN
/**
 * Tx Set Frequency Offset
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param iOffset value
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxSetFrequencyOffset (RTMP_ADAPTER *pAd, UINT32 iOffset, BOOLEAN HasBeenSet)
{
	UINT32 Value = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s iOffset=0x%x-->\n",__FUNCTION__,iOffset));

	if(HasBeenSet == FALSE)
	{
		if(IS_MT7603(pAd))
		{
			//RG_XO_C2[8:14]  Set 60 (0x3C )at first
			MAC_IO_READ32(pAd,RG_XO_C2,&Value);
			Value = (Value & 0xFFFF80FF) | (0x3C << 8);
			MAC_IO_WRITE32(pAd,RG_XO_C2,Value);
			//RG_XO_C2_MANUAL [8:14]  Set 0x7F at first
			MAC_IO_READ32(pAd,RG_XO_C2_MANUAL,&Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd,RG_XO_C2_MANUAL,Value);
			//only set at first time
		}
		else if(IS_MT76x6(pAd))
		{
			MAC_IO_READ32(pAd,FREQ_OFFSET_MANUAL_ENABLE,&Value);
			Value = (Value & 0xFFFF80FF) | (0x7F << 8);
			MAC_IO_WRITE32(pAd,FREQ_OFFSET_MANUAL_ENABLE,Value);

		}
	}
	if(IS_MT7603(pAd))
	{
		HW_IO_READ32(pAd,RG_XO_C2,&Value);
		Value = (Value & 0xFFFF80FF) | (iOffset << 8);
		HW_IO_WRITE32(pAd,RG_XO_C2,Value);


	}
	return 0;

}
/**
 * Set Tx Power Range
 *
 * @param pDeviceObject pointer PDEVICE_OBJECT
 * @param ucMaxPowerDbm, Max Power Dbm
 * @param ucMinPowerDbm, Min Power Dbm
 *
 * @return void
 * Otherwise, an error code is returned.
 */
INT MTAsicTxConfigPowerRange(RTMP_ADAPTER *pAd, IN UCHAR ucMaxPowerDbm,IN UCHAR ucMinPowerDbm)
{
	UINT32 u4RegValue;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s-->\n",__FUNCTION__));

	MAC_IO_READ32(pAd,TMAC_FPCR,&u4RegValue);

	u4RegValue &= ~(FPCR_FRAME_POWER_MAX_DBM | FPCR_FRAME_POWER_MIN_DBM);
	u4RegValue |= ((ucMaxPowerDbm << FPCR_FRAME_POWER_MAX_DBM_OFFSET) & FPCR_FRAME_POWER_MAX_DBM);
	u4RegValue |= ((ucMinPowerDbm << FPCR_FRAME_POWER_MIN_DBM_OFFSET) & FPCR_FRAME_POWER_MIN_DBM);

	MAC_IO_WRITE32(pAd,TMAC_FPCR,u4RegValue);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s<--\n",__FUNCTION__));
	return 0;
}

INT MTAsicSetTMR(RTMP_ADAPTER *pAd,UCHAR enable, UCHAR BandIdx)
{
	UINT32 value = 0;
	UINT32 Reg = (BandIdx) ? RMAC_TMR_PA_BAND_1 : RMAC_TMR_PA;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, -->\n",__FUNCTION__));

	switch (enable)
	{
		case 1://initialiter
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: enable TMR report, as Initialiter\n", __FUNCTION__));
			MAC_IO_READ32(pAd,Reg, &value);//::TODO CMD_GET_MACFCSERRORCNT_COUNT
			value = value | BIT31;
			value = value & ~BIT30;
			value = value | 0x34;//Action frame register
			MAC_IO_WRITE32(pAd,Reg, value);
		}
		break;
		case 2://responder
       	{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: enable TMR report, as Responser\n", __FUNCTION__));
			MAC_IO_READ32(pAd,Reg, &value);//::TODO CMD_GET_MACFCSERRORCNT_COUNT
			value = value | BIT31;
			value = value | BIT30;
			value = value | 0x34;//Action frame register
			MAC_IO_WRITE32(pAd,Reg, value);
		}
		break;
		case 0://disable
		default:
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s: disable TMR report\n", __FUNCTION__));
			MAC_IO_READ32(pAd,Reg, &value);//::TODO CMD_GET_MACFCSERRORCNT_COUNT
			value = value & ~BIT31;
			MAC_IO_WRITE32(pAd,Reg, value);
		}
	}
	return 0;
}
#endif
#endif /* CONFIG_ATE */

#ifndef  MAC_INIT_OFFLOAD
VOID MtAsicSetRxGroup(RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable)
{
	UINT32 Value;


    MCU_IO_READ32(pAd,RXINF,&Value);
    if (Enable)
    {
        if (Group & RXS_GROUP1)
            Value |= RXSH_GROUP1_EN;

        if (Group & RXS_GROUP2)
            Value |= RXSH_GROUP2_EN;

        if (Group & RXS_GROUP3)
            Value |= RXSH_GROUP3_EN;
    }
    else
    {
        if (Group & RXS_GROUP1)
            Value &= ~RXSH_GROUP1_EN;

        if (Group & RXS_GROUP2)
            Value &= ~RXSH_GROUP2_EN;

        if (Group & RXS_GROUP3)
            Value &= ~RXSH_GROUP3_EN;
    }
    MCU_IO_WRITE32(pAd, RXINF, Value);

    MAC_IO_READ32(pAd, DMA_DCR1, &Value);
    if (Enable)
    {
        if (Group & RXS_GROUP1)
            Value |= RXSM_GROUP1_EN;

        if (Group & RXS_GROUP2)
            Value |= RXSM_GROUP2_EN;

        if (Group & RXS_GROUP3)
            Value |= RXSM_GROUP3_EN;
    }
    else
    {
        if (Group & RXS_GROUP1)
            Value &= ~RXSM_GROUP1_EN;

        if (Group & RXS_GROUP2)
            Value &= ~RXSM_GROUP2_EN;

        if (Group & RXS_GROUP3)
            Value &= ~RXSM_GROUP3_EN;
    }

    MAC_IO_WRITE32(pAd, DMA_DCR1, Value);
}



INT MtAsicSetBAWinSizeRange(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	/* A-MPDU BA WinSize control */
	MAC_IO_READ32(pAd, AGG_AWSCR0, &mac_val);
	mac_val &= ~WINSIZE0_MASK;
	mac_val |= WINSIZE0(ba_range[0]);
	mac_val &= ~WINSIZE1_MASK;
	mac_val |= WINSIZE1(ba_range[1]);
	mac_val &= ~WINSIZE2_MASK;
	mac_val |= WINSIZE2(ba_range[2]);
	mac_val &= ~WINSIZE3_MASK;
	mac_val |= WINSIZE3(ba_range[3]);
	MAC_IO_WRITE32(pAd, AGG_AWSCR0, mac_val);

	MAC_IO_READ32(pAd, AGG_AWSCR1, &mac_val);
	mac_val &= ~WINSIZE4_MASK;
	mac_val |= WINSIZE4(ba_range[4]);
	mac_val &= ~WINSIZE5_MASK;
	mac_val |= WINSIZE5(ba_range[5]);
	mac_val &= ~WINSIZE6_MASK;
	mac_val |= WINSIZE6(ba_range[6]);
	mac_val &= ~WINSIZE7_MASK;
	mac_val |= WINSIZE7(ba_range[7]);
	MAC_IO_WRITE32(pAd, AGG_AWSCR1, mac_val);

	/* A-MPDU Agg limit control */
	/* We can delivery 64! */
	MAC_IO_WRITE32(pAd, AGG_AALCR0, 0);
	MAC_IO_WRITE32(pAd, AGG_AALCR1, 0);
	MAC_IO_WRITE32(pAd, AGG_AALCR2, 0);
	MAC_IO_WRITE32(pAd, AGG_AALCR3, 0);


	// TODO: shiang-MT7615, document mismatch, where should we set range 7??
	/* AMPDU Statistics Range Control setting
		0 < agg_cnt - 1 <= range_cr(0), 			=> 1
		range_cr(0) < agg_cnt - 1 <= range_cr(4),		=> 2~5
		range_cr(4) < agg_cnt - 1 <= range_cr(14),	=> 6~15
		range_cr(14) < agg_cnt - 1, 				=> 16~
	*/
	MAC_IO_READ32(pAd, AGG_ASRCR0, &mac_val);
	mac_val =  (ba_mib_range[0] << 0) | (ba_mib_range[1] << 8) |
				(ba_mib_range[2] << 16) | (ba_mib_range[3] << 24);
	MAC_IO_WRITE32(pAd, AGG_ASRCR0, mac_val);
	MAC_IO_READ32(pAd, AGG_ASRCR1, &mac_val);
	mac_val =  (ba_mib_range[4] << 0) | (ba_mib_range[5] << 8) |
				(ba_mib_range[6] << 16) | (ba_mib_range[7] << 24);
	MAC_IO_WRITE32(pAd, AGG_ASRCR1, mac_val);

	return TRUE;
}


INT MtAsicSetBARTxRate(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;


	// TODO: shiang-MT7615, document mismatch!!
	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd, AGG_ACR0, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd, AGG_ACR0, mac_val);

	return TRUE;
}


VOID MtAsicSetBARTxCntLimit(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count)
{
	UINT32 Value;

	// TODO: check for RTY_MODE!!

	MAC_IO_READ32(pAd, AGG_MRCR, &Value);
	if (Enable)
	{
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(Count);
	}
	else
	{
		Value &= ~BAR_TX_CNT_LIMIT_MASK;
		Value |= BAR_TX_CNT_LIMIT(0);
	}
	MAC_IO_WRITE32(pAd, AGG_MRCR, Value);
}


VOID MtAsicSetTxSClassifyFilter(RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ,
								UINT32 AggNums, UINT32 Filter,UCHAR BandIdx)
{
	UINT32 Value;
	UINT32 Reg;

	if (Port == TXS2HOST)
	{
		Reg = (BandIdx) ? DMA_BN1TCFR1 : DMA_BN0TCFR1;
		MAC_IO_READ32(pAd, Reg, &Value);
		Value &= ~TXS2H_BIT_MAP_MASK;
		Value |= TXS2H_BIT_MAP(Filter);
		Value &= ~TXS2H_AGG_CNT_MASK;
		Value |= TXS2H_AGG_CNT(AggNums);
		if (DestQ == 0)
			Value &= ~TXS2H_QID;
		else
			Value |= TXS2H_QID;
		MAC_IO_WRITE32(pAd, Reg, Value);

	}
	else if (Port == TXS2MCU)
	{

		Reg = (BandIdx) ? DMA_BN1TCFR0 : DMA_BN0TCFR0;
		MAC_IO_READ32(pAd, Reg, &Value);
		Value &= ~TXS2M_BIT_MAP_MASK;
		Value |= TXS2M_BIT_MAP(Filter);
		Value &= ~TXS2M_AGG_CNT_MASK;
		Value |= TXS2M_AGG_CNT(AggNums);
		Value &= ~TXS2M_QID_MASK;
		Value |= TXS2M_QID(DestQ);
		MAC_IO_WRITE32(pAd, Reg, Value);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow Port(%d)\n", __FUNCTION__, Port));
	}
}
#endif /* MAC_INIT_OFFLOAD */

INT32 MtAsicGetMacInfo(RTMP_ADAPTER *pAd,UINT32 *ChipId,UINT32 *HwVer, UINT32 *FwVer)
{
	UINT32 Value;

	HW_IO_READ32(pAd, TOP_HVR, &Value);
	*HwVer = Value;

	HW_IO_READ32(pAd, TOP_FVR, &Value);
	*FwVer = Value;

	HW_IO_READ32(pAd, TOP_HCR, &Value);
	*ChipId = Value;

	return TRUE;
}


INT32 MtAsicGetFwSyncValue(RTMP_ADAPTER *pAd)
{
	UINT32 value;

#ifdef MT7622
        RTMP_IO_READ32(pAd, TOP_OFF_RSV, &value);
        value = GET_SW_SYNC(value);
#else

	MAC_IO_READ32(pAd, TOP_MISC2, &value);
#endif
	MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,
				("%s: Current TOP_MISC2(0x%x)\n", __FUNCTION__, value));

	return value;
}


VOID MtAsicInitMac(RTMP_ADAPTER *pAd)
{
#ifndef BCN_OFFLOAD_SUPPORT
    UINT32 mac_val;
    MAC_IO_READ32(pAd, PLE_RELEASE_CTRL, &mac_val);
    mac_val = mac_val |
                SET_BCN0_RLS_QID(UMAC_PLE_CTRL_P3_Q_0X1F) |
                SET_BCN0_RLS_PID(P_IDX_PLE_CTRL_PSE_PORT_3) |
                SET_BCN1_RLS_QID(UMAC_PLE_CTRL_P3_Q_0X1F) |
                SET_BCN1_RLS_PID(P_IDX_PLE_CTRL_PSE_PORT_3);
    MAC_IO_WRITE32(pAd, PLE_RELEASE_CTRL, mac_val);
#endif

#ifndef MAC_INIT_OFFLOAD
        {
	UINT32 mac_val_bnd[2];

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	MtAsicSetBAWinSizeRange(pAd);

	/* TMR report queue setting */
	MAC_IO_READ32(pAd, DMA_BN0TMCFR0, &mac_val_bnd[0]);
	mac_val_bnd[0] |= BIT13;//TMR report send to HIF q1.
	mac_val_bnd[0] &= (~(BIT0));
	mac_val_bnd[0] &= (~(BIT1));
	MAC_IO_WRITE32(pAd, DMA_BN0TMCFR0, mac_val_bnd[0]);
	MAC_IO_READ32(pAd, RMAC_TMR_PA, &mac_val);
	mac_val = mac_val & ~BIT31;
	MAC_IO_WRITE32(pAd, RMAC_TMR_PA, mac_val);

#ifdef DBDC_MODE
	MAC_IO_READ32(pAd, DMA_BN1TMCFR0, &mac_val_bnd[1]);
	mac_val_bnd[1] = mac_val_bnd[0];
	MAC_IO_WRITE32(pAd, DMA_BN1TMCFR0, mac_val_bnd[1]);
	MAC_IO_READ32(pAd, RMAC_TMR_PA_BAND_1, &mac_val);
	mac_val = mac_val & ~BIT31;
	MAC_IO_WRITE32(pAd, RMAC_TMR_PA_BAND_1, mac_val);
#endif /*DBDC_MODE*/


	/* Configure all rx packets to HIF, except WOL2M packet */
	MAC_IO_READ32(pAd, DMA_BN0RCFR0, &mac_val_bnd[0]);
	mac_val_bnd[0] = 0x00010000; // drop duplicate
	// TODO: shiang-MT7615, make sure the bit31~30 setting for BN0_RX2M_QID
	mac_val_bnd[0] |= 0xc0108000; // receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED
	MAC_IO_WRITE32(pAd, DMA_BN0RCFR0, mac_val_bnd[0]);


	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd, DMA_BN0VCFR0, &mac_val);
	mac_val &= (~(BIT0)); // To HIF
	mac_val |= BIT13; // RxRing 1
	MAC_IO_WRITE32(pAd, DMA_BN0VCFR0, mac_val);

#ifdef DBDC_MODE
	MAC_IO_READ32(pAd, DMA_BN1RCFR0, &mac_val_bnd[1]);
	mac_val_bnd[1] = mac_val_bnd[0];
	MAC_IO_WRITE32(pAd, DMA_BN1RCFR0, mac_val_bnd[1]);
	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd, DMA_BN1VCFR0, &mac_val);
	mac_val &= (~(BIT0)); // To HIF
	mac_val |= BIT13; // RxRing 1
	MAC_IO_WRITE32(pAd, DMA_BN1VCFR0, mac_val);
#endif /*DBDC_MODE*/

	/*RCR for Ctrl Fram can match*/
	MAC_IO_READ32(pAd, WTBL_OFF_RCR, &mac_val);
	mac_val |= CHECK_CTRL(1);
	MAC_IO_WRITE32(pAd, WTBL_OFF_RCR, mac_val);

	// TODO: shiang-MT7615, need further check!!

	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	MtAsicSetBARTxCntLimit(pAd, TRUE, 1);

	MtAsicSetBARTxRate(pAd);
	// TODO: shiang-MT7615, document mismatch!!
	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd, AGG_ACR0, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd, AGG_ACR0, mac_val);

	// Enable MIB counters for band 0 and band 1
	MAC_IO_WRITE32(pAd, MIB_M0SCR0, 0x7effffff);
	MAC_IO_WRITE32(pAd, MIB_M0SCR1, 0xfffe);
	MAC_IO_WRITE32(pAd, MIB_M0PBSCR, 0x7f7f7f7f);
#ifdef DBDC_MODE
	MAC_IO_WRITE32(pAd, MIB_M1SCR, 0x7ef3ffff);
	MAC_IO_WRITE32(pAd, MIB_M1SCR1, 0xfffe);
	MAC_IO_WRITE32(pAd, MIB_M1PBSCR, 0x7f7f7f7f);
#endif /*DBDC_MODE*/
    /* Enable RxV to HIF/MCU */
    MtAsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);

	/* CCA Setting */
	MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd, TMAC_TRCR0, mac_val);

	// TODO: shiang-MT7615, need further check!!
        }
#endif /* MAC_INIT_OFFLOAD */
}

VOID MtAsicSetExtMbssMacByDriver(RTMP_ADAPTER *pAd, INT idx, VOID *wdev_void)
{
    struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
    UCHAR *pmac_addr = (UCHAR *)wdev->if_addr;

    UINT32 cr_base_0 = 0, cr_base_1 = 0, cr_val = 0;
    cr_base_0 = RMAC_MBSSIDEXT1_0 + (8 * (idx - 1));
    cr_base_1 = cr_base_0 + 4;

    cr_val = pmac_addr[0] | (pmac_addr[1] << 8) | (pmac_addr[2] << 16) | (pmac_addr[3] << 24);
    MAC_IO_WRITE32(pAd, cr_base_0, cr_val);
    cr_val = 0;
    cr_val = pmac_addr[4] | (pmac_addr[5] << 8);
    MAC_IO_WRITE32(pAd, cr_base_1, cr_val);
}

#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    UINT32 Value = 0;
    //where the register for sub Bssid start from
    UINT32 bssid_reg_base = LPON_SBTOR1;
    BOOLEAN any_other_mbss_enable = FALSE;

    ASSERT(mbss_idx >= 1);

    if (enable == TRUE)
    {
        //if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always.
        MAC_IO_READ32(pAd, bssid_reg_base, &Value);
        Value |= SBSS_TBTT0_TSF0_EN;
        MAC_IO_WRITE32(pAd, bssid_reg_base, Value);

        MAC_IO_READ32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
        Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
        Value |= SUB_BSSID0_TIME_OFFSET_n(mbss_idx * BCN_TRANSMIT_ESTIMATE_TIME);
        Value |= TBTT0_n_INT_EN;
        Value |= PRE_TBTT0_n_INT_EN;

        MAC_IO_WRITE32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
        pAd->ApCfg.ext_mbss_enable_bitmap |= (enable << mbss_idx);
    }
    else
    {
        pAd->ApCfg.ext_mbss_enable_bitmap &= ~(enable << mbss_idx);

        if (pAd->ApCfg.ext_mbss_enable_bitmap)
            any_other_mbss_enable = TRUE;

        //if there is any ext bssid is enable. this bit in LPON_SBTOR1 shall be 1 always.
        MAC_IO_READ32(pAd, bssid_reg_base, &Value);
        if (any_other_mbss_enable == TRUE)
            Value |= SBSS_TBTT0_TSF0_EN;
        else
            Value &= ~SBSS_TBTT0_TSF0_EN;
        MAC_IO_WRITE32(pAd, bssid_reg_base, Value);

        MAC_IO_READ32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), &Value);
        Value &= ~SUB_BSSID0_TIME_OFFSET_n_MASK;
        Value &= ~TBTT0_n_INT_EN;
        Value &= ~PRE_TBTT0_n_INT_EN;
        MAC_IO_WRITE32(pAd, (bssid_reg_base + (mbss_idx - 1) * (0x4)), Value);
    }
}

VOID MtDmacSetExtMbssEnableCR(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    UINT32 regValue;
    if (enable)
    {
        MAC_IO_READ32(pAd, RMAC_ACBEN, &regValue);
        regValue |=  (1 << mbss_idx);
        MAC_IO_WRITE32(pAd, RMAC_ACBEN, regValue);
    }
    else
    {
        MAC_IO_READ32(pAd, RMAC_ACBEN, &regValue);
        regValue &=  ~(1 << mbss_idx);
        MAC_IO_WRITE32(pAd, RMAC_ACBEN, regValue);
    }
}

VOID MtDmacSetMbssHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    MtAsicSetMbssLPOffset(pAd, mbss_idx, enable);
}

VOID MtAsicSetExtTTTTLPOffset(struct _RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    UINT32 Value = 0;
    //where the register for sub Bssid start from
    UINT32 bssid_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[mbss_idx];
    UINT32 first_reg_base = LPON_TT0SBOR_CR_MAPPING_TABLE[1];
    BOOLEAN any_other_mbss_tttt_enable = FALSE;

    if (mbss_idx == 0)
        return;

    if (enable == TRUE)
    {
        //if there is any sub bssid is enable. this bit in LPON_SBTOR1 shall be 1 always.
        MAC_IO_READ32(pAd, first_reg_base, &Value);
        Value |= SBSS_TTTT0_TSF0_EN;
        MAC_IO_WRITE32(pAd, first_reg_base, Value);

        MAC_IO_READ32(pAd, bssid_reg_base, &Value);
        Value &= ~SUB_BSSID0_TTTT_OFFSET_n_MASK;
        Value |= DEFAULT_TTTT_OFFSET_IN_MS;
        Value |= TTTT0_n_INT_EN;
        Value |= PRE_TTTT0_n_INT_EN;

        MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
        pAd->ApCfg.ext_mbss_tttt_enable_bitmap |= (enable << mbss_idx);
    }
    else
    {
        pAd->ApCfg.ext_mbss_tttt_enable_bitmap &= ~(enable << mbss_idx);

        if (pAd->ApCfg.ext_mbss_tttt_enable_bitmap)
            any_other_mbss_tttt_enable = TRUE;

        //if there is any ext bssid is enable. this bit shall be 1 always.
        MAC_IO_READ32(pAd, first_reg_base, &Value);
        if (any_other_mbss_tttt_enable == TRUE)
            Value |= SBSS_TTTT0_TSF0_EN;
        else
            Value &= ~SBSS_TTTT0_TSF0_EN;
        MAC_IO_WRITE32(pAd, first_reg_base, Value);

        MAC_IO_READ32(pAd, bssid_reg_base, &Value);
        Value &= ~TTTT0_n_INT_EN;
        Value &= ~PRE_TTTT0_n_INT_EN;
        MAC_IO_WRITE32(pAd, bssid_reg_base, Value);
    }
}

VOID MtDmacSetExtTTTTHwCRSetting(RTMP_ADAPTER *pAd, UCHAR mbss_idx, BOOLEAN enable)
{
    MtAsicSetExtTTTTLPOffset(pAd, mbss_idx, enable);
}
#endif /* CONFIG_AP_SUPPORT */


#ifdef DBDC_MODE
static INT32 GetBctrlBitBaseByType(UCHAR Type, UINT8 Index)
{
	INT32 Bit = 0;

	switch(Type){
	case  DBDC_TYPE_REPEATER:
	{
		Bit = REPEATER_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_PTA:
	{
		Bit = PTA_BAND_SELPTA_BIT_BASE;
	}
	break;
	case DBDC_TYPE_BF:
	{
		Bit = BF_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_MU:
	{
		Bit = MU_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_BSS:
	{
		Bit = BSS_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_MBSS:
	{
		Bit = MBSS_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_MGMT:
	{
		Bit = MNG_BAND_SEL_BIT_BASE;
	}
	break;
	case DBDC_TYPE_WMM:
	{
		Bit = WMM_BAND_SEL_BIT_BASE;
	}
	break;
	default:
		Bit = -1;
	break;
	}

	if(Bit >= 0)
	{
		Bit+=Index;
	}

	return Bit;
}

INT32 MtAsicGetDbdcCtrl(RTMP_ADAPTER *pAd,BCTRL_INFO_T *pbInfo)
{
	UINT32 Value=0,i=0,j=0;

	HW_IO_READ32(pAd,CFG_DBDC_CTRL0,&Value);

	/*DBDC enable will not need BctrlEntries so minus 1*/
	pbInfo->TotalNum = MAX_BCTRL_ENTRY-1;
	/*DBDC Enable*/
	pbInfo->DBDCEnable = (Value  >> DBDC_EN_BIT_BASE ) & 0x1;
	/*PTA*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_PTA;
	pbInfo->BctrlEntries[i].Index = 0;
	pbInfo->BctrlEntries[i].BandIdx =(Value  >> PTA_BAND_SELPTA_BIT_BASE) & 0x1;
	i++;
	/*MU*/
	pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MU;
	pbInfo->BctrlEntries[i].Index = 0;
	pbInfo->BctrlEntries[i].BandIdx = (Value  >> MU_BAND_SEL_BIT_BASE) & 0x1;
	i++;
	/*BF*/
	for(j=0;j<3;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BF;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+BF_BAND_SEL_BIT_BASE) ) & 0x1;
		i++;
	}
	/*WMM*/
	for(j=0;j<4;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_WMM;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+WMM_BAND_SEL_BIT_BASE) ) & 0x1;
		i++;
	}
	/*MGMT*/
	for(j=0;j<2;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MGMT;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+MNG_BAND_SEL_BIT_BASE) ) & 0x1;
		i++;
	}

	/*MBSS*/
	for(j=0;j<15;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_MBSS;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+MBSS_BAND_SEL_BIT_BASE) ) & 0x1;
		i++;
	}
	/*BSS*/
	for(j=0;j<5;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_BSS;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j + BSS_BAND_SEL_BIT_BASE)) & 0x1;
		i++;
	}

	HW_IO_READ32(pAd,CFG_DBDC_CTRL1,&Value);

	/*Repeater*/
	for(j=0;j<32;j++)
	{
		pbInfo->BctrlEntries[i].Type = DBDC_TYPE_REPEATER;
		pbInfo->BctrlEntries[i].Index = j;
		pbInfo->BctrlEntries[i].BandIdx = (Value  >> (j+REPEATER_BAND_SEL_BIT_BASE) ) & 0x1;
		i++;
	}
	return 0;
}


INT32 MtAsicSetDbdcCtrl(RTMP_ADAPTER *pAd,BCTRL_INFO_T *pbInfo)
{
	UINT32 Value1=0,Value2=0;
	UINT32 i=0;
	INT32 shift=0; /* GetBctrlBitBaseByType may return -1 , declare signed variable:shift */
	BCTRL_ENTRY_T *pEntry = NULL;

	/*Clock Control for Band1, Band0 is enabled by MacInit*/
	if(pbInfo->DBDCEnable)
	{
		MAC_IO_READ32(pAd, CFG_CCR, &Value1);
		Value1 |= (BIT30 | BIT24);
		MAC_IO_WRITE32(pAd, CFG_CCR, Value1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
	                            ("%s: MAC D1 2x 1x initial(val=%x)\n", __FUNCTION__, Value1));
	}
	/*BandCtrl*/
	MAC_IO_READ32(pAd,CFG_DBDC_CTRL0,&Value1);
	MAC_IO_READ32(pAd,CFG_DBDC_CTRL1,&Value2);

	Value1  |= DBDC_EN(pbInfo->DBDCEnable);

	for(i=0;i<pbInfo->TotalNum;i++)
	{
		pEntry = &pbInfo->BctrlEntries[i];
		if((shift = GetBctrlBitBaseByType(pEntry->Type,pEntry->Index)) < 0)
			continue;

		if(pEntry->Type != DBDC_TYPE_REPEATER)
		{
			Value1 &= ~(0x1 << shift);
			Value1 |= (pEntry->BandIdx & 0x1) << shift;
		}else
		{
			Value2 &= ~(0x1 << shift);
			Value2 |= (pEntry->BandIdx & 0x1) << shift;
		}

	}

	MAC_IO_WRITE32(pAd,CFG_DBDC_CTRL0,Value1);
	MAC_IO_WRITE32(pAd,CFG_DBDC_CTRL1,Value2);
	return 0;
}
#endif /*DBDC_MODE*/


#ifndef COMPOS_TESTMODE_WIN
VOID MtSetTmrCal(
    IN  PRTMP_ADAPTER   pAd,
    IN  UCHAR TmrType,
    IN  UCHAR Channel,
    IN UCHAR Bw)
{
    UINT32  value = 0;

    MAC_IO_READ32(pAd, TMAC_B0BRR0, &value);

    if (TmrType == TMR_DISABLE)
        /* Enanle Spatial Extension for ACK/BA/CTS after TMR Disable*/
        value |= BSSID00_RESP_SPE_EN;
    else
        /* Disable Spatial Extension for ACK/BA/CTS when TMR Enable*/
        value &= ~BSSID00_RESP_SPE_EN;

    MAC_IO_WRITE32(pAd, TMAC_B0BRR0, value);

}

VOID MtSetTmrCR(
    IN  PRTMP_ADAPTER   pAd,
    IN  UCHAR TmrType)
{
    UINT32  value = 0;

    MAC_IO_READ32(pAd, RMAC_TMR_PA, &value);

    /* Clear role bit */
    value &= ~RMAC_TMR_ROLE;

    if (TmrType == TMR_DISABLE)
    {
        value &= ~RMAC_TMR_ENABLE;
    }
    else
    {
        value |= RMAC_TMR_ENABLE;

        if (TmrType == TMR_RESPONDER)
        {
            value |= RMAC_TMR_ROLE;

            /* Set type/sub_type to Action frame */
            value &= 0xffffffc0;    /* clear type/sub_type field */
            value |= (SUBTYPE_ACTION << 2) | FC_TYPE_MGMT;
        }
    }

    MAC_IO_WRITE32(pAd, RMAC_TMR_PA, value);
}
#endif

/*
    enable/disable beaconQ,
    return status for disable beaconQ.
*/
static BOOLEAN MtDmacAsicSetBeaconQ(
                struct _RTMP_ADAPTER *pAd,
                UINT8 OmacIdx,
                UCHAR BandIdx,
                BOOLEAN enable)
{
    UINT32 cr_collection[2][4] =
                            {{ARB_TQSM0, ARB_TQSE0, ARB_TQFM0, ARB_TQFE0}, /*band 0*/
                            {ARB_TQSM1, ARB_TQSE1, ARB_TQFM1, ARB_TQFE1}};
    UINT32 Value = 0;
    UINT32 cr_base = 0;

    if(BandIdx >= DBDC_BAND_NUM)
    {
        MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                            ("%s: BandIdx >= 2\n", __FUNCTION__));
        return FALSE;
    }

    if (enable)
    {
        if (OmacIdx > HW_BSSID_MAX)
            cr_base = cr_collection[BandIdx][1];
        else
            cr_base = cr_collection[BandIdx][0];
    }
    else
    {
        if (OmacIdx > HW_BSSID_MAX)
            cr_base = cr_collection[BandIdx][3];
        else
            cr_base = cr_collection[BandIdx][2];
    }

    MAC_IO_READ32(pAd, cr_base, &Value);
    if (OmacIdx > HW_BSSID_MAX)
        Value |= (1 << OmacIdx);
    else
        Value |= (1 << (0x10 + OmacIdx));
    MAC_IO_WRITE32(pAd, cr_base, Value);

    return TRUE;
}

INT MtDmacAsicEnableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void, UCHAR NumOfBcns)
{
    struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
    BCN_BUF_STRUC *bcn_info = &wdev->bcn_buf;
    UINT8 OmacIdx = wdev->OmacIdx;
    UCHAR BandIdx = HcGetBandByWdev(wdev);

    if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW)
    {
        //FW help to disable beacon.
        return TRUE;
    }
    else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT)
    {
        return MtDmacAsicSetBeaconQ(pAd, OmacIdx, BandIdx, TRUE);
    }
    return TRUE;
}

INT MtDmacAsicDisableBeacon(struct _RTMP_ADAPTER *pAd, VOID *wdev_void)
{
	struct wifi_dev *wdev = (struct wifi_dev *)wdev_void;
    BCN_BUF_STRUC *bcn_info = &wdev->bcn_buf;
    UINT8 OmacIdx = wdev->OmacIdx;
    UCHAR BandIdx = HcGetBandByWdev(wdev);

    if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_FW)
    {
        //FW help to disable beacon.
        return TRUE;
    }
    else if (bcn_info->BcnUpdateMethod == BCN_GEN_BY_HOST_IN_PRETBTT)
    {
        return MtDmacAsicSetBeaconQ(pAd, OmacIdx, BandIdx, FALSE);
    }
    return TRUE;
}

INT32 MtAsicRxHeaderTransCtl(RTMP_ADAPTER *pAd, BOOLEAN En, BOOLEAN ChkBssid, BOOLEAN InSVlan, BOOLEAN RmVlan, BOOLEAN SwPcP)
{


	if (IS_MT7636(pAd) || IS_MT7615(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
	{
		return CmdRxHdrTransUpdate(pAd, En, ChkBssid, InSVlan, RmVlan, SwPcP);
	}
	else
	{
		return 0;
	}

}

INT32 MtAsicRxHeaderTaranBLCtl(RTMP_ADAPTER *pAd, UINT32 Index, BOOLEAN En, UINT32 EthType)
{

	if (IS_MT7636(pAd) || IS_MT7615(pAd) || IS_MT7637(pAd) || IS_MT7622(pAd))
	{
		return CmdRxHdrTransBLUpdate(pAd, Index, En, EthType);
	}
	else
	{
		return 0;
	}

}

INT MtAsicTOPInit(RTMP_ADAPTER *pAd)
{
#ifdef MT7615_FPGA
	mt7615_chk_top_default_cr_setting(pAd);
	mt7615_chk_hif_default_cr_setting(pAd);
#endif /* MT7615_FPGA */

#ifdef MT7622_FPGA
	mt7622_chk_top_default_cr_setting(pAd);
	mt7622_chk_hif_default_cr_setting(pAd);
#endif /* MT7622_FPGA */

	return TRUE;
}


INT set_get_fid(RTMP_ADAPTER *pAd, RTMP_STRING *arg)
{
    //TODO: Carter, at present, only can read pkt in Port2(LMAC port)
    volatile UCHAR   q_idx = 0, loop = 0, dw_idx = 0;
    volatile UINT32  head_fid_addr = 0, next_fid_addr = 0, value = 0x00000000L, dw_content;
    q_idx = simple_strtol(arg, 0, 10);

    value = 0x00400000 | (q_idx << 16);//port2. queue by input value.
    RTMP_IO_WRITE32(pAd, 0x8024, value);
    RTMP_IO_READ32(pAd, 0x8024, (UINT32 *)&head_fid_addr);//get head FID.
    head_fid_addr = head_fid_addr & 0xfff;

    if (head_fid_addr == 0xfff) {
        MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s, q_idx:%d empty!!\n", __func__, q_idx));
        return TRUE;
    }

    value = (0 | (head_fid_addr << 16));
    while (1) {
        for (dw_idx = 0; dw_idx < 8; dw_idx++) {
            RTMP_IO_READ32(pAd, ((MT_PCI_REMAP_ADDR_1 + (((value & 0x0fff0000) >> 16) * 128)) + (dw_idx * 4)), (UINT32 *)&dw_content);//get head FID.
            MTWF_LOG(DBG_CAT_CFG, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("pkt:%d, fid:%x, dw_idx = %d, dw_content = 0x%x\n", loop, ((value & 0x0fff0000) >> 16), dw_idx, dw_content));
        }
        RTMP_IO_WRITE32(pAd, 0x8028, value);
        RTMP_IO_READ32(pAd, 0x8028, (UINT32 *)&next_fid_addr);//get next FID.
        if ((next_fid_addr & 0xfff) == 0xfff) {
            return TRUE;
        }

        value = (0 | ((next_fid_addr & 0xffff) << 16));
        loop++;
        if (loop > 5) {
            return TRUE;
        }
    }
    return TRUE;
}


VOID MtAsicUpdateRtsThld(struct _RTMP_ADAPTER *pAd,
        MT_RTS_THRESHOLD_T *rts_thrld)
{
    UINT32 val;

	/* Config ASIC RTS threshold register*/
	MAC_IO_READ32(pAd, AGG_PCR1, &val);
	val &= ~RTS_THRESHOLD_MASK;
	val &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	val |= RTS_THRESHOLD(rts_thrld->pkt_len_thld);
	val |= RTS_PKT_NUM_THRESHOLD(rts_thrld->pkt_num_thld);
	MAC_IO_WRITE32(pAd, AGG_PCR1, val);
}

INT32 MtAsicGetAntMode(RTMP_ADAPTER *pAd,UCHAR *AntMode)
{

	return TRUE;
}


INT32 MtAsicSetDmaByPassMode(RTMP_ADAPTER *pAd,BOOLEAN isByPass)
{
	return TRUE;
}

BOOLEAN MtAsicGetMcuStatus(RTMP_ADAPTER *pAd, MCU_STAT State)
{
	return TRUE;
}

VOID MtAsicGetTxTscByDriver(RTMP_ADAPTER *pAd, struct wifi_dev *wdev, UCHAR *pTxTsc)
{
	USHORT Wcid = 0;
	struct wtbl_entry tb_entry;
	UINT32 val = 0;

	GET_GroupKey_WCID(wdev, Wcid);
	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
			("%s: Cannot found WTBL2/3/4 for WCID(%d)\n", __FUNCTION__, Wcid));
		return;
	}

	HW_IO_READ32(pAd, tb_entry.wtbl_addr + (4 * 9), &val);
	*pTxTsc     = val & 0xff;
	*(pTxTsc+1) = (val >> 8) & 0xff;
	*(pTxTsc+2) = (val >> 16) & 0xff;
	*(pTxTsc+3) = (val >> 24) & 0xff;

	HW_IO_READ32(pAd, tb_entry.wtbl_addr + (4 * 10), &val);
	*(pTxTsc+4) = val & 0xff;
	*(pTxTsc+5) = (val >> 8) & 0xff;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n",
				__FUNCTION__, Wcid,
				*pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));

	return;
}


INT32 MtAsicSetDevMac(
	RTMP_ADAPTER *pAd,
	UINT8 OwnMacIdx,
	UINT8 *OwnMacAddr,
	UINT8 BandIdx,
	UINT8 Active,
	UINT32 EnableFeature)

{
	UINT32 val;
	UINT32 own_mac_reg_base = RMAC_OMA0R0;//register for Own_Mac from 0x60140024

	if (Active)
	{
		val = (OwnMacAddr[0]) | (OwnMacAddr[1]<<8) |  (OwnMacAddr[2]<<16) | (OwnMacAddr[3]<<24);
		MAC_IO_WRITE32(pAd, own_mac_reg_base + (OwnMacIdx * 8), val);

		val = OwnMacAddr[4] | (OwnMacAddr[5]<<8) |	(1 <<16);
		MAC_IO_WRITE32(pAd, (own_mac_reg_base + 4) + (OwnMacIdx * 8), val);
	}

#if !defined(COMPOS_TESTMODE_WIN) && !defined(COMPOS_WIN)
	AsicDevInfoUpdate(pAd, OwnMacIdx, OwnMacAddr, BandIdx, Active, DEVINFO_ACTIVE_FEATURE);
#endif

	return TRUE;
}


BOOLEAN MtAsicSetBcnQCR(
    IN struct _RTMP_ADAPTER *pAd,
    IN UCHAR Operation,
    IN UCHAR HwBssidIdx,
    IN UINT32 apidx)
{
	return TRUE;
}

VOID MtAsicSetBssid(struct _RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx)
{

}


#ifdef DOT11_VHT_AC
INT MtAsicSetRtsSignalTA(RTMP_ADAPTER *pAd, UINT8 BandIdx, BOOLEAN Enable)
{
    UINT32 Value = 0;

    if (BandIdx) {
        MAC_IO_READ32(pAd, TMAC_TCR1, &Value);
    } else {
        MAC_IO_READ32(pAd, TMAC_TCR, &Value);
    }

    if (Enable) {
        Value |= RTS_SIGTA_EN;
    } else {
        Value &= ~(RTS_SIGTA_EN);
    }

    if (BandIdx) {
        MAC_IO_WRITE32(pAd, TMAC_TCR1, Value);
    } else {
        MAC_IO_WRITE32(pAd, TMAC_TCR, Value);
    }

    return TRUE;
}
#endif /* DOT11_VHT_AC */

INT MtAsicAMPDUEfficiencyAdjust(struct _RTMP_ADAPTER *ad,UCHAR	wmm_idx, UCHAR aifs_adjust)
{
	UINT32 cr;
	UINT32 value;

	cr = ARB_WMMAC01+(wmm_idx*16);

	MAC_IO_READ32(ad,cr,&value);

	value = ((value & 0xffffff00) | aifs_adjust);

	MAC_IO_WRITE32(ad,cr,value);

	return 0;
}

INT MtAsicRTSOnOff(struct _RTMP_ADAPTER *ad,UCHAR band_idx, UINT32 rts_num, UINT32 rts_len, BOOLEAN rts_en)
{
	UINT32 cr;
	UINT32 value;
	/*adjust cts/rts rate*/
	cr = (band_idx == BAND1) ? TMAC_PCR1 : TMAC_PCR;
	value = (rts_en == TRUE) ? TMAC_PCR_AUTO_RATE : TMAC_PCR_FIX_OFDM_6M_RATE;
	MAC_IO_WRITE32(ad,cr,value);
	/*adjust rts rts threshold*/
	cr = (band_idx == BAND1) ? AGG_PCR2 : AGG_PCR1;
	value = RTS_THRESHOLD(rts_len) | RTS_PKT_NUM_THRESHOLD(rts_num);
	MAC_IO_WRITE32(ad,cr,value);
	return 0;
}
