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

#include "rt_config.h"

extern const UCHAR wmm_aci_2_hw_ac_queue[18] ;

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
	UINT32	msdr16;
	MAC_IO_READ32(pAd, MIB_MSDR16, &msdr16);
	msdr16 &= 0x00ffffff;
	return msdr16;
}


#ifdef CONFIG_STA_SUPPORT
VOID MtAsicUpdateAutoFallBackTable(RTMP_ADAPTER *pAd, UCHAR *pRateTable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}

INT32 MtAsicForceWakeUpHdlr(IN PRTMP_ADAPTER pAd)
{
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: --->\n", __FUNCTION__));
		AsicForceWakeup(pAd, TRUE);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s:: <---\n", __FUNCTION__));
	}

	return 0;
}

INT32 MtAsicForceSleepAutoWakeupHdlr(IN PRTMP_ADAPTER pAd)
{
	USHORT  TbttNumToNextWakeUp;
	USHORT  NextDtim = pAd->StaCfg.DtimPeriod;
	ULONG   Now;

	NdisGetSystemUpTime(&Now);
	NextDtim -= (USHORT)(Now - pAd->StaCfg.LastBeaconRxTime)/pAd->CommonCfg.BeaconPeriod;

	TbttNumToNextWakeUp = pAd->StaCfg.DefaultListenCount;
	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_RECEIVE_DTIM) && (TbttNumToNextWakeUp > NextDtim))
		TbttNumToNextWakeUp = NextDtim;

	/* if WMM-APSD is failed, try to disable following line*/
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(line=%d): -->\n", __FUNCTION__, __LINE__));
	AsicSleepThenAutoWakeup(pAd, TbttNumToNextWakeUp);

	return 0;
}

#endif /* CONFIG_STA_SUPPORT */


INT MtAsicSetAutoFallBack(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
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
	Value |= RATE3_UP_MPDU_LIMIT(0);
	Value &= ~RATE4_UP_MPDU_LIMIT_MASK;
	Value |= RATE4_UP_MPDU_LIMIT(0);
	Value &= ~RATE5_UP_MPDU_LIMIT_MASK;
	Value |= RATE5_UP_MPDU_LIMIT(0);
	Value &= ~RATE6_UP_MPDU_LIMIT_MASK;
	Value |= RATE6_UP_MPDU_LIMIT(0);
	Value &= ~RATE7_UP_MPDU_LIMIT_MASK;
	Value |= RATE7_UP_MPDU_LIMIT(0);
	Value &= ~RATE8_UP_MPDU_LIMIT_MASK;
	Value |= RATE8_UP_MPDU_LIMIT(0);
	MAC_IO_WRITE32(pAd, AGG_ARUCR, Value);

	MAC_IO_READ32(pAd, AGG_ARDCR, &Value);
	Value &= ~RATE1_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE1_DOWN_MPDU_LIMIT(2);
	Value &= ~RATE2_DOWN_MPDU_LIMIT_MASK;
	Value |= RATE2_DOWN_MPDU_LIMIT(2);
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
	Value &= ~RTS_RATE_DOWN_TH_MASK;
	Value &= ~RATE_DOWN_EXTRA_RATIO_MASK;
	Value |= RATE_DOWN_EXTRA_RATIO(1);
	Value |= RATE_DOWN_EXTRA_RATIO_EN;
	Value &= ~RATE_UP_EXTRA_TH_MASK;
	Value |= RATE_UP_EXTRA_TH(4);
	MAC_IO_WRITE32(pAd, AGG_ARCR, Value);

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
// TODO: shiang-usw, refine this to remove some system-dependent checking/assignment
VOID MtAsicUpdateProtect(
	IN PRTMP_ADAPTER pAd,
	IN USHORT OperationMode,
	IN UCHAR SetMask,
	IN BOOLEAN bDisableBGProtect,
	IN BOOLEAN bNonGFExist)
{
	UINT32 Value = 0;

#ifdef CONFIG_ATE
	if (ATE_ON(pAd))
		return;
#endif /* CONFIG_ATE */

#ifdef DOT11_N_SUPPORT
	if (!(pAd->CommonCfg.bHTProtect) && (OperationMode != 8))
		return;

    if (pAd->CommonCfg.ManualProtection == 1)
        return;
#endif /* DOT11_N_SUPPORT */

	/* Config ASIC RTS threshold register*/
	MAC_IO_READ32(pAd, AGG_PCR1, &Value);
	Value &= ~RTS_THRESHOLD_MASK;
    Value &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	if ((
#ifdef DOT11_N_SUPPORT
		(pAd->CommonCfg.BACapability.field.AmsduEnable) ||
#endif /* DOT11_N_SUPPORT */
		(pAd->bDisableRtsProtect == TRUE))
		&& (pAd->CommonCfg.RtsThreshold == MAX_RTS_THRESHOLD))
	{
		Value |= RTS_THRESHOLD(0xFFFFF);
		Value |= RTS_PKT_NUM_THRESHOLD(0x7F);
	}
	else
	{
		Value |= RTS_THRESHOLD(pAd->CommonCfg.RtsThreshold);
		Value |= RTS_PKT_NUM_THRESHOLD(1);
	}

	MAC_IO_WRITE32(pAd, AGG_PCR1, Value);

	/* Handle legacy(B/G) protection*/
	if (bDisableBGProtect)
    {
		MAC_IO_READ32(pAd, AGG_PCR, &Value);
		Value &= ~ERP_PROTECTION_MASK;
		MAC_IO_WRITE32(pAd, AGG_PCR, Value);
    	pAd->FlgCtsEnabled = 0; /* CTS-self is not used */
	}
    else
	{
	    pAd->FlgCtsEnabled = 1; /* CTS-self is used */
    }

	MAC_IO_READ32(pAd, AGG_PCR, &Value);

	if (!RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RDG_ACTIVE))
	{ /* Single Protection Mode */
		Value |= PROTECTION_MODE;
	}
	else
	{
		Value &= ~PROTECTION_MODE;
	}


	MAC_IO_WRITE32(pAd, AGG_PCR, Value);


#ifdef DOT11_N_SUPPORT
	/* Decide HT frame protection.*/
	if ((SetMask & ALLN_SETPROTECT) != 0)
	{
		switch(OperationMode)
		{
			case 0x0:
				/* NO PROTECT */
				/* 1.All STAs in the BSS are 20/40 MHz HT*/
				/* 2. in ai 20/40MHz BSS*/
				/* 3. all STAs are 20MHz in a 20MHz BSS*/
				/* Pure HT. no protection.*/
				MAC_IO_READ32(pAd, AGG_PCR, &Value);
				Value &= ~(MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION
							| RIFS_PROTECTION | BW80_PROTECTION | BW160_PROTECTION);

				MAC_IO_WRITE32(pAd, AGG_PCR, Value);
				break;
 			case 0x1:
				/* This is "HT non-member protection mode." */
				/* If there may be non-HT STAs my BSS*/
				MAC_IO_READ32(pAd, AGG_PCR, &Value);
				Value &= ~(MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION
							| RIFS_PROTECTION | BW80_PROTECTION | BW160_PROTECTION);

				Value |= (MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION);

				MAC_IO_WRITE32(pAd, AGG_PCR, Value);
				break;
			case 0x2:
				/* If only HT STAs are in BSS. at least one is 20MHz. Only protect 40MHz packets */
				MAC_IO_READ32(pAd, AGG_PCR, &Value);
				Value &= ~(MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION
							| RIFS_PROTECTION | BW80_PROTECTION | BW160_PROTECTION);

				Value |= (BW40_PROTECTION);

				MAC_IO_WRITE32(pAd, AGG_PCR, Value);
				break;

			case 0x3:
				/* HT mixed mode. PROTECT ALL!*/
				/* both 20MHz and 40MHz are protected. Whether use RTS or CTS-to-self depends on the */
				MAC_IO_READ32(pAd, AGG_PCR, &Value);
				Value &= ~(MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION
							| RIFS_PROTECTION | BW80_PROTECTION | BW160_PROTECTION);

				Value |= (MM_PROTECTION | GF_PROTECTION | BW40_PROTECTION);

				MAC_IO_WRITE32(pAd, AGG_PCR, Value);
				pAd->CommonCfg.IOTestParm.bRTSLongProtOn = TRUE;
				break;

            case 0x8:
				// TODO...
                break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: unknown protection mode(%d)\n", __FUNCTION__, OperationMode));
		}
	}
#endif /* DOT11_N_SUPPORT */

	return;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicSwitchChannel(RTMP_ADAPTER *pAd, UCHAR Channel, BOOLEAN bScan)
{
	UINT32 val;

	// TODO: shiang-usw, unify the pAd->chipOps
	if (pAd->chipOps.ChipSwitchChannel)
		pAd->chipOps.ChipSwitchChannel(pAd, Channel, bScan);
	else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("For this chip, no specified channel switch function!\n"));

	// TODO: shiang-7603
	MAC_IO_READ32(pAd, RMAC_CHFREQ, &val);
	val = 1;
	MAC_IO_WRITE32(pAd, RMAC_CHFREQ, val);
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */


VOID MtAsicResetBBPAgent(RTMP_ADAPTER *pAd)
{
	/* Still need to find why BBP agent keeps busy, but in fact, hardware still function ok. Now clear busy first.	*/
	/* IF chipOps.AsicResetBbpAgent == NULL, run "else" part */
	// TODO: shiang-usw, unify the pAd->chipOps
	if (pAd->chipOps.AsicResetBbpAgent != NULL)
		pAd->chipOps.AsicResetBbpAgent(pAd);
}



/*
	==========================================================================
	Description:
		Set My BSSID

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
 /* CFG_TODO */
VOID MtAsicSetBssid(RTMP_ADAPTER *pAd, UCHAR *pBssid, UCHAR curr_bssid_idx)
{
	UINT32 val;
	// TODO: shiang-7603, now only configure Current BSSID Address 0
	ULONG curr_bssid_reg_base = RMAC_CB0R0;//register for Current_Bssid from 0x60140004

	val = (UINT32)((pBssid[0]) |
				  (UINT32)(pBssid[1] << 8) |
				  (UINT32)(pBssid[2] << 16) |
				  (UINT32)(pBssid[3] << 24));
	MAC_IO_WRITE32(pAd, curr_bssid_reg_base + (curr_bssid_idx * 8), val);

	val = (UINT32)(pBssid[4]) | (UINT32)(pBssid[5] << 8) | (1 <<16);
	MAC_IO_WRITE32(pAd, (curr_bssid_reg_base + 4) + (curr_bssid_idx * 8), val);

	// If we enable BSSID0, we should not enable MBSS0, or the BSSID index will wrong
	//MAC_IO_READ32(pAd, RMAC_ACBEN, &val);
	//val |= 0x1;
	//MAC_IO_WRITE32(pAd, RMAC_ACBEN, val);

	return;
}




INT MtAsicSetDevMac(RTMP_ADAPTER *pAd, UCHAR *addr, UCHAR omac_idx)
{
	UINT32 val;
	ULONG own_mac_reg_base = RMAC_OMA0R0;//register for Own_Mac from 0x60140024

	val = (addr[0]) | (addr[1]<<8) |  (addr[2]<<16) | (addr[3]<<24);
	MAC_IO_WRITE32(pAd, own_mac_reg_base + (omac_idx * 8), val);

	val = addr[4] | (addr[5]<<8) |  (1 <<16);
	MAC_IO_WRITE32(pAd, (own_mac_reg_base + 4) + (omac_idx * 8), val);

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
// TODO: shiang-usw, refine this to move system-dependent code!
VOID MtAsicSetMbssMode(RTMP_ADAPTER *pAd, UCHAR NumOfBcns)
{
	UCHAR NumOfMacs;
	UINT32 regValue;

	/*
		Note:
			1.The MAC address of Mesh and AP-Client link are different from Main BSSID.
			2.If the Mesh link is included, its MAC address shall follow the last MBSSID's MAC by increasing 1.
			3.If the AP-Client link is included, its MAC address shall follow the Mesh interface MAC by increasing 1.
	*/
	NumOfMacs = pAd->ApCfg.BssidNum + MAX_MESH_NUM + MAX_APCLI_NUM;

	/* set Multiple BSSID mode */
	if (NumOfMacs <= 1)
	{
		pAd->ApCfg.MacMask = ~(1-1);
	}
	else if (NumOfMacs <= 2)
	{
		pAd->ApCfg.MacMask = ~(2-1);
	}
	else if (NumOfMacs <= 4)
	{
		pAd->ApCfg.MacMask = ~(4-1);
	}
	else if (NumOfMacs <= 8)
	{
		pAd->ApCfg.MacMask = ~(8-1);
	}
	else if (NumOfMacs <= 16)
	{
		/* Set MULTI_BSSID_MODE_BIT4 in MAC register 0x1014 */
		pAd->ApCfg.MacMask = ~(16-1);
	}

	// TODO: shiang-7603
	MAC_IO_READ32(pAd, RMAC_ACBEN, &regValue);
	regValue |=  ((1 << NumOfMacs) - 1);
	// TODO: shiang-MT7603, for MBSS0, now we use BSS0 instead of MBSS0, or the BSSID Index will be mismatch!
	regValue &= (~0x01);
	MAC_IO_WRITE32(pAd, RMAC_ACBEN, regValue);

}
#endif /* CONFIG_AP_SUPPORT */


#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
INT MtAsicSetMacAddrExt(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	RMAC_MORE_STRUC rmac_more;

	MAC_IO_READ32(pAd, RMAC_MORE, &rmac_more.word);
	if (bEnable == 0)
		rmac_more.field.muar_mode_sel = 0;
	else
		rmac_more.field.muar_mode_sel = 1;
	MAC_IO_WRITE32(pAd, RMAC_MORE, rmac_more.word);

	return TRUE;
}


VOID MtAsicInsertRepeaterEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR CliIdx,
	IN PUCHAR pAddr)
{
	UCHAR tempMAC[MAC_ADDR_LEN];
        RMAC_MAR0_STRUC rmac_mcbcs0;
        RMAC_MAR1_STRUC rmac_mcbcs1;

	COPY_MAC_ADDR(tempMAC, pAddr);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("\n%s %02x:%02x:%02x:%02x:%02x:%02x-%02x\n",
			__FUNCTION__, PRINT_MAC(tempMAC), CliIdx));

	NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
	rmac_mcbcs0.addr_31_0 = tempMAC[0] + (tempMAC[1] << 8) +(tempMAC[2] << 16) +(tempMAC[3] << 24);
	MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

	NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
	rmac_mcbcs1.field.addr_39_32 = tempMAC[4];
	rmac_mcbcs1.field.addr_47_40 = tempMAC[5];
	rmac_mcbcs1.field.access_start = 1;
	rmac_mcbcs1.field.readwrite = 1;
#ifdef MT7628
    if (CliIdx > 15) //entry is in 16~23
    {
            CliIdx = (CliIdx - 16) + 32;
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("CliIdx correct to %02x\n",
                    CliIdx));
    }
#endif

	rmac_mcbcs1.field.multicast_addr_index = CliIdx;
	MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);

	return;
}


VOID MtAsicRemoveRepeaterEntry(RTMP_ADAPTER *pAd, UCHAR CliIdx)
{
    RMAC_MAR0_STRUC rmac_mcbcs0;
    RMAC_MAR1_STRUC rmac_mcbcs1;
    UCHAR temp_idx;

	//TODO: Carter, not finish yet!
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN, (" %s.\n", __FUNCTION__));

    NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
    MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

    NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
    rmac_mcbcs1.field.access_start = 1;
    rmac_mcbcs1.field.readwrite = 1;

    if (CliIdx <= 15)
        rmac_mcbcs1.field.multicast_addr_index = CliIdx;//start from idx 0
#ifdef MT7628
    else {
        temp_idx = (CliIdx - 16) + 0x20;//start from idx 32
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nReptCliIdx correct to %02x\n",
            temp_idx));
        rmac_mcbcs1.field.multicast_addr_index = temp_idx;
    }
#endif

    MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);//clear client entry first.

    NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
    MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

    NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
    rmac_mcbcs1.field.access_start = 1;
    rmac_mcbcs1.field.readwrite = 1;

    if (CliIdx <= 15)
        rmac_mcbcs1.field.multicast_addr_index = 0x10 + CliIdx;//start from idx 16
#ifdef MT7628
    else {
        temp_idx = (CliIdx - 16) + 0x28;//start from idx 40
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nReptCliIdx correct to %02x\n",
            temp_idx));
        rmac_mcbcs1.field.multicast_addr_index = temp_idx;
    }
#endif
    MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);//clear rootap entry.
}


void insert_repeater_root_entry(
	IN PRTMP_ADAPTER pAd,
	IN MAC_TABLE_ENTRY *pEntry,
	IN  UCHAR *pAddr,
	IN UCHAR ReptCliIdx)
{
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	RMAC_MAR0_STRUC rmac_mcbcs0;
	RMAC_MAR1_STRUC rmac_mcbcs1;

	// TODO: shiang-usw, remove the pEntry and use wcid directly!
	if (pEntry)
	{
		//STA_TR_ENTRY *tr_entry = &pAd->MacTab.tr_entry[pEntry->wcid];
		//tr_entry->PortSecured = WPA_802_1X_PORT_SECURED;
		//pEntry->AuthState = AS_AUTH_OPEN;//TODO, Carter, check this.
		//pEntry->Sst = SST_ASSOC;

		tb_entry.wtbl_addr[0] = wtbl_ctrl->wtbl_base_addr[0] +
							pEntry->wcid * wtbl_ctrl->wtbl_entry_size[0];

		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0], &tb_entry.wtbl_1.wtbl_1_d0.word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 4, &tb_entry.wtbl_1.wtbl_1_d1.word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 8, &tb_entry.wtbl_1.wtbl_1_d2.word);
		dw0->field.wm = 0;
		dw0->field.muar_idx = 0x20 + ReptCliIdx;
		dw0->field.rc_a1 = 1;
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, ReptCliIdx, tb_entry.wtbl_addr[0], dw0->word));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, ReptCliIdx,  tb_entry.wtbl_addr[0] + 4, dw1->word));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, ReptCliIdx, tb_entry.wtbl_addr[0] + 8, dw2->word));

		NdisZeroMemory(&rmac_mcbcs0, sizeof(RMAC_MAR0_STRUC));
		rmac_mcbcs0.addr_31_0 = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
		MAC_IO_WRITE32(pAd, RMAC_MAR0, rmac_mcbcs0.addr_31_0);

		NdisZeroMemory(&rmac_mcbcs1, sizeof(RMAC_MAR1_STRUC));
		rmac_mcbcs1.field.addr_39_32 = pAddr[4];
		rmac_mcbcs1.field.addr_47_40 = pAddr[5];
		rmac_mcbcs1.field.access_start = 1;
		rmac_mcbcs1.field.readwrite = 1;

        if (ReptCliIdx <= 15)
            rmac_mcbcs1.field.multicast_addr_index = 0x10 + ReptCliIdx;//start from idx 16
#ifdef MT7628
        if (ReptCliIdx > 15) //entry is in 16~23
        {
            ReptCliIdx = (ReptCliIdx - 16) + 0x28;
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\nReptCliIdx correct to %02x\n",
                ReptCliIdx));
            rmac_mcbcs1.field.multicast_addr_index = ReptCliIdx;//start from idx 40
        }
#endif
		MAC_IO_WRITE32(pAd, RMAC_MAR1, rmac_mcbcs1.word);
	}
}
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */


INT MtAsicSetRxFilter(RTMP_ADAPTER *pAd)
{
	UINT32 Value;

//	MAC_IO_READ32(pAd, RMAC_RFCR, &Value);
	#define MT_RXFILTER_NORMAL	 0x001FEF9A
	Value = MT_RXFILTER_NORMAL;
	Value &= ~RM_FRAME_REPORT_EN;
	Value &= ~DROP_NOT_MY_BSSID;
	Value &= ~DROP_NOT_IN_MC_TABLE;
#if defined(MT7628_FPGA)
	Value = 0x1002;
#endif
	/*disable RX diff BSSID BCN filter */
	Value &= ~DROP_DIFF_BSSID_BCN;
	/*disable RX PROB request filter */
	Value &= ~DROP_PROBE_REQ;
	/* Disable Rx Duplicate Packet Drop filter */
	Value &= ~DROP_DUPLICATE;


#ifdef CONFIG_SNIFFER_SUPPORT
	if ((MONITOR_ON(pAd)) && pAd->monitor_ctrl.CurrentMonitorMode == MONITOR_MODE_FULL) /* Enable Rx with promiscuous reception */
		Value = 0x3;
#endif


	MAC_IO_WRITE32(pAd, RMAC_RFCR, Value);
	MAC_IO_WRITE32(pAd, RMAC_RFCR1, 0);
	return TRUE;
}


#ifdef DOT11_N_SUPPORT
// TODO: shiang-usw, fix me for windows compatible!
INT MtAsicWtblSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
    UINT8 wcid;
    MAC_TABLE *pMacTable;
    MAC_TABLE_ENTRY *pEntry;
    struct wtbl_entry tb_entry;
    union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;

    pMacTable = &pAd->MacTab;
    NdisZeroMemory(&tb_entry, sizeof(tb_entry));

    /* Search for the Ralink STA */
    for (wcid = 1; wcid < MAX_LEN_OF_MAC_TABLE; wcid++)
    {
        pEntry = &pMacTable->Content[wcid];

        if (CLIENT_STATUS_TEST_FLAG(pEntry, fCLIENT_STATUS_RALINK_CHIPSET))
        {
            if (mt_wtbl_get_entry234(pAd, wcid, &tb_entry) == FALSE)
            {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
                            __FUNCTION__, wcid));
                return FALSE;
            }

            HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), &dw2->word);
            if (bEnable)
            {
                dw2->field.r = 1;
                dw2->field.rdg_ba = 1;
            }
            else
            {
                dw2->field.r = 0;
                dw2->field.rdg_ba = 0;
            }
            HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + (2 * 4), dw2->word);
        }
    }

    return TRUE;
}


INT MtAsicSetRDG(RTMP_ADAPTER *pAd, BOOLEAN bEnable)
{
	UINT32 tmac_tcr, agg_pcr, tmac_trcr;

	MAC_IO_READ32(pAd, TMAC_TCR, &tmac_tcr);
	MAC_IO_READ32(pAd, AGG_PCR, &agg_pcr);
    MAC_IO_READ32(pAd, TMAC_TRCR, &tmac_trcr);

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

    //WHQA_00018120
    if (MTK_REV_GTE(pAd, MT7628, MT7628E2))
        tmac_trcr = tmac_trcr | I2T_CHK_EN;

	MAC_IO_WRITE32(pAd, TMAC_TCR, tmac_tcr);
	MAC_IO_WRITE32(pAd, AGG_PCR, agg_pcr);
	MAC_IO_WRITE32(pAd, TMAC_TRCR, tmac_trcr);

	MtAsicWtblSetRDG(pAd, bEnable);

	return TRUE;
}
#endif /* DOT11_N_SUPPORT */


/*
    ========================================================================
    Routine Description:
        Set/reset MAC registers according to bPiggyBack parameter

    Arguments:
        pAd         - Adapter pointer
        bPiggyBack  - Enable / Disable Piggy-Back

    Return Value:
        None

    ========================================================================
*/
VOID MtAsicSetPiggyBack(RTMP_ADAPTER *pAd, BOOLEAN bPiggyBack)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}


#define INT_TIMER_EN_PRE_TBTT	0x1
#define INT_TIMER_EN_GP_TIMER	0x2
static INT MtAsicSetIntTimerEn(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 type, UINT32 timeout)
{
//	UINT32 mask, time_mask;
//	UINT32 Value;

return 0;
}


INT MtAsicSetPreTbtt(RTMP_ADAPTER *pAd, BOOLEAN enable, UCHAR idx)
{
	UINT32 timeout = 0, bitmask = 0;
	INT bss_idx = 0; // TODO: this index may carried by parameters!

	bss_idx=idx;

	ASSERT(bss_idx <= 3);
    bitmask = 0xff << (bss_idx * 8);

	if (enable == TRUE) {
		/*
		   each HW BSSID has its own PreTBTT interval,
		   unit is 64us, 0x00~0xff is configurable.
		   Base on RTMP chip experience,
		   Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		*/


		MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		timeout |= (0xf0 << (bss_idx * 8));//Carter, 20140710, sync 7603
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): bss_idx=%d, PreTBTT timeout = 0x%x\n",
					__FUNCTION__, bss_idx, timeout));
	}
    else {
        MAC_IO_READ32(pAd, LPON_PISR, &timeout);
		timeout &= (~bitmask);
		MAC_IO_WRITE32(pAd, LPON_PISR, timeout);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): bss_idx=%d, PreTBTT timeout = 0x%x\n",
					__FUNCTION__, bss_idx, timeout));
    }

	return TRUE;
}


INT MtAsicSetGPTimer(RTMP_ADAPTER *pAd, BOOLEAN enable, UINT32 timeout)
{
	return MtAsicSetIntTimerEn(pAd, enable, INT_TIMER_EN_GP_TIMER, timeout);
}


INT MtAsicSetChBusyStat(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));

	return TRUE;
}


INT MtAsicGetTsfTime(RTMP_ADAPTER *pAd, UINT32 *high_part, UINT32 *low_part)
{
    UINT32 Value = 0;

    MAC_IO_READ32(pAd, LPON_T0CR, &Value);
    Value = (Value & 0xc) | TSF_TIMER_VALUE_READ;//keep HW mode value.
    //Value = Value | TSF_TIMER_VALUE_READ;
	MAC_IO_WRITE32(pAd, LPON_T0CR, Value);

	MAC_IO_READ32(pAd, LPON_UTTR0, low_part);
	MAC_IO_READ32(pAd, LPON_UTTR1, high_part);

	return TRUE;
}


/*
	==========================================================================
	Description:

	IRQL = PASSIVE_LEVEL
	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicDisableSync(RTMP_ADAPTER *pAd)
{
	UINT32  value = 0;

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->Disable TSF synchronization\n"));

    pAd->TbttTickCount = 0;

#ifdef CONFIG_AP_SUPPORT
	//INT32 IdBss, MaxNumBss = pAd->ApCfg.BssidNum;
	//BSS_STRUCT *pMbss;

	/* Disable PreTBTT/TBTT interrupt for inform host */
	value = 0;
	MAC_IO_WRITE32(pAd, HWIER3, value);

	return;
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
    /* Configure Beacon interval */
	MAC_IO_WRITE32(pAd, LPON_T0TPCR, value);

	/* Clear Pre-TBTT Trigger, and calcuate next TBTT timer by HW*/
	//enable PRETBTT0INT_EN, PRETBTT0TIMEUP_EN
	//and TBTT0PERIODTIMER_EN, TBTT0TIMEUP_EN
	MAC_IO_WRITE32(pAd, LPON_MPTCR1, 0x99);//TODO: TBTT1, TBTT2.

	/* Enable interrupt */
	value = 0;
	MAC_IO_WRITE32(pAd, HWIER3, value);

	/* Config BCN/BMC timoeut, or the normal Tx wil be blocked forever if no beacon frame in Queue */
	//Value = 0x01800180;
	//MAC_IO_WRITE32(pAd, LPON_BCNTR, mac_val);

	/* Configure Beacon Queue Operation mode */
	MAC_IO_READ32(pAd, ARB_SCR, &value);
	value &= 0xfffffffc;//Clear bit[0:1] for MBSSID 0
	MAC_IO_WRITE32(pAd, ARB_SCR, value);

	/* Clear Beacon Queue */
	value = 0x1;
	MAC_IO_WRITE32(pAd, ARB_BCNQCR1, value);

    return;
#endif /* CONFIG_STA_SUPPORT */

}


VOID MtAsicEnableBssSync(RTMP_ADAPTER *pAd, MT_BSS_SYNC_CTRL_T BssSync)
{
	UINT32 bitmask = 0;
	UINT32 Value=0;
	UINT32 RegLponTpcr=0,RegMpTcr=0;
	UINT32 MptcrValue=0,BcnQValue=0,BcnOpMode=0;

    	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("--->%s():\n", __FUNCTION__));

	ASSERT(BssSync.BssSet <= 3);

	if(BssSync.BssOpMode==MT_BSS_MODE_STA)
	{

		/* Configure Beacon interval */
		Value |= BEACONPERIODn(BssSync.BeaconPeriod);
		/* Configure DTIM interval */
		Value |= DTIMPERIODn(BssSync.DtimPeriod);

		Value |= TBTTn_CAL_EN;
		RTMP_IO_WRITE32(pAd, LPON_T0TPCR, Value);

	}else
	{

		/*Decision HW index by BSS_AP_TYPE*/
		BcnQValue = 1 << BssSync.BssSet;
		BcnOpMode = (BssSync.BssOpMode==MT_BSS_MODE_AP) ? BCNQ_OP_MODE_AP : BCNQ_OP_MODE_ADHOC;

		MptcrValue = TBTT_TIMEUP_EN |TBTT_PERIOD_TIMER_EN | PRETBTT_TIMEUP_EN | PRETBTT_INT_EN;

		switch(BssSync.BssSet){
		case 0:
			RegLponTpcr = LPON_T0TPCR;
			RegMpTcr = LPON_MPTCR0;
		break;
		case 1:
			RegLponTpcr = LPON_T0TPCR+4;
			RegMpTcr = LPON_MPTCR0;
			MptcrValue    = MptcrValue << 8;
		break;
		case 2:
			RegLponTpcr = LPON_T2TPCR;
			RegMpTcr = LPON_MPTCR2;
		break;
		case 3:
			RegLponTpcr = LPON_T2TPCR+4;
			RegMpTcr = LPON_MPTCR2;
			MptcrValue    = MptcrValue << 8;
		break;
		default:
		break;
		}



		/* Configure Beacon interval */
		MAC_IO_READ32(pAd, RegLponTpcr, &Value);
		Value = 0;
		Value &= ~BEACONPERIODn_MASK;
		Value |= BEACONPERIODn(BssSync.BeaconPeriod);
		Value |= TBTTn_CAL_EN;
		MAC_IO_WRITE32(pAd, RegLponTpcr, Value);

		/* Enable Pre-TBTT Trigger, and calcuate next TBTT timer by HW*/
		//enable PRETBTT0INT_EN, PRETBTT0TIMEUP_EN
		//and TBTT0PERIODTIMER_EN, TBTT0TIMEUP_EN
		MAC_IO_WRITE32(pAd, RegMpTcr, MptcrValue);//TODO: TBTT1, TBTT2.

		/* Set Pre-TBTT interval */
		/*
		   each HW BSSID has its own PreTBTT interval,
		   unit is 64us, 0x00~0xff is configurable.
		   Base on RTMP chip experience,
		   Pre-TBTT is 6ms before TBTT interrupt. 1~10 ms is reasonable.
		*/
		bitmask = 0xff << (BssSync.BssSet* 8);
		MAC_IO_READ32(pAd, LPON_PISR, &Value);
		Value &= (~bitmask);
		Value |= (BssSync.PreTbttInterval << (BssSync.BssSet * 8));

		MAC_IO_WRITE32(pAd, LPON_PISR, Value);

		/* Enable interrupt */
		MAC_IO_READ32(pAd, HWIER3, &Value);
		Value = 0;
		Value |= TBTT0;
		Value |= PRETBTT0;
		MAC_IO_WRITE32(pAd, HWIER3, Value);

		/* Config BCN/BMC timoeut, or the normal Tx wil be blocked forever if no beacon frame in Queue */
		//Value = 0x01800180;
		//MAC_IO_WRITE32(pAd, LPON_BCNTR, mac_val);

		/* Configure Beacon Queue Operation mode */
		MAC_IO_READ32(pAd, ARB_SCR, &Value);

		if(BssSync.BssOpMode==BCNQ_OP_MODE_AP)
		{
			Value &= (~(BIT30)); //make BCN need to content with other ACs
			Value |= BIT31;
		}

		Value |= BcnOpMode;//TODO, Carter, when use other HWBSSID, shall could choose index to set correcorresponding bit.
	   	 //Value = Value & 0xefffffff;
		MAC_IO_WRITE32(pAd, ARB_SCR, Value);

		/* Start Beacon Queue */
		MAC_IO_READ32(pAd, ARB_BCNQCR0, &Value);
		Value |= BcnQValue;
		MAC_IO_WRITE32(pAd, ARB_BCNQCR0, Value);
	}
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
	{ARB_AIFSR0, 0x0000000f, 0}, /* AC0 - BK */
	{ARB_AIFSR0, 0x000000f0, 4}, /* AC1 - BE */
	{ARB_AIFSR0, 0x00000f00, 8}, /* AC2  - VI */
	{ARB_AIFSR0, 0x0000f000, 12}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmin_mask[] = {
	{ARB_ACCWIR0, 0x000000ff, 0}, /* AC0 - BK */
	{ARB_ACCWIR0, 0x0000ff00, 8}, /* AC1 - BE */
	{ARB_ACCWIR0, 0x00ff0000, 16}, /* AC2  - VI */
	{ARB_ACCWIR0, 0xff000000, 24}, /* AC3 - VO */
};

static RTMP_WMM_PAIR wmm_cwmax_mask[] = {
	{ARB_ACCWXR0, 0x0000ffff, 0}, /* AC0 - BK */
	{ARB_ACCWXR0, 0xffff0000, 16}, /* AC1 - BE */
	{ARB_ACCWXR1, 0x0000ffff, 0}, /* AC2  - VI */
	{ARB_ACCWXR1, 0xffff0000, 16}, /* AC3 - VO */
};


UINT32 MtAsicGetWmmParam(RTMP_ADAPTER *pAd, UINT32 ac, UINT32 type)
{
	UINT32 addr = 0, cr_val, mask = 0, shift = 0;

	if (ac <= WMM_PARAM_AC_3)
	{
		switch (type)
        {
			case WMM_PARAM_TXOP:
				addr = wmm_txop_mask[ac].Address;
				mask = wmm_txop_mask[ac].Mask;
				shift = wmm_txop_mask[ac].Shift;
				break;
			case WMM_PARAM_AIFSN:
				addr = wmm_aifsn_mask[ac].Address;
				mask = wmm_aifsn_mask[ac].Mask;
				shift = wmm_aifsn_mask[ac].Shift;
				break;
			case WMM_PARAM_CWMIN:
				addr = wmm_cwmin_mask[ac].Address;
				mask = wmm_cwmin_mask[ac].Mask;
				shift = wmm_cwmin_mask[ac].Shift;
				break;
			case WMM_PARAM_CWMAX:
				addr = wmm_cwmax_mask[ac].Address;
				mask = wmm_cwmax_mask[ac].Mask;
				shift = wmm_cwmax_mask[ac].Shift;
				break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __FUNCTION__, __LINE__, type));
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


 INT MtAsicSetWmmParam(RTMP_ADAPTER *pAd, UINT ac, UINT type, UINT val)
{
	CMD_EDCA_SET_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;

	NdisZeroMemory(&EdcaParam,sizeof(CMD_EDCA_SET_T));
	EdcaParam.ucTotalNum = 1;
	pAcParam = &EdcaParam.rAcParam[0];
	pAcParam->ucAcNum = (UINT8)ac;

	switch (type) {
	case WMM_PARAM_TXOP:
		pAcParam->ucVaildBit = CMD_EDCA_TXOP_BIT;
		pAcParam->u2Txop= (UINT16)val;
		break;
	case WMM_PARAM_AIFSN:
		pAcParam->ucVaildBit = CMD_EDCA_AIFS_BIT;
		pAcParam->ucAifs = (UINT8)val;
		break;
	case WMM_PARAM_CWMIN:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MIN_BIT;
		pAcParam->ucWinMin= (UINT8)val;
		break;
	case WMM_PARAM_CWMAX:
		pAcParam->ucVaildBit = CMD_EDCA_WIN_MAX_BIT;
		pAcParam->u2WinMax= (UINT16)val;
		break;
	default:
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Error type=%d\n", __FUNCTION__, __LINE__, type));
		break;
	}

	NdisCopyMemory(&pAd->CurrEdcaParam[ac], pAcParam, sizeof(TX_AC_PARAM_T));
	RTEnqueueInternalCmd(pAd, CMDTHREAD_EDCA_PARAM_SET, (VOID *)&EdcaParam, sizeof(CMD_EDCA_SET_T));
	return TRUE;
}


static INT MtAsicSetAllWmmParam(RTMP_ADAPTER *pAd,PEDCA_PARM pEdcaParm)
{
	CMD_EDCA_SET_T EdcaParam;
	P_TX_AC_PARAM_T pAcParam;
	UINT32 ac=0,index=0;;

	NdisZeroMemory(&EdcaParam,sizeof(CMD_EDCA_SET_T));
	EdcaParam.ucTotalNum = CMD_EDCA_AC_MAX;

	for ( ac=0; ac < CMD_EDCA_AC_MAX;  ac++)
	{
		index = wmm_aci_2_hw_ac_queue[ac];
		pAcParam = &EdcaParam.rAcParam[index];
		pAcParam->ucVaildBit = CMD_EDCA_ALL_BITS;
		pAcParam->ucAcNum = (UINT8)ac;
		pAcParam->ucAifs = pEdcaParm->Aifsn[index];
		pAcParam->ucWinMin= (1 << pEdcaParm->Cwmin[index]) -1;
		pAcParam->u2WinMax= (1 << pEdcaParm->Cwmax[index]) -1;
		pAcParam->u2Txop= pEdcaParm->Txop[index];

		NdisCopyMemory(&pAd->CurrEdcaParam[index], pAcParam, sizeof(TX_AC_PARAM_T));
	}

	RTEnqueueInternalCmd(pAd, CMDTHREAD_EDCA_PARAM_SET, (VOID *)&EdcaParam, sizeof(CMD_EDCA_SET_T));

	return TRUE;


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
	if ((pEdcaParm == NULL) || (pEdcaParm->bValid == FALSE))
	{
	}
	else
	{
		MtAsicSetAllWmmParam(pAd,pEdcaParm);
	}

	// TODO: shiang-MT7603, fix me after BurstMode is finished!
}


INT MtAsicSetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type, UINT32 limit)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


UINT32 MtAsicGetRetryLimit(RTMP_ADAPTER *pAd, UINT32 type)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
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
VOID MtAsicSetSlotTime(RTMP_ADAPTER *pAd, UINT32 SlotTime, UINT32 SifsTime)
{
	CmdSlotTimeSet(pAd,(UINT8)SlotTime,(UINT8)SifsTime,(UINT8)RIFS_TIME, EIFS_TIME);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): SetSlotTime!\n",__FUNCTION__, __LINE__));
}

#define MAX_RX_PKT_LENGTH   0x400 /* WORD(4 Bytes) unit */
INT MtAsicSetMacMaxLen(RTMP_ADAPTER *pAd)
{
	// TODO: shiang-7603
	UINT32 val;

	// Rx max packet length
	MAC_IO_READ32(pAd, DMA_DCR0, &val);
	val &= (~0xfffc);
	val |= (MAX_RX_PKT_LENGTH << 2);
	MAC_IO_WRITE32(pAd, DMA_DCR0, val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(%d): Set the Max RxPktLen=%d!\n",
				__FUNCTION__, __LINE__, MAX_RX_PKT_LENGTH));

	return TRUE;
}


#ifdef CONFIG_AP_SUPPORT
VOID MtAsicGetTxTsc(RTMP_ADAPTER *pAd, UCHAR apidx, UCHAR *pTxTsc)
{
	USHORT Wcid;
	struct wtbl_entry tb_entry;
	UINT32 addr = 0, val = 0;

	/* Sanity check of apidx */
	if (apidx >= MAX_MBSSID_NUM(pAd))
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): invalid apidx(%d)\n",
					__FUNCTION__, apidx));
		return;
	}

	GET_GroupKey_WCID(pAd, Wcid, apidx);
	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, Wcid, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, Wcid));
		return;
	}
	addr = pAd->mac_ctrl.wtbl_base_addr[1] + Wcid * pAd->mac_ctrl.wtbl_entry_size[1];
	HW_IO_READ32(pAd, addr, &val);

	*pTxTsc     = val & 0xff;
	*(pTxTsc+1) = (val >> 8) & 0xff;
	*(pTxTsc+2) = (val >> 16) & 0xff;
	*(pTxTsc+3) = (val >> 24) & 0xff;

	HW_IO_READ32(pAd, addr+4, &val);
	*(pTxTsc+4) = val & 0xff;
	*(pTxTsc+5) = (val >> 8) & 0xff;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(): WCID(%d) TxTsc 0x%02x-0x%02x-0x%02x-0x%02x-0x%02x-0x%02x \n",
				__FUNCTION__, Wcid,
				*pTxTsc, *(pTxTsc+1), *(pTxTsc+2), *(pTxTsc+3), *(pTxTsc+4), *(pTxTsc+5)));

	return;
}
#endif /* CONFIG_AP_SUPPORT */


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
	IN PRTMP_ADAPTER 	pAd,
	IN UCHAR		 	BssIndex,
	IN UCHAR		 	KeyIdx,
	IN PCIPHER_KEY		pCipherKey)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
						__FUNCTION__, __LINE__));
}


/*	IRQL = DISPATCH_LEVEL*/
VOID MtAsicRemoveSharedKeyEntry(
	IN PRTMP_ADAPTER pAd,
	IN UCHAR		 BssIndex,
	IN UCHAR		 KeyIdx)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
}


VOID MtAsicUpdateWCIDIVEIV(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		WCID,
	IN ULONG        uIV,
	IN ULONG        uEIV)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
}


/**
 * Wtbl2RateTableUpdate
 *
 *
 *
 */
VOID Wtbl2RateTableUpdate(RTMP_ADAPTER *pAd, UCHAR ucWcid, UINT32 u4Wtbl2D9, UINT32* Rate)
{
    UINT32 u4RegVal;
    UCHAR ucWaitCnt = 0;
#ifdef WTBL_RATE_DEBUG	
    UINT32 idx;
#endif
    do {
        MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &u4RegVal);
        if ((u4RegVal & 0x10000)  == 0)
            break;
        ucWaitCnt++;
        RtmpusecDelay(50);
    }while (ucWaitCnt < 100);

    if (ucWaitCnt == 100) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                __FUNCTION__, u4RegVal));
        return;
    }
#ifdef WTBL_RATE_DEBUG
	for (idx=0 ; idx<8; idx++)
	{
		UCHAR  phy_mode, rate;
		UINT32 raw_data;
		raw_data = Rate[idx] & 0xfff;
		phy_mode = (raw_data & 0x1c0) >> 6;
		rate = (raw_data & 0x3f);

		if (phy_mode >2 || (rate < 0) || (rate > 15))
		{
			printk("Invalid rate[%d] phy_mode = %d rate=%d wcid=%d\n",idx, phy_mode, rate, ucWcid);
		}
		
	}
#endif
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR0, u4Wtbl2D9);
	u4RegVal = (Rate[0] | (Rate[1] << 12) | (Rate[2] << 24));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR1, u4RegVal);
	u4RegVal = ((Rate[2] >> 8) | (Rate[3] << 4) | (Rate[4] << 16) | (Rate[5] << 28));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR2, u4RegVal);
	u4RegVal = ((Rate[5] >> 4) | (Rate[6] << 8) | (Rate[7] << 20));
	MAC_IO_WRITE32(pAd, WTBL_ON_RIUCR3, u4RegVal);

	// TODO: shiang-MT7603, shall we also clear TxCnt/RxCnt/AdmCnt here??
	u4RegVal = (ucWcid | (1 << 13) | (1 << 14));
	MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);

}


/**
 * Wtbl2TxRateCounterGet
 *
 *
 *
 */
VOID Wtbl2TxRateCounterGet(RTMP_ADAPTER *pAd, UCHAR ucWcid, TX_CNT_INFO *tx_cnt_info)
{
	UINT32 u4RegVal;
	UCHAR ucWaitCnt = 0;
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UCHAR wtbl_idx;
	UINT32 addr;

	wtbl_ctrl = &pAd->mac_ctrl;
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (ucWcid < wtbl_ctrl->wtbl_entry_cnt[0] ? ucWcid : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return;
	}

	addr = pAd->mac_ctrl.wtbl_base_addr[1] + wtbl_idx * pAd->mac_ctrl.wtbl_entry_size[1];
	HW_IO_READ32(pAd, addr + 5 * 4, &(tx_cnt_info->wtbl_2_d5.word));
	HW_IO_READ32(pAd, addr + 6 * 4, &(tx_cnt_info->wtbl_2_d6.word));
	HW_IO_READ32(pAd, addr + 7 * 4, &(tx_cnt_info->wtbl_2_d7.word));
	HW_IO_READ32(pAd, addr + 8 * 4, &(tx_cnt_info->wtbl_2_d8.word));
	HW_IO_READ32(pAd, addr + 9 * 4, &(tx_cnt_info->wtbl_2_d9.word));

	do {
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &u4RegVal);
		if ((u4RegVal & 0x10000)  == 0)
			break;
		ucWaitCnt++;
		RtmpusecDelay(50);
	}while (ucWaitCnt < 100);

	if (ucWaitCnt == 100) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
			__FUNCTION__, u4RegVal));
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
VOID Wtbl2RcpiGet(RTMP_ADAPTER *pAd, UCHAR ucWcid, union WTBL_2_DW13 *wtbl_2_d13)
{
	struct rtmp_mac_ctrl *wtbl_ctrl;
	UCHAR wtbl_idx;
	UINT32 addr;
	wtbl_ctrl = &pAd->mac_ctrl;
	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
		wtbl_idx = (ucWcid < wtbl_ctrl->wtbl_entry_cnt[0] ? ucWcid : wtbl_ctrl->wtbl_entry_cnt[0] - 1);
	else {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return;
	}

	addr = pAd->mac_ctrl.wtbl_base_addr[1] + wtbl_idx * pAd->mac_ctrl.wtbl_entry_size[1];
	HW_IO_READ32(pAd, addr + 13 * 4, &(wtbl_2_d13->word));
}


VOID MtAsicTxCntUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, MT_TX_COUNTER *pTxInfo)
{
	TX_CNT_INFO tx_cnt_info;
	UINT32 TxSuccess, TxRetransmit;

	if (!IS_VALID_ENTRY(pEntry))
		return;

	Wtbl2TxRateCounterGet(pAd, pEntry->wcid, &tx_cnt_info);

	pTxInfo->TxCount = tx_cnt_info.wtbl_2_d7.field.current_bw_tx_cnt;
	pTxInfo->TxCount += tx_cnt_info.wtbl_2_d8.field.other_bw_tx_cnt;
	pTxInfo->TxFailCount = tx_cnt_info.wtbl_2_d7.field.current_bw_fail_cnt;
	pTxInfo->TxFailCount += tx_cnt_info.wtbl_2_d8.field.other_bw_fail_cnt;

	pTxInfo->Rate1TxCnt = tx_cnt_info.wtbl_2_d5.field.rate_1_tx_cnt;
	pTxInfo->Rate1FailCnt = tx_cnt_info.wtbl_2_d5.field.rate_1_fail_cnt;
	pTxInfo->Rate2TxCnt = tx_cnt_info.wtbl_2_d6.field.rate_2_tx_cnt;
	pTxInfo->Rate3TxCnt = tx_cnt_info.wtbl_2_d6.field.rate_3_tx_cnt;
	pTxInfo->Rate4TxCnt = tx_cnt_info.wtbl_2_d6.field.rate_4_tx_cnt;
	pTxInfo->Rate5TxCnt = tx_cnt_info.wtbl_2_d6.field.rate_5_tx_cnt;

	pTxInfo->RateIndex = tx_cnt_info.wtbl_2_d9.field.rate_idx;

	TxSuccess = pTxInfo->TxCount - pTxInfo->TxFailCount;
	TxRetransmit = pTxInfo->TxFailCount;

	if ( pTxInfo->TxFailCount == 0 )
	{
		pAd->RalinkCounters.OneSecTxNoRetryOkCount += pTxInfo->TxCount;
		pAd->MacTab.Content[pEntry->wcid].OneSecTxNoRetryOkCount += pTxInfo->TxCount;
	}
	else
	{
		pAd->RalinkCounters.OneSecTxRetryOkCount += pTxInfo->TxCount;
		pAd->MacTab.Content[pEntry->wcid].OneSecTxRetryOkCount += pTxInfo->TxCount;
	}
	
	pAd->RalinkCounters.OneSecTxFailCount += pTxInfo->TxFailCount;
	pAd->MacTab.Content[pEntry->wcid].OneSecTxFailCount += pTxInfo->TxFailCount;

#ifdef STATS_COUNT_SUPPORT
	pAd->WlanCounters.TransmittedFragmentCount.u.LowPart += TxSuccess;
	pAd->WlanCounters.FailedCount.u.LowPart += pTxInfo->TxFailCount;
#endif /* STATS_COUNT_SUPPORT */

	if ((TxSuccess == 0) && (TxRetransmit > 0))
	{
		/* prevent fast drop long range clients */
		if (TxRetransmit > MAC_ENTRY_LIFE_CHECK_CNT / 4)
			TxRetransmit = MAC_ENTRY_LIFE_CHECK_CNT / 4;
		
		/* No TxPkt ok in this period as continue tx fail */
		pEntry->ContinueTxFailCnt += TxRetransmit;
	}
	else
	{
		pEntry->ContinueTxFailCnt = 0;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s:(OK:%d, FAIL:%d, ConFail:%d) \n",
		__FUNCTION__, TxSuccess, pTxInfo->TxFailCount, pEntry->ContinueTxFailCnt));
}


VOID MtAsicRssiUpdate(RTMP_ADAPTER *pAd)
{
	union WTBL_2_DW13 wtbl_2_d13;
	MAC_TABLE_ENTRY *pEntry;
	INT32 Rssi0 = 0, Rssi1 = 0, Rssi2 = 0;

#ifdef CONFIG_AP_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_AP(pAd)
	{
		if ( pAd->MacTab.Size == 0 )
		{
			pEntry = &pAd->MacTab.Content[MCAST_WCID];
			Wtbl2RcpiGet(pAd, pEntry->wcid, &wtbl_2_d13);

			Rssi0 = (wtbl_2_d13.field.resp_rcpi_0 - 220) / 2;
			Rssi1 = (wtbl_2_d13.field.resp_rcpi_1 - 220) / 2;
			Rssi2 = 0;

			pEntry->RssiSample.AvgRssi[0] = pEntry->RssiSample.LastRssi[0] = Rssi0;
			pEntry->RssiSample.AvgRssi[1] = pEntry->RssiSample.LastRssi[1] = Rssi1;
			pEntry->RssiSample.AvgRssi[2] = pEntry->RssiSample.LastRssi[2] = Rssi2;

			pAd->ApCfg.RssiSample.AvgRssi[0] = Rssi0;
			pAd->ApCfg.RssiSample.AvgRssi[1] = Rssi1;
			pAd->ApCfg.RssiSample.AvgRssi[2] = Rssi2;

			pAd->ApCfg.RssiSample.LastRssi[0] = Rssi0;
			pAd->ApCfg.RssiSample.LastRssi[1] = Rssi1;
			pAd->ApCfg.RssiSample.LastRssi[2] = Rssi2;
		}
		else
		{
			UINT16 i;
			INT32 TotalRssi0 = 0, TotalRssi1 = 0, TotalRssi2 = 0;
			for (i = 1; i < MAX_LEN_OF_MAC_TABLE; i++)
			{
				pEntry = &pAd->MacTab.Content[i];
				if (IS_VALID_ENTRY(pEntry)) {
					Wtbl2RcpiGet(pAd, pEntry->wcid, &wtbl_2_d13);

					Rssi0 = (wtbl_2_d13.field.resp_rcpi_0 - 220) / 2;
					Rssi1 = (wtbl_2_d13.field.resp_rcpi_1 - 220) / 2;
					Rssi2 = 0;

					pEntry->RssiSample.AvgRssi[0] = pEntry->RssiSample.LastRssi[0] = Rssi0;
					pEntry->RssiSample.AvgRssi[1] = pEntry->RssiSample.LastRssi[1] = Rssi1;
					pEntry->RssiSample.AvgRssi[2] = pEntry->RssiSample.LastRssi[2] = Rssi2;

					TotalRssi0 += Rssi0;
					TotalRssi1 += Rssi1;
					TotalRssi2 += Rssi2;
				}
			}

			pAd->ApCfg.RssiSample.AvgRssi[0] = pAd->ApCfg.RssiSample.LastRssi[0] = TotalRssi0 / pAd->MacTab.Size;
			pAd->ApCfg.RssiSample.AvgRssi[1] = pAd->ApCfg.RssiSample.LastRssi[1] = TotalRssi1 / pAd->MacTab.Size;
			pAd->ApCfg.RssiSample.AvgRssi[2] = pAd->ApCfg.RssiSample.LastRssi[2] = TotalRssi2 / pAd->MacTab.Size;
		}

	}
#endif /* CONFIG_AP_SUPPORT */

#ifdef CONFIG_STA_SUPPORT
	IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
	{
		pEntry = &pAd->MacTab.Content[BSSID_WCID];
		Wtbl2RcpiGet(pAd, pEntry->wcid, &wtbl_2_d13);

		Rssi0 = (wtbl_2_d13.field.resp_rcpi_0 - 220) / 2;
		Rssi1 = (wtbl_2_d13.field.resp_rcpi_1 - 220) / 2;
		Rssi2 = 0;

		pEntry->RssiSample.AvgRssi[0] = pEntry->RssiSample.LastRssi[0] = Rssi0;
		pEntry->RssiSample.AvgRssi[1] = pEntry->RssiSample.LastRssi[1] = Rssi1;
		pEntry->RssiSample.AvgRssi[2] = pEntry->RssiSample.LastRssi[2] = Rssi2;

		pAd->StaCfg.RssiSample.AvgRssi[0] = Rssi0;
		pAd->StaCfg.RssiSample.AvgRssi[1] = Rssi1;
		pAd->StaCfg.RssiSample.AvgRssi[2] = Rssi2;

		pAd->StaCfg.RssiSample.LastRssi[0] = Rssi0;
		pAd->StaCfg.RssiSample.LastRssi[1] = Rssi1;
		pAd->StaCfg.RssiSample.LastRssi[2] = Rssi2;
	}
#endif /* CONFIG_STA_SUPPORT */
}


VOID MtAsicRcpiReset(RTMP_ADAPTER *pAd, UCHAR ucWcid)
{
    UINT32 u4RegVal;
    UCHAR ucWaitCnt = 0;

    do {
        MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &u4RegVal);
        if ((u4RegVal & 0x10000)  == 0)
            break;
        ucWaitCnt++;
        RtmpusecDelay(50);
    }while (ucWaitCnt < 100);

    if (ucWaitCnt == 100) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                __FUNCTION__, u4RegVal));
        return;
    }

    u4RegVal = (ucWcid | (1 << 15));
    MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, u4RegVal);
}


VOID MtAsicSetSMPS(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR smps)
{
    struct wtbl_entry tb_entry;
    struct wtbl_1_struc wtbl_1;

    NdisZeroMemory((UCHAR *)&wtbl_1, sizeof(struct wtbl_1_struc));
    NdisZeroMemory(&tb_entry, sizeof(tb_entry));
    if (mt_wtbl_get_entry234(pAd, wcid, &tb_entry) == FALSE) {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Cannot found WTBL2/3/4 for WCID(%d)\n",
                    __FUNCTION__, wcid));

        return ;
    }
    // WTBL1.dw2 bit19, support Dynamic SMPS
    HW_IO_READ32(pAd, tb_entry.wtbl_addr[0] + 8, &wtbl_1.wtbl_1_d2.word);
    wtbl_1.wtbl_1_d2.field.smps = smps;
    HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, wtbl_1.wtbl_1_d2.word);
}


#ifdef MCS_LUT_SUPPORT
UCHAR aucHtMaxRetryLimit[]={
	MCS_0,  4,
	MCS_1,  4,
	MCS_2,  7,
	MCS_3,  8,
	MCS_4,  9,
	MCS_5,  10,
	MCS_6,  11,
	MCS_7,  12,
	MCS_8,  4,
	MCS_9,  7,
	MCS_10, 8,
	MCS_11, 9,
	MCS_12, 10,
	MCS_13, 11,
	MCS_14, 12,
	MCS_15, 12,
};

VOID MtAsicMcsLutUpdate(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry)
{
	union  WTBL_2_DW9 wtbl_2_d9 = {.word = 0};
	UINT32 rate[8]; // reg_val;
	UCHAR stbc, bw, nss, preamble; //, wait_cnt = 0;
	//CHAR rssi;
	UCHAR ucMaxTxRetryCnt = 0;

	// TODO: shiang-MT7603, shall we use MaxHTPhyMode.field.BW or HTPhyMode.field.BW here??
	switch (pEntry->HTPhyMode.field.BW)
	{
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
	wtbl_2_d9.field.fcap = bw;
	wtbl_2_d9.field.ccbw_sel = bw;
	wtbl_2_d9.field.cbrn = 7; // change bw as (fcap/2) if rate_idx > 7, temporary code


	if (pEntry->HTPhyMode.field.ShortGI) {
		wtbl_2_d9.field.g2 = 1;
		wtbl_2_d9.field.g4 = 1;
		wtbl_2_d9.field.g8 = 1;
		wtbl_2_d9.field.g16 = 1;
	} else {
		wtbl_2_d9.field.g2 = 0;
		wtbl_2_d9.field.g4 = 0;
		wtbl_2_d9.field.g8 = 0;
		wtbl_2_d9.field.g16 = 0;
	}
	wtbl_2_d9.field.rate_idx = 0;

	if (OPSTATUS_TEST_FLAG(pAd, fOP_STATUS_SHORT_PREAMBLE_INUSED))
		preamble = SHORT_PREAMBLE;
	else
		preamble = LONG_PREAMBLE;

    stbc = pEntry->HTPhyMode.field.STBC;
#ifdef THERMAL_PROTECT_SUPPORT
    if  (pAd->force_one_tx_stream == TRUE) {
        stbc = 0;
    }
#endif /* THERMAL_PROTECT_SUPPORT */

	nss = get_nss_by_mcs(pEntry->HTPhyMode.field.MODE, pEntry->HTPhyMode.field.MCS, stbc);

	if (((pEntry->MmpsMode != MMPS_STATIC) || ( pEntry->HTPhyMode.field.MODE < MODE_HTMIX ))
#ifdef THERMAL_PROTECT_SUPPORT
			&& (pAd->force_one_tx_stream == FALSE)
#endif /* THERMAL_PROTECT_SUPPORT */
            )
	{
		//rssi = RTMPMaxRssi(pAd, pEntry->RssiSample.AvgRssi[0], pEntry->RssiSample.AvgRssi[1], pEntry->RssiSample.AvgRssi[2]);
		//if (rssi < -50 )
		{
		    if(pAd->CommonCfg.TxStream == 1)
		        wtbl_2_d9.field.spe_en = 0;
		    else
			wtbl_2_d9.field.spe_en = 1;
		}
	}

	rate[0] = tx_rate_to_tmi_rate(pEntry->HTPhyMode.field.MODE,
											pEntry->HTPhyMode.field.MCS,
											nss,
											pEntry->HTPhyMode.field.MCS==MCS_32? 0: stbc,
											preamble);
	rate[0] &= 0xfff;

	if ( pEntry->bAutoTxRateSwitch == TRUE )
	{
		UCHAR	ucIndex;
		UCHAR	DownRateIdx, CurrRateIdx;
		UCHAR mode, mcs;

		CurrRateIdx = pEntry->CurrTxRateIndex;

#ifdef NEW_RATE_ADAPT_SUPPORT
#ifdef WAPI_SUPPORT
		if (IS_MT7603(pAd) || IS_MT7628(pAd) || IS_MT7636(pAd))
		{
			if ((pEntry->AuthMode == Ndis802_11AuthModeWAICERT) || (pEntry->AuthMode == Ndis802_11AuthModeWAIPSK))
			{
				if (pEntry->pTable == RateSwitchTableAdapt11N2S)
				{
					if ((CurrRateIdx >= 14) && (CurrRateIdx <= 16))
					{
						CurrRateIdx = 13;
					}
				}
			}
		}
#endif /* WAPI_SUPPORT */
#endif /* NEW_RATE_ADAPT_SUPPORT */
#ifdef WTBL_RATE_DEBUG
		if((pEntry->pTable !=RateSwitchTableAdapt11B) &&
			(pEntry->pTable !=RateSwitchTableAdapt11G) &&
			(pEntry->pTable !=RateSwitchTableAdapt11BG) &&
			(pEntry->pTable !=RateSwitchTableAdapt11N1S) &&
			(pEntry->pTable !=RateSwitchTableAdapt11N2S))
			{
				printk("invalid pEntry->pTable\n");

				if (pEntry->pTable  == RateSwitchTableAdapt11N3S)
					printk("pTable  == RateSwitchTableAdapt11N3\n");
				else 
					printk("pTable  == unknow\n");
			}
#endif

		for ( ucIndex = 1; ucIndex < 8 ; ucIndex++ )
		{
			if (ADAPT_RATE_TABLE(pEntry->pTable)) {
				RTMP_RA_GRP_TB *pCurrTxRate;

				if ( ucIndex == 7 )
					DownRateIdx = pEntry->LowestTxRateIndex;
				else
					DownRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);

				if (pEntry->HTPhyMode.field.ShortGI)
				{
					pCurrTxRate = PTX_RA_GRP_ENTRY(pEntry->pTable, DownRateIdx);

					if ( pCurrTxRate->CurrMCS == pEntry->HTPhyMode.field.MCS )
					{
						CurrRateIdx = DownRateIdx;
						DownRateIdx = MlmeSelectDownRate(pAd, pEntry, CurrRateIdx);
					}
				}

				pCurrTxRate = PTX_RA_GRP_ENTRY(pEntry->pTable, DownRateIdx);
				mode = pCurrTxRate->Mode;
				mcs = pCurrTxRate->CurrMCS;
			} else {
				RTMP_RA_LEGACY_TB *pCurrTxRate;

				if ( ucIndex == 7 )
					DownRateIdx = pEntry->LowestTxRateIndex;
				else {
					if ( CurrRateIdx > 0 )
						CurrRateIdx -= 1;
					DownRateIdx = CurrRateIdx;
				}

				pCurrTxRate = PTX_RA_LEGACY_ENTRY(pEntry->pTable, DownRateIdx);

				mode = pCurrTxRate->Mode;
				mcs = pCurrTxRate->CurrMCS;
			}


			nss = get_nss_by_mcs(mode, mcs, pEntry->HTPhyMode.field.STBC);
			rate[ucIndex] = tx_rate_to_tmi_rate(mode,
											mcs,
											nss,
											pEntry->HTPhyMode.field.MCS==MCS_32? 0: stbc,
											preamble);

			rate[ucIndex] &= 0xfff;

			CurrRateIdx = DownRateIdx;
		}
	}
	else
	{
		rate[1] = rate[2] = rate[3] = rate[4] = rate[5] = rate[6] = rate[7] = rate[0];
	}

    Wtbl2RateTableUpdate(pAd, pEntry->wcid, wtbl_2_d9.word, rate);

	if (pEntry->MaxHTPhyMode.field.MODE == MODE_HTMIX)
	{
		if (pEntry->bAutoTxRateSwitch == TRUE)
		{
			if(pEntry->HTPhyMode.field.MODE == MODE_CCK)
			{
				if(pEntry->HTPhyMode.field.MCS == 0)
				{
					ucMaxTxRetryCnt = 3;
				}
				else
				{
					ucMaxTxRetryCnt = 4;
				}
			}
			else
			{
				if(pEntry->HTPhyMode.field.MCS == MCS_32)
				{
					ucMaxTxRetryCnt = 4;
				}
				else
				{
					ucMaxTxRetryCnt = aucHtMaxRetryLimit[(pEntry->HTPhyMode.field.MCS*2) + 1];
				}
			}
		}
		else 	   
		{
			ucMaxTxRetryCnt = MT_TX_SHORT_RETRY;
		}
	}
	else
	{
		ucMaxTxRetryCnt = MT_TX_SHORT_RETRY;
	}

	if (pAd->MacTab.Size > 3)
		pEntry->ucMaxTxRetryCnt = ucMaxTxRetryCnt;
	else
		pEntry->ucMaxTxRetryCnt = MT_TX_SHORT_RETRY;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_RA, ("%s():WCID=%d, HTPhyMode=0x%x\n",
				__FUNCTION__, pEntry->wcid, pEntry->HTPhyMode.word));

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO | DBG_FUNC_RA, ("\tCurTxRateIdx=%d, Mode/BW/MCS/STBC/SGI=%d/%d/%d/%d/%d\n\n",
		pEntry->CurrTxRateIndex,
		pEntry->HTPhyMode.field.MODE,
		pEntry->HTPhyMode.field.BW,
		pEntry->HTPhyMode.field.MCS,
		pEntry->HTPhyMode.field.STBC,
		pEntry->HTPhyMode.field.ShortGI));
}
#endif /* MCS_LUT_SUPPORT */


UINT16 AsicGetTidSn(RTMP_ADAPTER *pAd, UCHAR Wcid, UCHAR Tid)
{
    UINT16 Sn = 0;
    UINT32 Reg = 0;
	struct wtbl_entry Entry = {0};
	struct wtbl_2_struc *Wtbl_2 = NULL;


    NdisZeroMemory((UCHAR *)(&Entry), sizeof(struct wtbl_entry));
    mt_wtbl_get_entry234(pAd, Wcid, &Entry);

    Wtbl_2 = &Entry.wtbl_2;

    switch (Tid)
    {
        case 0:
            Reg = Entry.wtbl_addr[1] + (4 * 2); //WTBL2.DW2
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d2.word);
            Sn = Wtbl_2->wtbl_2_d2.field.tid_ac_0_sn;
            break;

        case 1:
            Reg = Entry.wtbl_addr[1] + (4 * 2); //WTBL2.DW2
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d2.word);
            Sn = Wtbl_2->wtbl_2_d2.field.tid_ac_1_sn;
            break;

        case 2:
            Reg = Entry.wtbl_addr[1] + (4 * 2); //WTBL2.DW2
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d2.word);
            Sn = Wtbl_2->wtbl_2_d2.field.tid_ac_2_sn_0;

            Reg = Entry.wtbl_addr[1] + (4 * 3); //WTBL2.DW3
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d3.word);
            Sn = (Wtbl_2->wtbl_2_d3.field.tid_ac_2_sn_9 << 8) | Sn;
            break;

        case 3:
            Reg = Entry.wtbl_addr[1] + (4 * 3); //WTBL2.DW3
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d3.word);
            Sn = Wtbl_2->wtbl_2_d3.field.tid_ac_3_sn;
            break;

        case 4:
            Reg = Entry.wtbl_addr[1] + (4 * 3); //WTBL2.DW3
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d3.word);
            Sn = Wtbl_2->wtbl_2_d3.field.tid_4_sn;
            break;

        case 5:
            Reg = Entry.wtbl_addr[1] + (4 * 3); //WTBL2.DW3
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d3.word);
            Sn = Wtbl_2->wtbl_2_d3.field.tid_5_sn_0;

            Reg = Entry.wtbl_addr[1] + (4 * 4); //WTBL2.DW4
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d4.word);
            Sn = (Wtbl_2->wtbl_2_d4.field.tid_5_sn_5 << 4) | Sn;
            break;

        case 6:
            Reg = Entry.wtbl_addr[1] + (4 * 4); //WTBL2.DW4
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d4.word);
            Sn = Wtbl_2->wtbl_2_d4.field.tid_6_sn;
            break;

        case 7:
            Reg = Entry.wtbl_addr[1] + (4 * 4); //WTBL2.DW4
            HW_IO_READ32(pAd, Reg, &Wtbl_2->wtbl_2_d4.word);
            Sn = Wtbl_2->wtbl_2_d4.field.tid_7_sn;
            break;

        default:
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown TID(%d) \n", __FUNCTION__, Tid));
            break;
    }

    return Sn;
}


static UCHAR ba_range[] = {4, 5, 8, 10, 16, 20, 21, 42};
// TODO: shiang-usw, fix me for pAd->MacTab.!!
VOID MtAsicUpdateBASession(RTMP_ADAPTER *pAd, UCHAR wcid, UCHAR tid, UINT16 sn, UCHAR basize, BOOLEAN isAdd, INT ses_type)
{
	struct wtbl_entry ent;
	struct wtbl_2_struc *wtbl_2;
	UINT32 range_mask = 0x7 << (tid * 3);
	UINT32 reg, value;
	// TODO: shiang-usw, revise to remove mac_entry->Addr[]!!
    MAC_TABLE_ENTRY *mac_entry;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Update BA Session Info of wcid(%d)=>tid=%d, sn = %d, basize=%d, isAdd=%d, ses_type=%s(%d)\n",
			__FUNCTION__, wcid, tid, sn, basize, isAdd, (ses_type == BA_SESSION_ORI ? "Ori" : "Recp"), ses_type));

	if (ses_type == BA_SESSION_RECP)
	{
        /* Reset BA SSN & Score Board Bitmap, for BA Receiptor */
		if (isAdd)
		{
			mac_entry = &pAd->MacTab.Content[wcid];
        	value = (mac_entry->Addr[0] | (mac_entry->Addr[1] << 8) |
                (mac_entry->Addr[2] << 16) | (mac_entry->Addr[3] << 24));
			MAC_IO_WRITE32(pAd, BSCR0, value);

			MAC_IO_READ32(pAd, BSCR1, &value);
        	value &= ~(BA_MAC_ADDR_47_32_MASK | RST_BA_TID_MASK | RST_BA_SEL_MASK);
        	value |= BA_MAC_ADDR_47_32((mac_entry->Addr[4] | (mac_entry->Addr[5] << 8)));
        	value |= (RST_BA_SEL(RST_BA_MAC_TID_MATCH) | RST_BA_TID(tid) | START_RST_BA_SB);
			MAC_IO_WRITE32(pAd, BSCR1, value);
		}
	}
	else
	{
		NdisZeroMemory((UCHAR *)(&ent), sizeof(struct wtbl_entry));
		mt_wtbl_get_entry234(pAd, wcid, &ent);

		wtbl_2 = &ent.wtbl_2;

		if (isAdd)
		{
			INT idx = 0;

			while (ba_range[idx] < basize) {
				if (idx == 7)
					break;
				idx++;
			};

			if (ba_range[idx] > basize)
				idx--;

			reg = ent.wtbl_addr[1] + (15 * 4);
			HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d15.word);

			wtbl_2->wtbl_2_d15.field.ba_en |= 1 <<tid;
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid &= (~range_mask);
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid |= (idx << (tid * 3));

			HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d15.word);
		}
		else
		{
			reg = ent.wtbl_addr[1] + (15 * 4);
			HW_IO_READ32(pAd, reg, &wtbl_2->wtbl_2_d15.word);

			wtbl_2->wtbl_2_d15.field.ba_en &=  (~(1 <<tid));
			wtbl_2->wtbl_2_d15.field.ba_win_size_tid &= (~range_mask);

			HW_IO_WRITE32(pAd, reg, wtbl_2->wtbl_2_d15.word);
		}

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): ent->wtbl_addr[1]=0x%x, val=0x%08x\n",
					__FUNCTION__, reg, wtbl_2->wtbl_2_d15.word));
	}
}


// TODO: Shiang-usw, fix me for WCID role definition both for windows/Linux !!
VOID MtAsicUpdateRxWCIDTable(RTMP_ADAPTER *pAd, USHORT WCID, UCHAR *pAddr)
{
	struct wtbl_entry tb_entry;
	union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
	union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
	union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
	union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
	union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;
	struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	MAC_TABLE_ENTRY *mac_entry;
	UINT32 Value, Index;
	UCHAR WaitCnt = 0;

#ifdef RT_CFG80211_P2P_SUPPORT
	INT apidx = MAIN_MBSSID;
#endif /* RT_CFG80211_P2P_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
	/* HW BSSID  will be 1, if in softap or P2P GO mode. Need this value to stop TX queue AC0~AC3 when update WCID*/
	if (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
		apidx = CFG_GO_BSSID_IDX;
#endif /* RT_CFG80211_P2P_SUPPORT */

	if (wtbl_ctrl->wtbl_entry_cnt[0] > 0)
	{
		WCID = (WCID < wtbl_ctrl->wtbl_entry_cnt[0] ? WCID : MCAST_WCID);
	}
	else
        {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():PSE not init yet!\n", __FUNCTION__));
		return;
	}

	// TODO: shiang-7603

	NdisZeroMemory(&tb_entry, sizeof(tb_entry));
	if (mt_wtbl_get_entry234(pAd, (UCHAR)WCID, &tb_entry) == FALSE) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
					__FUNCTION__, WCID));
		return;
	}

    dw0->field.wm = 0;

    if (WCID == MCAST_WCID)
    {
        dw0->field.muar_idx = 0xe;
        dw2->field.adm = 0;
        dw2->field.cipher_suit = WTBL_CIPHER_NONE;
        dw0->field.rv = 1;
	dw0->field.rc_a1 = 1;
	dw0->field.rc_a2 = 1;
    }
#ifdef CONFIG_STA_SUPPORT
    /* for ad-hoc BC/MC Entry using, Addr ==> FF:FF:FF:FF:FF:FF */
    else if (WCID == pAd->StaCfg.wdev.tr_tb_idx) {
	dw0->field.muar_idx = 0xe;
        dw2->field.adm = 0;
        dw2->field.cipher_suit = WTBL_CIPHER_NONE;
		dw0->field.rc_a1 = 1;
    }
#endif /* CONFIG_STA_SUPPORT */	
#ifdef MULTI_APCLI_SUPPORT
    else if (WCID == APCLI_MCAST_WCID(0) || WCID == APCLI_MCAST_WCID(1))
#else
    else if (WCID == APCLI_MCAST_WCID)
#endif
    {
        dw0->field.muar_idx = 0xe;
        dw0->field.rv = 1;
        dw2->field.adm = 0;
        dw2->field.cipher_suit = WTBL_CIPHER_NONE;
        dw0->field.rc_a2 = 1;
	dw0->field.rc_a1 = 1;
        dw3->field.i_psm = 1;
        dw3->field.du_i_psm = 1;  
	    
    }
    else {
        mac_entry = &pAd->MacTab.Content[WCID];

        //dw0->field.muar_idx = 0x0; // TODO: need to change depends on different BssIdx!
        if (IS_ENTRY_CLIENT(mac_entry)) {
            if (mac_entry->func_tb_idx == 0)
                dw0->field.muar_idx = 0x0;
            else if (mac_entry->func_tb_idx >= 1 && mac_entry->func_tb_idx <= 15)
                dw0->field.muar_idx = 0x10 | mac_entry->func_tb_idx;
            /* for concurrent to handle HW_BSSID_1/2/3 */
            if (mac_entry->wdev->hw_bssid_idx != 0)
            	dw0->field.muar_idx = mac_entry->wdev->hw_bssid_idx ;
			
        }
        else if (IS_ENTRY_APCLI(mac_entry)) {
#ifdef MULTI_APCLI_SUPPORT
		dw0->field.muar_idx = (0x1 + mac_entry->func_tb_idx);
#else /* MULTI_APCLI_SUPPORT */
            dw0->field.muar_idx = 0x1;//Carter, MT_MAC apcli use HWBSSID1 to go.
#endif /* !MULTI_APCLI_SUPPORT */
            dw0->field.rc_a1 = 1;
		dw3->field.i_psm = 1;
		dw3->field.du_i_psm = 1; 
           
        }
        else
            dw0->field.muar_idx = 0x0;

        dw0->field.rv = 1;
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): WCID=%d mac_entry->type=%d, pMacEntry->MaxHTPhyMode.field.MODE=%d, StatusFlag=0x%lx\n",
                    __FUNCTION__,WCID, mac_entry->EntryType,
                    mac_entry->MaxHTPhyMode.field.MODE,
                    mac_entry->ClientStatusFlags));

        dw0->field.rc_a2 = 1;
        // TODO: shiang-MT7603, in which case we need to check A1???
        //dw0->field.rc_a1 = 1;
        dw2->field.adm = 1;
        switch (mac_entry->WepStatus)
        {
            case Ndis802_11WEPDisabled:
                dw2->field.cipher_suit = WTBL_CIPHER_NONE;
                break;
            default:
                break;
        }

		if (mac_entry->AuthMode == Ndis802_11AuthModeShared)
		{
			dw0->field.rkv = 0;
			dw2->field.cipher_suit = WTBL_CIPHER_WEP_40;
		}

        if (IS_HT_STA(mac_entry))
        {
            dw2->field.ht = 1;
            dw2->field.qos = 1;
            dw2->field.mm = mac_entry->MpduDensity;
            dw2->field.af = mac_entry->MaxRAmpduFactor;
            if (CLIENT_STATUS_TEST_FLAG(mac_entry, fCLIENT_STATUS_RDG_CAPABLE)) {
                dw2->field.r = 1;
                dw2->field.rdg_ba = 1;
            }

            if (mac_entry->MmpsMode == MMPS_DYNAMIC)
                dw2->field.smps = 1;
            else
                dw2->field.smps = 0;

            dw4->field.partial_aid = WCID;
        }
    }

    dw0->field.addr_4 = pAddr[4];
    dw0->field.addr_5 = pAddr[5];
    dw1->word = pAddr[0] + (pAddr[1] << 8) +(pAddr[2] << 16) +(pAddr[3] << 24);
    dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
    dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
    dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
    dw3->field.psm = 0;
#ifndef MT_PS
	dw3->field.i_psm = 1;
	dw3->field.du_i_psm = 1;
#endif /* !MT_PS */
    dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
    dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
    dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];

    if (WCID >= 0) {
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
        HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
        HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);

        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
                    __FUNCTION__, WCID, tb_entry.wtbl_addr[0], dw0->word));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
                    __FUNCTION__, WCID,  tb_entry.wtbl_addr[0] + 4, dw1->word));
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
                    __FUNCTION__, WCID, tb_entry.wtbl_addr[0] + 8, dw2->word));

		MAC_IO_READ32(pAd, WTBL1OR, &Value);
        Value |= PSM_W_FLAG;
		MAC_IO_WRITE32(pAd, WTBL1OR, Value);

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
                    __FUNCTION__, WCID, tb_entry.wtbl_addr[0] + 12, dw3->word));

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 16, dw4->word);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
                    __FUNCTION__, WCID, tb_entry.wtbl_addr[0] + 16, dw4->word));

		MAC_IO_READ32(pAd, WTBL1OR, &Value);
        Value &= ~PSM_W_FLAG;
		MAC_IO_WRITE32(pAd, WTBL1OR, Value);


        /* Clear BA Information */
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (15 * 4), tb_entry.wtbl_2.wtbl_2_d15.word);

        /* Clear WTBL2 TID SN to default */
	/* We need change disable TX to just disable TX Q only, or RX path may have problem due to we can not TX ack*/
        AsicSetMacTxRx(pAd, ASIC_MAC_TX, FALSE);
	//MtAsicACQueue(pAd, AC_QUEUE_STOP, apidx, 0xF);
        HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (2 * 4), 0x0);
        HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (3 * 4), 0x0);
        HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (4 * 4), 0x0);

        MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(WCID);
        Value |= WTBL2_UPDATE_FLAG;
        MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
            MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
	    //MtAsicACQueue(pAd, AC_QUEUE_STOP, apidx, 0xF);
            return;
        }
        AsicSetMacTxRx(pAd, ASIC_MAC_TX, TRUE);
	//MtAsicACQueue(pAd, AC_QUEUE_STOP, apidx, 0xF);

        /* RX Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(WCID);
        Value |= RX_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

/*7636 E1 workaroud, must after RX counter clear*/

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }

        /* TX Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(WCID);
        Value |= TX_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }

        /* Clear Cipher Key */
        for (Index = 0; Index < 8; Index++)
			HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[2] + (4 * Index), 0x0);

        /* Admission Control Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(WCID);
        Value |= ADM_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }
    }
}


/*
   ========================================================================
Description:
Add Client security information into ASIC WCID table and IVEIV table.
Return:

Note :
The key table selection rule :
1.	Wds-links and Mesh-links always use Pair-wise key table.
2. 	When the CipherAlg is TKIP, AES, SMS4 or the dynamic WEP is enabled,
it needs to set key into Pair-wise Key Table.
3.	The pair-wise key security mode is set NONE, it means as no security.
4.	In STA Adhoc mode, it always use shared key table.
5.	Otherwise, use shared key table

========================================================================
 */
VOID MtAsicUpdateWcidAttributeEntry(
        IN	PRTMP_ADAPTER	pAd,
        IN	UCHAR			BssIdx,
        IN 	UCHAR		 	KeyIdx,
        IN 	UCHAR		 	CipherAlg,
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
VOID MtAsicDelWcidTab(RTMP_ADAPTER *pAd, UCHAR wcid_idx)
{
    UCHAR cnt, cnt_s, cnt_e;
    struct wtbl_entry tb_entry;
    UCHAR WaitCnt = 0;
    UINT32 Index = 0, Value;
    union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
    union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
    union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():wcid_idx=0x%x\n",
				__FUNCTION__, wcid_idx));

    if (wcid_idx == WCID_ALL) {
        cnt_s = 0;
#ifdef RT_CFG80211_P2P_SUPPORT
		//YF TODO: shall re-write the API parm
		cnt_s = 2;
#endif /* RT_CFG80211_P2P_SUPPORT */		
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

        dw0->field.wm = 0;
        dw0->field.rc_a2 = 1;
        dw0->field.rv = 1;
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], tb_entry.wtbl_1.wtbl_1_d0.word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, tb_entry.wtbl_1.wtbl_1_d1.word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, tb_entry.wtbl_1.wtbl_1_d2.word);

		MAC_IO_READ32(pAd, WTBL1OR, &Value);
        Value |= PSM_W_FLAG;
		MAC_IO_WRITE32(pAd, WTBL1OR, Value);

        dw3->field.wtbl2_fid = tb_entry.wtbl_fid[1];
        dw3->field.wtbl2_eid = tb_entry.wtbl_eid[1];
        dw3->field.wtbl4_fid = tb_entry.wtbl_fid[3];
        dw4->field.wtbl3_fid = tb_entry.wtbl_fid[2];
        dw4->field.wtbl3_eid = tb_entry.wtbl_eid[2];
        dw4->field.wtbl4_eid = tb_entry.wtbl_eid[3];

#ifdef MT_PS
        if (cnt == MCAST_WCID) {
            dw3->field.i_psm=1;
            dw3->field.du_i_psm=1;
        }
#endif

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 12, dw3->word);

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 16, dw4->word);

		MAC_IO_READ32(pAd, WTBL1OR, &Value);
        Value &= ~PSM_W_FLAG;
		MAC_IO_WRITE32(pAd, WTBL1OR, Value);


        /* Clear BA Information */
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (15 * 4), tb_entry.wtbl_2.wtbl_2_d15.word);

	/*Clear PN */
	tb_entry.wtbl_2.wtbl_2_d0.pn_0=0;
	tb_entry.wtbl_2.wtbl_2_d1.field.pn_32=0;
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (0 * 4), tb_entry.wtbl_2.wtbl_2_d0.word);
	HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[1] + (1 * 4), tb_entry.wtbl_2.wtbl_2_d1.word);		
	
        /* RX Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(cnt);
        Value |= RX_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }

        /* TX Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(cnt);
        Value |= TX_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }

        /* Clear Cipher Key */
        for (Index = 0; Index < 8; Index++)
			HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[2] + (4 * Index), 0x0);

        /* Admission Control Counter Clear */
		MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
        Value &= ~WLAN_IDX_MASK;
        Value |= WLAN_IDX(cnt);
        Value |= ADM_CNT_CLEAR;
		MAC_IO_WRITE32(pAd, WTBL_OFF_WIUCR, Value);

        WaitCnt = 0;

        do {
			MAC_IO_READ32(pAd, WTBL_OFF_WIUCR, &Value);
            if ((Value & IU_BUSY)  == 0)
                break;
            WaitCnt++;
            RtmpusecDelay(50);
        } while (WaitCnt < 100);

        if (WaitCnt == 100) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): Previous update not applied by HW yet!(reg_val=0x%x)\n",
                        __FUNCTION__, Value));
            return;
        }


    }
}


VOID CmdProcAddRemoveKey(
        IN  PRTMP_ADAPTER 	pAd,
        IN	UCHAR			AddRemove,
        IN  UCHAR			BssIdx,
        IN	UCHAR			key_idx,
        IN 	UCHAR			Wcid,
        IN	UCHAR			KeyTabFlag,
        IN	PCIPHER_KEY		pCipherKey,
        IN	PUCHAR			PeerAddr)
{
    CMD_802_11_KEY CmdKey;
    UCHAR *pKey = pCipherKey->Key;
    UCHAR *pTxMic = pCipherKey->TxMic;
    UCHAR *pRxMic = pCipherKey->RxMic;
    UCHAR CipherAlg = pCipherKey->CipherAlg;
    struct wtbl_entry tb_entry;
    union WTBL_1_DW0 *dw0 = (union WTBL_1_DW0 *)&tb_entry.wtbl_1.wtbl_1_d0.word;
    union WTBL_1_DW1 *dw1 = (union WTBL_1_DW1 *)&tb_entry.wtbl_1.wtbl_1_d1.word;
    union WTBL_1_DW2 *dw2 = (union WTBL_1_DW2 *)&tb_entry.wtbl_1.wtbl_1_d2.word;
    //union WTBL_1_DW3 *dw3 = (union WTBL_1_DW3 *)&tb_entry.wtbl_1.wtbl_1_d3.word;
    //union WTBL_1_DW4 *dw4 = (union WTBL_1_DW4 *)&tb_entry.wtbl_1.wtbl_1_d4.word;
    //struct rtmp_mac_ctrl *wtbl_ctrl = &pAd->mac_ctrl;
	// TODO: shiang-usw, revise this function!!!

    memset(&CmdKey, 0x00, sizeof(CMD_802_11_KEY));

    CmdKey.ucAddRemove = AddRemove;
    CmdKey.ucWlanIndex = Wcid;
    CmdKey.ucBssIndex = BssIdx;
    CmdKey.ucKeyId = key_idx;
    CmdKey.ucKeyType = KeyTabFlag;
    memcpy(CmdKey.aucPeerAddr, PeerAddr, 6);
    switch(CipherAlg)
    {
        case CIPHER_WEP64:
            CmdKey.ucAlgorithmId = 1;
            CmdKey.ucKeyLen = 5;
            memcpy(CmdKey.aucKeyMaterial, pKey, CmdKey.ucKeyLen);
            break;
        case CIPHER_WEP128:
            CmdKey.ucAlgorithmId = 5;
            CmdKey.ucKeyLen = 13;
            memcpy(CmdKey.aucKeyMaterial, pKey, CmdKey.ucKeyLen);
            break;
        case CIPHER_TKIP:
            CmdKey.ucAlgorithmId = 2;
            CmdKey.ucKeyLen = 32;
            memcpy(CmdKey.aucKeyMaterial, pKey, 16);
            memcpy(&CmdKey.aucKeyMaterial[16], pRxMic, 8);
            memcpy(&CmdKey.aucKeyMaterial[24], pTxMic, 8);
            break;
        case CIPHER_AES:
            CmdKey.ucAlgorithmId = 4;
            CmdKey.ucKeyLen = 16;
            memcpy(CmdKey.aucKeyMaterial, pKey, CmdKey.ucKeyLen);
            break;
        case CIPHER_SMS4:
            CmdKey.ucAlgorithmId = 8;
            CmdKey.ucKeyLen = 32;
            memcpy(CmdKey.aucKeyMaterial, pKey, 16);
		memcpy(&CmdKey.aucKeyMaterial[16], pTxMic, 8);
		memcpy(&CmdKey.aucKeyMaterial[24], pRxMic, 8);
            break;
        case CIPHER_WEP152:
            CmdKey.ucAlgorithmId = 7;
            CmdKey.ucKeyLen = 16;
            memcpy(CmdKey.aucKeyMaterial, pKey, CmdKey.ucKeyLen);
            break;
        case CIPHER_BIP:
            CmdKey.ucAlgorithmId = 6;
            CmdKey.ucKeyLen = 16;
            memcpy(CmdKey.aucKeyMaterial, pKey, CmdKey.ucKeyLen);
            break;
        default:
            if (CmdKey.ucAddRemove)
                break;
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support Cipher[%d] for HIF_MT yet!\n",
                        __FUNCTION__, __LINE__, CipherAlg));
            return;
    }

    /* Driver set security key */
	if (CmdKey.ucAddRemove == 0) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("add key table:wcid[%d]\n", CmdKey.ucWlanIndex));

		NdisZeroMemory(&tb_entry, sizeof(tb_entry));
		if (mt_wtbl_get_entry234(pAd, CmdKey.ucWlanIndex, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
						__FUNCTION__, CmdKey.ucWlanIndex));
			return;
		}

		{
			UINT32 addr = 0, index = 0, *pKey = (UINT32 *)CmdKey.aucKeyMaterial;
			addr = pAd->mac_ctrl.wtbl_base_addr[2] + CmdKey.ucWlanIndex * pAd->mac_ctrl.wtbl_entry_size[2];
			if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128) || (CipherAlg == CIPHER_WEP152))
				addr = CmdKey.ucKeyId*16+addr;
			if (CipherAlg == CIPHER_SMS4)
				addr = CmdKey.ucKeyId*32+addr;
			if (CipherAlg == CIPHER_BIP)
				addr = addr+16;
			for (index=0;index<CmdKey.ucKeyLen;index+=4)
			{
#ifdef RT_BIG_ENDIAN
				*pKey=SWAP32(*pKey);
#endif /* RT_BIG_ENDIAN */
				HW_IO_WRITE32(pAd, addr+index, *(pKey));
				pKey++;
			}
		}

		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0], &dw0->word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+4, &dw1->word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+8, &dw2->word);

		dw0->field.wm = 0;

		if((Wcid != MCAST_WCID)
#ifdef APCLI_SUPPORT
#ifdef MULTI_APCLI_SUPPORT
          && ((Wcid != APCLI_MCAST_WCID(0)) && (Wcid != APCLI_MCAST_WCID(1)) )
#else /* MULTI_APCLI_SUPPORT */
          && (Wcid != APCLI_MCAST_WCID)
#endif /* !MULTI_APCLI_SUPPORT */
#endif /* APCLI_SUPPORT */
           )
        {
			dw0->field.addr_4 = CmdKey.aucPeerAddr[4];
			dw0->field.addr_5 = CmdKey.aucPeerAddr[5];
			dw1->word = CmdKey.aucPeerAddr[0] + (CmdKey.aucPeerAddr[1] << 8) +(CmdKey.aucPeerAddr[2] << 16) +(CmdKey.aucPeerAddr[3] << 24);
        }


#ifdef CONFIG_AP_SUPPORT
		if ((pAd->OpMode == OPMODE_AP)
#ifdef RT_CFG80211_P2P_SUPPORT
			|| (pAd->cfg80211_ctrl.isCfgInApMode == RT_CMD_80211_IFTYPE_AP)
#endif /* RT_CFG80211_P2P_SUPPORT */
		)
		{
			if (KeyTabFlag == SHAREDKEYTABLE) {
				dw0->field.rv = 0;
				dw0->field.rkv = 0;
#ifdef APCLI_SUPPORT
#ifdef MULTI_APCLI_SUPPORT
				if ((Wcid == APCLI_MCAST_WCID(0)) || (Wcid == APCLI_MCAST_WCID(1)))
#else /* MULTI_APCLI_SUPPORT */
				if (Wcid == APCLI_MCAST_WCID)
#endif /* !MULTI_APCLI_SUPPORT */
				{
					dw0->field.rv = 1;
					dw0->field.rkv = 1;
				}
#endif /* APCLI_SUPPORT */
#ifdef RT_CFG80211_P2P_SUPPORT
            	if (Wcid == BSSID_WCID ||
		    	    Wcid == MCAST_WCID)
				{
					dw0->field.rv = 1;
					dw0->field.rkv = 1;
					dw0->field.rc_id = 1;
					dw0->field.rc_a1 = 1;
					if (CipherAlg == CIPHER_SMS4)
						dw2->field.wpi_even = 1;
				}
#endif /* RT_CFG80211_P2P_SUPPORT */
			}
			else {
				dw0->field.rv = 1;
				dw0->field.rkv = 1;
			}
		}
		else
#endif /* CONFIG_AP_SUPPORT */
		{
#ifdef CONFIG_STA_SUPPORT
			IF_DEV_CONFIG_OPMODE_ON_STA(pAd)
			{
				dw0->field.rv = 1;
				dw0->field.rkv = 1;
				dw0->field.rc_a1 = 1;
				dw0->field.rc_id = 1;
				if (CipherAlg == CIPHER_SMS4)
					dw2->field.wpi_even = 1;
				}
#endif /* CONFIG_STA_SUPPORT */
		}

		dw2->field.adm = 1;
		dw2->field.cipher_suit = CmdKey.ucAlgorithmId;
		dw0->field.kid = CmdKey.ucKeyId;

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex, tb_entry.wtbl_addr[0], dw0->word));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex,  tb_entry.wtbl_addr[0] + 4, dw1->word));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex, tb_entry.wtbl_addr[0] + 8, dw2->word));

	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("remove key table:wcid[%d]\n", CmdKey.ucWlanIndex));

		NdisZeroMemory(&tb_entry, sizeof(tb_entry));
		if (mt_wtbl_get_entry234(pAd, CmdKey.ucWlanIndex, &tb_entry) == FALSE) {
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s():Cannot found WTBL2/3/4 for WCID(%d)\n",
						__FUNCTION__, CmdKey.ucWlanIndex));
			return;
		}

		{
			UINT32 	addr = 0, index = 0;
			addr = pAd->mac_ctrl.wtbl_base_addr[2] + CmdKey.ucWlanIndex * pAd->mac_ctrl.wtbl_entry_size[2];
			if ((CipherAlg == CIPHER_WEP64) || (CipherAlg == CIPHER_WEP128) || (CipherAlg == CIPHER_WEP152))
				addr = CmdKey.ucKeyId*16+addr;

			for (index=0;index<CmdKey.ucKeyLen;index+=4)
				HW_IO_WRITE32(pAd, addr+index, 0);
		}

		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0], &dw0->word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+4, &dw1->word);
		HW_IO_READ32(pAd, tb_entry.wtbl_addr[0]+8, &dw2->word);

		dw0->field.wm = 0;
		dw0->field.addr_4 = CmdKey.aucPeerAddr[4];
		dw0->field.addr_5 = CmdKey.aucPeerAddr[5];
		dw1->word = CmdKey.aucPeerAddr[0] +
					(CmdKey.aucPeerAddr[1] << 8) +
					(CmdKey.aucPeerAddr[2] << 16) +
					(CmdKey.aucPeerAddr[3] << 24);

		dw0->field.rv = 0;
		dw2->field.adm = 0;
		dw0->field.rkv = 0;
		dw2->field.cipher_suit = 0;
		dw0->field.kid = 0;

		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0], dw0->word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 4, dw1->word);
		HW_IO_WRITE32(pAd, tb_entry.wtbl_addr[0] + 8, dw2->word);

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex, tb_entry.wtbl_addr[0], dw0->word));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex,  tb_entry.wtbl_addr[0] + 4, dw1->word));

		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d):Write WTBL 1 Addr:0x%x, Value:0x%x\n",
					__FUNCTION__, CmdKey.ucWlanIndex, tb_entry.wtbl_addr[0] + 8, dw2->word));


	}
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

    // TODO: shiang-usw, revise this!!!
#ifdef RTMP_MAC_PCI
	cmd_done = AsicCheckCommanOk(pAd, Token);
#endif /* RTMP_MAC_PCI */
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


VOID MtAsicTurnOffRFClk(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	// TODO: shiang-usw, unify the pAd->chipOps!
	if (pAd->chipOps.AsicRfTurnOff)
		pAd->chipOps.AsicRfTurnOff(pAd);
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s():Channel #%d, Unkonwn RFIC=%d\n",
					__FUNCTION__, Channel, pAd->RfIcType));
	}
}


#ifdef WAPI_SUPPORT
VOID MtAsicUpdateWAPIPN(
	IN PRTMP_ADAPTER pAd,
	IN USHORT		 WCID,
	IN ULONG         pn_low,
	IN ULONG         pn_high)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
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
		pAd		 ---
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
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}


VOID MtAsicStreamModeInit(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return;
}
#endif // STREAM_MODE_SUPPORT //


VOID MtAsicSetTxPreamble(RTMP_ADAPTER *pAd, USHORT TxPreamble)
{
	//AUTO_RSP_CFG_STRUC csr4;

	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
						__FUNCTION__, __LINE__));
	return;
}


#ifdef DOT11_N_SUPPORT
INT MtAsicReadAggCnt(RTMP_ADAPTER *pAd, ULONG *aggCnt, int cnt_len)
{
	NdisZeroMemory(aggCnt, cnt_len * sizeof(ULONG));

	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


INT MtAsicSetRalinkBurstMode(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return FALSE;
}


INT MtAsicUpdateTxOP(RTMP_ADAPTER *pAd, UINT32 ac_num, UINT32 txop_val)
{
    UINT32 last_txop_val;


    last_txop_val = MtAsicGetWmmParam(pAd, ac_num, WMM_PARAM_TXOP);

    if (last_txop_val == txop_val)
    { /* No need to Update TxOP CR */
        return TRUE;
    }
    else if (last_txop_val == 0xdeadbeef)
    {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Error CR value for TxOP = 0x%08x\n", __FUNCTION__, last_txop_val));

        return FALSE;
    }
    else {}

    MtAsicSetWmmParam(pAd, ac_num, WMM_PARAM_TXOP, txop_val);

    return TRUE;
}
#endif // DOT11_N_SUPPORT //



INT MtAsicWaitMacTxRxIdle(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


INT32 MtAsicSetMacTxRx(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable)
{
	UINT32 Value, Value1, Value2;

	MAC_IO_READ32(pAd, ARB_SCR, &Value);
	MAC_IO_READ32(pAd, ARB_TQCR0, &Value1);
	MAC_IO_READ32(pAd, ARB_RQCR, &Value2);

	switch (TxRx)
	{
		case ASIC_MAC_TX:
			if (Enable)
			{
				Value &= ~MT_ARB_SCR_TXDIS;
				Value1 = 0xffffffff;
			}
			else
			{
				Value |= MT_ARB_SCR_TXDIS;
				Value1 = 0;
			}
			break;
		case ASIC_MAC_RX:
			if (Enable)
			{
				Value &= ~MT_ARB_SCR_RXDIS;
				Value2 |= ARB_RQCR_RX_START;
			}
			else
			{
				Value |= MT_ARB_SCR_RXDIS;
				Value2 &= ~ARB_RQCR_RX_START;
			}
			break;
		case ASIC_MAC_TXRX:
			if (Enable)
			{
				Value &= ~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value1 = 0xffffffff;
				Value2 |= ARB_RQCR_RX_START;
			}
			else
			{
				Value |= (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value1 = 0;
				Value2 &= ~ARB_RQCR_RX_START;

			}
			break;
		case ASIC_MAC_TXRX_RXV:
			if (Enable)
			{
				Value &= ~(MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value1 = 0xffffffff;
				Value2 |= (ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN |	ARB_RQCR_RXV_T_EN);
			}
			else
			{
				Value |= (MT_ARB_SCR_TXDIS | MT_ARB_SCR_RXDIS);
				Value1 = 0;
				Value2 &= ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN |	ARB_RQCR_RXV_T_EN);
			}
			break;
		case ASIC_MAC_RXV:
			if (Enable)
			{
				Value &= ~MT_ARB_SCR_RXDIS;
				Value2 |= (ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN |	ARB_RQCR_RXV_T_EN);
			}
			else
			{
				Value2 &= ~(ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN |	ARB_RQCR_RXV_T_EN);
			}
			break;
		case ASIC_MAC_RX_RXV:
			if (Enable)
			{

				Value &= ~MT_ARB_SCR_RXDIS;
				Value2 |= (ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN | ARB_RQCR_RXV_T_EN);

			}
			else
			{
				Value |= MT_ARB_SCR_RXDIS;
				Value2 &= ~(ARB_RQCR_RX_START | ARB_RQCR_RXV_START |
							ARB_RQCR_RXV_R_EN |	ARB_RQCR_RXV_T_EN);
			}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __FUNCTION__,
										TxRx));
			break;
	}

	MAC_IO_WRITE32(pAd, ARB_SCR, Value);
	MAC_IO_WRITE32(pAd, ARB_TQCR0, Value1);
	MAC_IO_WRITE32(pAd, ARB_RQCR, Value2);

	return TRUE;
}


INT MtAsicSetWPDMA(RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN enable)
{
#ifdef RTMP_MAC_PCI
	WPDMA_GLO_CFG_STRUC GloCfg;

	// TODO: shiang-7603

	HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
	switch (TxRx)
	{
		case PDMA_TX:
			if (enable == TRUE)
			{
				GloCfg.field.EnableTxDMA = 1;
				GloCfg.field.EnTXWriteBackDDONE = 1;
				GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
			} else {
				GloCfg.field.EnableTxDMA = 0;
			}
			break;
		case PDMA_RX:
			if (enable == TRUE)
			{
				GloCfg.field.EnableRxDMA = 1;
				GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
			} else {
				GloCfg.field.EnableRxDMA = 0;
			}
			break;
		case PDMA_TX_RX:
			if (enable == TRUE)
			{
				GloCfg.field.EnableTxDMA = 1;
				GloCfg.field.EnableRxDMA = 1;
				GloCfg.field.EnTXWriteBackDDONE = 1;
				GloCfg.field.WPDMABurstSIZE = pAd->chipCap.WPDMABurstSIZE;
			} else {
				GloCfg.field.EnableRxDMA = 0;
				GloCfg.field.EnableTxDMA = 0;
			}
			break;
		default:
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknown path (%d\n", __FUNCTION__, TxRx));
			break;
	}
	HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, GloCfg.word);
#endif

	return TRUE;
}


BOOLEAN MtAsicWaitPDMAIdle(struct _RTMP_ADAPTER *pAd, INT round, INT wait_us)
{
#ifdef RTMP_MAC_PCI
	INT i = 0;
	WPDMA_GLO_CFG_STRUC GloCfg;

	// TODO: shiang-7603
	do {
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &GloCfg.word);
		if ((GloCfg.field.TxDMABusy == 0)  && (GloCfg.field.RxDMABusy == 0)) {
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


INT MtAsicSetMacWD(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


// TODO: shiang-usw, move this function to other place!
INT MtStopDmaRx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	PNDIS_PACKET pRxPacket;
	RX_BLK RxBlk, *pRxBlk;
	UINT32 RxPending = 0, MacReg = 0, MTxCycle = 0;
	BOOLEAN bReschedule = FALSE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> %s\n", __FUNCTION__));

	// TODO: shiang-7603
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(%d): Not support for HIF_MT yet!\n",
				__FUNCTION__, __LINE__));
	return 0;

	/*
		process whole rx ring
	*/
	while (1)
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))
			return 0;
		pRxBlk = &RxBlk;
		pRxPacket = GetPacketFromRxRing(pAd, pRxBlk, &bReschedule, &RxPending, 0);
		if ((RxPending == 0) && (bReschedule == FALSE))
			break;
		else
			RELEASE_NDIS_PACKET(pAd, pRxPacket, NDIS_STATUS_SUCCESS);
	}

	/*
		Check DMA Rx idle
	*/
	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &MacReg);
		if (MacReg & 0x8)
		{
			RtmpusecDelay(50);
		}
		else
			break;

#endif


		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s:RX DMA busy!! DMA_CFG = 0x%08x\n", __FUNCTION__, MacReg));
	}

	if (Level == RTMP_HALT)
	{
		/* Disable DMA RX */
#ifdef RTMP_MAC_PCI
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x4);
		HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, MacReg);
#endif
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT MtStopDmaTx(RTMP_ADAPTER *pAd, UCHAR Level)
{
	UINT32 MacReg = 0, MTxCycle = 0;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("====> %s\n", __FUNCTION__));

	// TODO: shiang-7603, shiang-usw
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Not support for HIF_MT yet!\n", __FUNCTION__));
	return 0;

	for (MTxCycle = 0; MTxCycle < 2000; MTxCycle++)
	{
#ifdef RTMP_MAC_PCI
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &MacReg);
		if ((MacReg & 0x2) == 0)
		{
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("==>  DMA Tx Idle, MacReg=0x%x\n", MacReg));
			break;
		}
#endif


		if (MacReg == 0xFFFFFFFF)
		{
			RTMP_SET_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST);
			return 0;
		}
	}

	if (MTxCycle >= 2000)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("TX DMA busy!! DMA_CFG(%x)\n", MacReg));
	}

	if (Level == RTMP_HALT)
	{
#ifdef RTMP_MAC_PCI
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &MacReg);
		MacReg &= ~(0x00000001);
		HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, MacReg);
#endif

	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("<==== %s\n", __FUNCTION__));

	return 0;
}


INT MtAsicSetTxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums)
{
	UINT32 Value;

	MAC_IO_READ32(pAd, TMAC_TCR, &Value);

	Value &= ~TMAC_TCR_TX_STREAM_NUM_MASK;
	Value |= TMAC_TCR_TX_STREAM_NUM(StreamNums - 1);

	MAC_IO_WRITE32(pAd, TMAC_TCR, Value);

	return TRUE;
}


INT MtAsicSetRxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums)
{
	UINT32 Value, Mask = 0;
	INT Ret = TRUE;

	MAC_IO_READ32(pAd, RMAC_RMCR, &Value);

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

		MAC_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	return Ret;
}


INT MtAsicSetBW(RTMP_ADAPTER *pAd, INT bw)
{
	UINT32 val;

	MAC_IO_READ32(pAd, AGG_BWCR, &val);
	val &= (~0x0c);
	switch (bw)
	{
		case BW_20:
			val |= (0);
			break;
		case BW_40:
			val |= (0x1 << 2);
			break;
		case BW_80:
			val |= (0x2 << 2);
			break;
	}
	MAC_IO_WRITE32(pAd, AGG_BWCR, val);

        // TODO: shiang-usw, some CR setting in bbp_set_bw() need to take care!!
	bbp_set_bw(pAd, bw);
	return TRUE;
}


// TODO: shiang-usw, windows compile error due to "MTK_REV_GTE(pAd, MT7603, MT7603E1)" is always true!
INT MtAsicSetRxPath(RTMP_ADAPTER *pAd, UINT32 RxPathSel)
{
	UINT32 Value = 0, Mask = 0;
	INT Ret = TRUE;

	MAC_IO_READ32(pAd, RMAC_RMCR, &Value);

	Value &= ~(RMAC_RMCR_RX_STREAM_0 |
				RMAC_RMCR_RX_STREAM_1 |
				RMAC_RMCR_RX_STREAM_2);

	switch (RxPathSel) {
		case 0: /* ALL */
			Mask = (RMAC_RMCR_RX_STREAM_0 | RMAC_RMCR_RX_STREAM_1 | RMAC_RMCR_RX_STREAM_2);
			if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
				(MTK_REV_LT(pAd, MT7628, MT7628E2)) ||
				(MTK_REV_GTE(pAd, MT76x6, MT76x6E1)))
			{
				PHY_IO_WRITE32(pAd, CR_RXTD_39, 0x0004ba43);
            }
			break;
		case 1: /* RX0 */
			Mask = RMAC_RMCR_RX_STREAM_0;
			if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
				(MTK_REV_LT(pAd, MT7628, MT7628E2)) ||
				(MTK_REV_GTE(pAd, MT76x6, MT76x6E1)))
			{
				PHY_IO_WRITE32(pAd, CR_RXTD_39, 0x0004ba43);
            }
			break;
		case 2: /* RX1 */
			Mask = RMAC_RMCR_RX_STREAM_1;
			if ((MTK_REV_GTE(pAd, MT7603, MT7603E1)) ||
				(MTK_REV_LT(pAd, MT7628, MT7628E2)) ||
				(MTK_REV_GTE(pAd, MT76x6, MT76x6E1)))
			{
				PHY_IO_WRITE32(pAd, CR_RXTD_39, 0x0005ba43);
			}
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

		MAC_IO_WRITE32(pAd, RMAC_RMCR, Value);
	}

	return Ret;
}


#ifdef CONFIG_ATE
INT MtAsicSetTxTonePower(RTMP_ADAPTER *pAd, INT dec0, INT dec1)
{
	UCHAR PowerDec0 = 0;
	UINT32 SetValue = 0;
	ULONG Tempdec1 = 0;
	INT Ret = TRUE;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s-v2 dec0 = %d, dec1 = %d\n",__FUNCTION__,dec0,dec1));
    if( dec0<0 || dec0>0xF || dec1>31 || dec1<-32) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s dec value invalid\n",__FUNCTION__));
	    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,("%s dec0 0~F, dec1 -32~31\n",__FUNCTION__));
        return FALSE;
    }
 	if((MTK_REV_GTE(pAd, MT7603, MT7603E1))||
		(MTK_REV_GTE(pAd, MT7603, MT7603E2))||
		(MTK_REV_GTE(pAd, MT7628, MT7628E1))){
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, for MT7603\n",__FUNCTION__));
		PowerDec0 = dec0&0x0F;
        SetValue = 0x04000000;
        Tempdec1 = 0;

		SetValue |= PowerDec0<<20;

        //RF Gain 1 db
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s RF 1db SetValue = 0x%x\n",__FUNCTION__,SetValue));

		CmdRFRegAccessWrite(pAd, 0, 0x70, SetValue); //bit 26(0x04000000) is enable
		CmdRFRegAccessWrite(pAd, 1, 0x70, SetValue);
        //DC Gain
        if(dec1<0){
			Tempdec1 = (0x40+dec1);
        }else{
            Tempdec1 = dec1;
        }
        SetValue = 0x40000000|(Tempdec1<<20);
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s DC 0.25db SetValue = 0x%x\n",__FUNCTION__,SetValue));
   		RTMP_IO_WRITE32(pAd, CR_TSSI_9, SetValue); //0x10D24, bit 30(0x40000000) isenable
   		RTMP_IO_WRITE32(pAd, CR_WF1_TSSI_9, SetValue); //0x11D24, bit 30(0x40000000) isenable
     }else{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, for MT7636\n ",__FUNCTION__));
	 	/* For 7636 FW command */
	 }

    return Ret;
}

INT MtAsicSetRfFreqOffset(RTMP_ADAPTER *pAd, UINT32 FreqOffset)
{
    UINT32 Value = 0;
	INT Ret = TRUE;

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
	}

	if ((1 == WFSelect) || (2 == WFSelect)) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s, Set WF#%d TSSI on\n",__FUNCTION__, WFSelect));
		PHY_IO_WRITE32(pAd,WF1Offset,CRValue);//3
	}

    return Ret;
}


INT MtAsicSetDPD(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR WFSelect)
{
	ULONG CRValue = 0x0;
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


#ifdef CONFIG_QA
UINT32 MtAsicGetRxStat(RTMP_ADAPTER *pAd, UINT type)
{
    UINT32 value = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type:%d\n", __FUNCTION__, type));
    switch (type) {
        case HQA_RX_STAT_MACFCSERRCNT:
            MAC_IO_READ32(pAd,MIB_MSDR4,&value);
            value = (value >> 16) & 0xFFFF; /* [31:16] FCS ERR */
            break;
        case HQA_RX_STAT_MAC_MDRDYCNT:
            MAC_IO_READ32(pAd,MIB_MSDR10,&value);
            break;
        case HQA_RX_STAT_PHY_MDRDYCNT:
            /* [31:16] OFDM [15:0] CCK */
            PHY_IO_READ32(pAd,RO_PHYCTRL_STS5,&value);
            break;
        case HQA_RX_STAT_PHY_FCSERRCNT:
            /* [31:16] OFDM [15:0] CCK */
            PHY_IO_READ32(pAd,RO_PHYCTRL_STS4,&value);
            break;
        case HQA_RX_STAT_PD:
            /* [31:16] OFDM [15:0] CCK */
            PHY_IO_READ32(pAd,RO_PHYCTRL_STS0,&value);
            break;
        case HQA_RX_STAT_CCK_SIG_SFD:
            /* [31:16] SIG [15:0] SFD */
            PHY_IO_READ32(pAd,RO_PHYCTRL_STS1,&value);
            break;
        case HQA_RX_STAT_OFDM_SIG_TAG:
            /* [31:16] SIG [15:0] TAG */
            PHY_IO_READ32(pAd,RO_PHYCTRL_STS2,&value);
            break;
        case HQA_RX_STAT_RSSI:
            /*[31:24]IBRSSI0 [23:16]WBRSSI0 [15:8]IBRSSI1 [7:0]WBRSSI1*/
            PHY_IO_READ32(pAd,RO_AGC_DEBUG_2,&value);
            break;
        case HQA_RX_RESET_PHY_COUNT:
            PHY_IO_READ32(pAd,CR_PHYCTRL_2,&value);
            value |= (1<<6); /* BIT6: CR_STSCNT_RST */
            PHY_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
            value &= (~(1<<6));
            PHY_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
            value |= (1<<7); /* BIT7: CR_STSCNT_EN */
            PHY_IO_WRITE32(pAd,CR_PHYCTRL_2,value);
            break;
		case HQA_RX_RESET_MAC_COUNT:
            MAC_IO_READ32(pAd,MIB_MSDR4,&value);
			MAC_IO_READ32(pAd,MIB_MSDR10,&value);
	        PHY_IO_READ32(pAd,RO_PHYCTRL_STS5,&value);
			PHY_IO_READ32(pAd,RO_PHYCTRL_STS4,&value);
			break;
        default:
            break;
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, Type(%d):%x\n", __FUNCTION__, type, value));
    return value;
}
#endif /* CONFIG_QA */


INT MtAsicSetTxToneTest(RTMP_ADAPTER *pAd, UINT32 bOnOff, UCHAR Type)
{
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s, bOnOff:%d Type:%d\n", __FUNCTION__, bOnOff, Type));

	if (bOnOff == 0) { /* 0 = off 1 = on */
		//WF0
		PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000000); //0x10000
		PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000000); //0x10004
		PHY_IO_WRITE32(pAd, CR_TSSI_0, 0x80274027); //0x10D00
		PHY_IO_WRITE32(pAd, CR_TSSI_1, 0xC0000800); //0x10D04
		PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x00000008); //0x1420C
		PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000580); //0x14214
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); //0x14704
		PHY_IO_WRITE32(pAd, CR_TSSI_9, 0x00000000); //0x10D24
		PHY_IO_WRITE32(pAd, CR_TXFE_3, 0x00000000); //0x10A08
		PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x00000000); //0x101A0
		PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000); //0x101A4
		PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00000000); //0x1020C
		PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x00000000); //0x10A0C
		PHY_IO_WRITE32(pAd, CR_DPD_CAL_03, 0x20300604); //0x1090C
		//WF1
		PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000000); //0x10000
		PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000000); //0x10004
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, 0x80274027); //0x11D00
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0xC0000800); //0x11D04
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x00000008); //0x1520C
		PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000580); //0x15214
		PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); //0x14704
		PHY_IO_WRITE32(pAd, CR_WF1_TSSI_9, 0x00000000); //0x11D24
		PHY_IO_WRITE32(pAd, CR_TXFE1_3, 0x00000000); //0x11A08
		PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x00000000); //0x101A0
		PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000); //0x101A4
		PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00000000); //0x1020C
		PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x00000000); //0x11A0C
		PHY_IO_WRITE32(pAd, CR_DPD_CAL_03, 0x20300604); //0x1090C
	}
	else if (bOnOff == 1)
	{
        /* WF0 */
        if (Type == WF0_TX_ONE_TONE_5M || Type == WF0_TX_TWO_TONE_5M ||
            Type == WF0_TX_ONE_TONE_10M || Type == WF0_TX_ONE_TONE_DC) {
            /* 1. clock setup */
            PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000021);
            PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000021);

            /* 2. TX setup */
            PHY_IO_WRITE32(pAd, CR_TSSI_0, 0x00274027);
            PHY_IO_WRITE32(pAd, CR_TSSI_1, 0xC0000400);
            PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x80000008);
            PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597);
            PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x10000000);
            PHY_IO_WRITE32(pAd, CR_TSSI_9, 0x60000000);
            PHY_IO_WRITE32(pAd, CR_TXFE_3, 0xF0000000);

            /* 3. Gen Tone */
            if (Type == WF0_TX_ONE_TONE_5M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C100C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
            } else if (Type == WF0_TX_TWO_TONE_5M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C104C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
            } else if (Type == WF0_TX_ONE_TONE_10M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C101C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00011122);
                PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
            } else if (Type == WF0_TX_ONE_TONE_DC) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C1048);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x07000700);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0);
            } else {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Type = %d error!!!\n",__FUNCTION__, Type));
            }

        } else if (Type == WF1_TX_ONE_TONE_5M || Type == WF1_TX_TWO_TONE_5M ||
                   Type == WF1_TX_ONE_TONE_10M || Type == WF1_TX_ONE_TONE_DC) {
            /* 1. clock setup */
            PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000021);
            PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000021);

            /* 2. TX setup */
            PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, 0x00274027);
            PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0xC0000400);
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80000008);
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000597);
            PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x10000000);
            PHY_IO_WRITE32(pAd, CR_WF1_TSSI_9, 0x60000000);
            PHY_IO_WRITE32(pAd, CR_TXFE1_3, 0xF0000000);

            /* 3. Gen Tone */
            if (Type == WF1_TX_ONE_TONE_5M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C100C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
            } else if (Type == WF1_TX_TWO_TONE_5M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C104C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
            } else if (Type == WF1_TX_ONE_TONE_10M) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C101C);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x00000000);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00011122);
                PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
            } else if (Type == WF1_TX_ONE_TONE_DC) {
                PHY_IO_WRITE32(pAd, CR_TXPTN_00, 0x000C1048);
                PHY_IO_WRITE32(pAd, CR_TXPTN_01, 0x07000700);
                PHY_IO_WRITE32(pAd, CR_RFINTF_03, 0x00010122);
                PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0);
            } else {
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Type = %d error!!!\n",__FUNCTION__, Type));
            }

        } else {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Type = %d error!!!\n",__FUNCTION__, Type));
        }
    } else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s bOnOff = %d error!!!\n",__FUNCTION__, bOnOff));
    }
    return TRUE;
}


INT MtAsicStartContinousTx(RTMP_ADAPTER *pAd, UINT32 PhyMode, UINT32 BW, UINT32 PriCh, UINT32 Mcs, UINT32 WFSel)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;

	UINT32 value = 0;
    UINT32 wf_txgc = 0;
	UINT32 tx_pwr = 0;
	UINT32 rf_pa = 0;
	UINT32 pa_bias = 0;
	UINT32 pa_gc = 0;
	/* Band Edge */
	INT32 gc_thresh = 0;
	INT32 pa_mode = 0;
	INT32 pwr_dbm = 0;
	INT32 gc = 0;	//for 0x0204, 0x1204 [24:20]
	UINT32 cr_hilo = 0;
	UINT32 bit = 0;

	UINT32 i = 0;
	UINT32 reg = 0;
	UINT32 reg1 = 0;
	UINT32 reg2 = 0;
	/* TSSI Patch */
	ATECtrl->Sgi = 0;
	ATECtrl->tssi0_wf0_cr = 0;
	ATECtrl->tssi0_wf1_cr = 0;
	ATECtrl->tssi1_wf0_cr = 0;
	ATECtrl->tssi1_wf1_cr = 0;
	ATECtrl->phy_mux_27 = 0;
	/* Change TSSI Training Time */
	PHY_IO_READ32(pAd, CR_TSSI_0, &(ATECtrl->tssi0_wf0_cr));	//0x0D00
	PHY_IO_READ32(pAd, CR_WF1_TSSI_0, &(ATECtrl->tssi0_wf1_cr));	//0x1D00
	PHY_IO_READ32(pAd, CR_TSSI_1, &(ATECtrl->tssi1_wf0_cr));	//0x0D04
	PHY_IO_READ32(pAd, CR_WF1_TSSI_1, &(ATECtrl->tssi1_wf1_cr));	//0x1D04

	PHY_IO_READ32(pAd, CR_TSSI_6, &reg); /* 0x0D18 */
	PHY_IO_READ32(pAd, CR_TSSI_13, &reg1);	/* 0x1D18 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): 0x0D18: 0x%x, tx_pwr(0x0D34):0x%x\n",	__FUNCTION__, reg, reg1));

	for(i=0; i<2; i++){
		mdelay(200);
		ATECtrl->TxCount = 8;
		ATEOp->StartTx(pAd);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): finish Start Tx\n",__FUNCTION__));
		mdelay(100);
		ATEOp->StopTx(pAd, ATECtrl->Mode);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): finish StOP Tx\n",__FUNCTION__));
	}
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): finish Tx TSSI Train \n",	__FUNCTION__));
	ATEOp->RfRegRead(pAd, WFSel, 0x410, &rf_pa); /* wf?_pa_gc = 0x410[18:16], wf?_pa_bias = 0x410[15:0];*/
	/* Setting GC mask */
	switch(WFSel){
	case 0:
	case 1:
		PHY_IO_READ32(pAd, CR_TSSI_13, &tx_pwr);
		break;
	case 2:
		PHY_IO_READ32(pAd, CR_WF1_TSSI_13, &tx_pwr);
		break;
	default:
		break;
	}
	/* Band Edge */
	pwr_dbm = (((INT32)tx_pwr) >> 24) & 0x0000007f;
	PHY_IO_READ32(pAd, CR_PHYMUX_24, &reg); /* 0x4260 */
	gc_thresh = (INT32)reg;
	pa_mode = (gc_thresh>>24) &0x0000007f; /* Half PA[30:24] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pwr_dbm:%d, pa_mode:%d\n",pwr_dbm, pa_mode));
	if(pwr_dbm >= pa_mode){
		gc = 0x1;
	}else{
		gc = 0x0;
	}

	pa_mode = (gc_thresh>>16) &0x7f; /* Full PA[22:16] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pwr_dbm:%d, pa_mode:%d\n",pwr_dbm, pa_mode));
	if(pwr_dbm >= pa_mode){
		gc = 0x2;
	}else{
		/* Use previous value */
	}

	pa_mode = (gc_thresh>>8) &0x7f; /* Super PA[14:8] */
	pa_mode = (pa_mode&0x40)?(pa_mode-128):pa_mode;
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pwr_dbm:%d, pa_mode:%d\n",pwr_dbm, pa_mode));
	if(pwr_dbm >= pa_mode){
		gc = 0x3;
	}else{
		/* Use previous value */
	}
	pa_mode = (gc_thresh>>1) &0x1; /* TX_PA_DYNA[1] */
	if(pa_mode == 1){
		/* Use previous value */
	}else{
		gc = (gc_thresh>>2) & 0x3;
	}
	/* gc write back to [24:20] */
	gc = ((gc<<2)|0x00000010)<<20;
	PHY_IO_WRITE32(pAd, CR_RFINTF_01, gc); /* 0x0204 */
	PHY_IO_WRITE32(pAd, CR_WF1_RFINTF_01, gc); /* 0x1204 */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): gc[0x0204]: 0x%x,gc_thresh:%x\n",__FUNCTION__, gc, gc_thresh));

	/* Original Flow */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("%s(): Modulation = %d, BW=%d, pri_ch = %d, rate = %d, WFSelect = %d-->\n",
				__FUNCTION__, PhyMode, BW, PriCh, Mcs, WFSel));
	PHY_IO_READ32(pAd, CR_TSSI_6, &reg);
	reg1 = reg;
	PHY_IO_READ32(pAd, CR_WF1_TSSI_6, &reg2);

    PHY_IO_WRITE32(pAd, CR_FR_RST_CTRL, 0xFFFFFFFF); //0x10008
    PHY_IO_WRITE32(pAd, CR_BK_RST_CTRL, 0xFFFFFFFF); //0x14004

	PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL,  0x00000078); 	//0x10000
    PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000078);	//0x10004
	/* Back TSSI CR for restore */
	PHY_IO_READ32(pAd, CR_TSSI_0, &(ATECtrl->tssi0_wf0_cr));	//0x0D00
	PHY_IO_READ32(pAd, CR_WF1_TSSI_0, &(ATECtrl->tssi0_wf1_cr));	//0x1D00
	PHY_IO_READ32(pAd, CR_TSSI_1, &(ATECtrl->tssi1_wf0_cr));	//0x0D04
	PHY_IO_READ32(pAd, CR_WF1_TSSI_1, &(ATECtrl->tssi1_wf1_cr));	//0x1D04
	/* Contiuous Tx power patch */
	PHY_IO_WRITE32(pAd, CR_TSSI_0,		0x024041C0);	//0x0D00
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0,	0x024041C0);	//0x1D00
	PHY_IO_WRITE32(pAd, CR_TSSI_1,		0x23F3F800);	//0x0D04
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, 0x23F3F800);	//0x1D04
	//
 	PHY_IO_WRITE32(pAd, CR_TSSI_6, reg1);	//0x0D18
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_6, reg2);	//0x1D18

    PHY_IO_WRITE32(pAd, CR_FFT_MANU_CTRL, 0x0AA00000); //0x10704

    if (BW_20 == BW)
        value = 0x00000000;
    else if (BW_40 == BW)
        value = 0x01000000;
    else if (BW_80 == BW)
        value = 0x02000000;
    else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s BW = %d error!!!\n",__FUNCTION__, BW));

    if (0 == PriCh)
        value |= 0x00000000;
    else if (1 == PriCh)
        value |= 0x00100000;
    else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s pri_ch = %d error!!!\n",__FUNCTION__, PriCh));

    PHY_IO_WRITE32(pAd, CR_PHYCTRL_0, value); //0x14100
    PHY_IO_WRITE32(pAd, CR_PHYCTRL_DBGCTRL, 0x80000030); //0x14140
    PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x80680008); //0x1420C

    switch (WFSel) {
        case 0:
            PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597); //0x14214
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000010); //0x15214
            break;
        case 1:
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80680008); //0x1520C
            PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000590); //0x14214
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000017); //0x15214
            break;
		case 2:

            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_3, 0x80680008); //0x1520C
            PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000597); //0x14214
            PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000017); //0x15214
            break;
        default:
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s WFSelect = %d error!!!\n",__FUNCTION__, WFSel)); /* No ALL?*/
            break;
    }

    if (BW_20 == BW)
        PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x00030000); //0x14700
    else if (BW_40 == BW)
        PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x14030000); //0x14700
    else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s BW = %d error!!!\n",__FUNCTION__, BW));

    if (0 == PhyMode) { /* MODULATION_SYSTEM_CCK */
        value = 0;
        switch (Mcs) {
            case MCS_0:
			case MCS_8:
				value = 0x00000000;
				break;
			case MCS_1:
			case MCS_9:
				value = 0x00200000;
				break;
			case MCS_2:
			case MCS_10:
				value = 0x00400000;
				break;
			case MCS_3:
			case MCS_11:
				value = 0x00600000;
				break;
			default:
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s CCK rate = %d error!!!\n",__FUNCTION__, Mcs));
                break;
        }
        PHY_IO_WRITE32(pAd, CR_TXFD_3, value);
    }
    else if (1 == PhyMode) { /* MODULATION_SYSTEM_OFDM */
		PHY_IO_READ32(pAd, CR_PHYMUX_26, &cr_hilo); /* 0x4268 */
        value = 0;
		switch (Mcs) {
			case MCS_0:
				value = 0x01600000;
				cr_hilo &= BIT(0);
				break;
			case MCS_1:
				value = 0x01E00000;
				cr_hilo &= BIT(1);
				break;
			case MCS_2:
				value = 0x01400000;
				cr_hilo &= BIT(2);
				break;
			case MCS_3:
				value = 0x01C00000;
				cr_hilo &= BIT(3);
				break;
			case MCS_4:
				value = 0x01200000;
				cr_hilo &= BIT(4);
				break;
			case MCS_5:
				value = 0x01900000;
				cr_hilo &= BIT(5);
				break;
			case MCS_6:
				value = 0x01000000;
				cr_hilo &= BIT(6);
				break;
			case MCS_7:
				value = 0x01800000;
				cr_hilo &= BIT(7);
				break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s OFDM rate = %d error!!!\n",__FUNCTION__, Mcs));
				break;
		}
        PHY_IO_WRITE32(pAd, CR_TXFD_3, value); //0x1470C

		/* Patch for HiLo Rate */
		PHY_IO_READ32(pAd, CR_PHYMUX_27, &value); //0x1426C
		ATECtrl->phy_mux_27 = value;
		if(cr_hilo == 0){ //Low Rate
			bit = value & BIT(12);
			value |= bit << 2;
		}else{ //High Rate
			bit = value & BIT(13);
			value |= bit << 1;
		}
		PHY_IO_WRITE32(pAd, CR_PHYMUX_27, value); //0x1426C
    }
    else if (2 == PhyMode || 3 == PhyMode) {
		/* MODULATION_SYSTEM_HT20 || MODULATION_SYSTEM_HT40 */
		PHY_IO_READ32(pAd, CR_PHYMUX_25, &cr_hilo); /* 0x4264 */
        value = 0;
        switch(Mcs) {
			case MCS_0:
				value = 0x00000000;
				cr_hilo &= BIT(0);
				break;
			case MCS_1:
				value = 0x00200000;
				cr_hilo &= BIT(1);
				break;
			case MCS_2:
				value = 0x00400000;
				cr_hilo &= BIT(2);
				break;
			case MCS_3:
				value = 0x00600000;
				cr_hilo &= BIT(3);
				break;
			case MCS_4:
				value = 0x00800000;
				cr_hilo &= BIT(4);
				break;
			case MCS_5:
				value = 0x00A00000;
				cr_hilo &= BIT(5);
				break;
			case MCS_6:
				value = 0x00C00000;
				cr_hilo &= BIT(6);
				break;
			case MCS_7:
				value = 0x00E00000;
				cr_hilo &= BIT(7);
				break;
			case MCS_32:
				value = 0x04000000;
				cr_hilo &= BIT(0);
				break;
			default:
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s OFDM HT MCS = %d error!!!\n",__FUNCTION__, Mcs));
				break;
		}
        PHY_IO_WRITE32(pAd, CR_TXFD_3, value); //0x1470C

		/* Patch for HiLo Rate */
		PHY_IO_READ32(pAd, CR_PHYMUX_27, &value); //0x1426C
		ATECtrl->phy_mux_27 = value;
		if(cr_hilo == 0){ //Low Rate
			bit = value & BIT(12);
			value |= bit << 2;
		}else{ //High Rate
			bit = value & BIT(13);
			value |= bit << 1;
		}
		PHY_IO_WRITE32(pAd, CR_PHYMUX_27, value); //0x1426C
    }
    else {
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Modulation = %d error!!!\n",__FUNCTION__, PhyMode));
    }

	/* load the tag_pwr */
	wf_txgc |= 0x08000000;	/* TXGC_MANUAL_ENABLE */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("tx_pwr:0x%x\n",tx_pwr));

	tx_pwr = (tx_pwr>>4)&0x0FF00000;
	wf_txgc |= tx_pwr;
   	PHY_IO_WRITE32(pAd, CR_PHYMUX_10, wf_txgc);
	PHY_IO_READ32(pAd, CR_PHYMUX_10, &reg);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("tx_pwr:0x%x, wf_txgc:0x%08x, CR_PHYMUX_10:0x%08x\n",tx_pwr , wf_txgc, reg));
	/* Pre-Load Setting to Continuous Tx */
	ATEOp->RfRegRead(pAd, WFSel, 0x0D4, &pa_bias);
	pa_bias |= 0x80000000; /* pa_bias manual enablei */
	ATEOp->RfRegWrite(pAd,WFSel, 0x0D4, pa_bias);
	/* load pa_gc & pa_bias */
	pa_gc |= 0x00008000; /* bit[15] = 1*/
	pa_gc |= (rf_pa&0x0000ffff)<<16; /* bit[31:16] = wf0_pa_gc */
	pa_gc |= (rf_pa>>4)&0x00007000; /* bit[14:12] */
	ATEOp->RfRegWrite(pAd,WFSel, 0x0D8, pa_gc);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,("pa_gc:0x%x,<-- rf_pa:0x%x\n",pa_gc, rf_pa));
    if (0 == WFSel)
        PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0xA0000000); //0x1422C
    else if (1 == WFSel)
        PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0x90000000); //0x1422C
    else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s WFSelect = %d error!!!\n",__FUNCTION__, WFSel)); /* No ALL?*/

    if (0 == PhyMode) /* PREAMBLE_CCK */
        PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x300000F8); //0x14704
    else if (1 == PhyMode) /* PREAMBLE_OFDM */
        PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x310000F2); //0x14704
    else if (2 == PhyMode || 3 == PhyMode) /* PREAMBLE_GREEN_FIELD */
        PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x320000F2); //0x14704
    else
        MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_WARN,("%s Modulation = %d error!!!\n",__FUNCTION__, PhyMode));

    PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x000000C0); //0x10A0C
    PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x000000C0); //0x11A0C
    return 0;
}


INT MtAsicStopContinousTx(RTMP_ADAPTER *pAd)
{
	ATE_CTRL *ATECtrl = &(pAd->ATECtrl);
	ATE_OPERATION *ATEOp = ATECtrl->ATEOp;
	UINT32 val = 0;
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

    PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000005); //0x14704
    PHY_IO_WRITE32(pAd, CR_TXFD_1, 0x00000000); //0x14704
    PHY_IO_WRITE32(pAd, CR_PHYCK_CTRL, 0x00000045); //0x10000
    PHY_IO_WRITE32(pAd, CR_FR_CKG_CTRL, 0x00000045); //0x10004
    PHY_IO_WRITE32(pAd, CR_FFT_MANU_CTRL, 0x00000000); //0x10704
    PHY_IO_WRITE32(pAd, CR_PHYCTRL_0, 0x00000000); //0x14100
    PHY_IO_WRITE32(pAd, CR_PHYCTRL_DBGCTRL, 0x00000000); //0x14140
    PHY_IO_WRITE32(pAd, CR_PHYMUX_3, 0x7C900408); //0x1420C
    PHY_IO_WRITE32(pAd, CR_PHYMUX_5, 0x00000580); //0x14214
    PHY_IO_WRITE32(pAd, CR_PHYMUX_WF1_5, 0x00000000); //0x15214
    PHY_IO_WRITE32(pAd, CR_PHYMUX_10, 0x00000000); //0x14228
    PHY_IO_WRITE32(pAd, CR_PHYMUX_11, 0x00000000); //0x1422C
    PHY_IO_WRITE32(pAd, CR_TXFE_4, 0x00000000); //0x10A0C
    PHY_IO_WRITE32(pAd, CR_TXFE1_4, 0x00000000); //0x11A0C
    PHY_IO_WRITE32(pAd, CR_TXFD_0, 0x00000000); //0x14700
    PHY_IO_WRITE32(pAd, CR_TXFD_3, 0x00000000); //0x1470C
    MAC_IO_WRITE32(pAd, TMAC_PCTSR, 0x00000000); //0x21708
    PHY_IO_WRITE32(pAd, CR_FR_RST_CTRL, 0xFFFFFFFF); //0x10008
    PHY_IO_WRITE32(pAd, CR_BK_RST_CTRL, 0xFFFFFFFF); //0x14004
	ATEOp->RfRegRead(pAd, ATECtrl->TxAntennaSel, 0x0D4, &val);
	val &= 0x7fffffff;
	ATEOp->RfRegWrite(pAd, ATECtrl->TxAntennaSel, 0x0D4, val);
	ATEOp->RfRegRead(pAd, ATECtrl->TxAntennaSel, 0x0D8, &val);
	val &= 0x00000fff;
	ATEOp->RfRegWrite(pAd, ATECtrl->TxAntennaSel, 0x0D8, val);
	/* Restore TSSI CR */
	PHY_IO_WRITE32(pAd, CR_TSSI_0, ATECtrl->tssi0_wf0_cr);	//0x0D00
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_0, ATECtrl->tssi0_wf1_cr);	//0x1D00
	PHY_IO_WRITE32(pAd, CR_TSSI_1, ATECtrl->tssi1_wf0_cr);	//0x0D04
	PHY_IO_WRITE32(pAd, CR_WF1_TSSI_1, ATECtrl->tssi1_wf1_cr);	//0x1D04
	/* Restore Band Edge Patch CR */
	PHY_IO_WRITE32(pAd, CR_RFINTF_01, 0x00000000); /* 0x0204 */
	PHY_IO_WRITE32(pAd, CR_WF1_RFINTF_01, 0x00000000); /* 0x1204 */
	PHY_IO_WRITE32(pAd, CR_PHYMUX_27, ATECtrl->phy_mux_27); //0x1426C
    return TRUE;
}
#endif /* CONFIG_ATE */


#ifdef MAC_APCLI_SUPPORT
/*
	==========================================================================
	Description:
		Set BSSID of Root AP

	IRQL = DISPATCH_LEVEL

	==========================================================================
 */
VOID MtAsicSetApCliBssid(
	IN PRTMP_ADAPTER pAd,
	IN PUCHAR pBssid,
	IN UCHAR index)
{
	UINT32 val;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s(): Set BSSID=%02x:%02x:%02x:%02x:%02x:%02x\n",
				__FUNCTION__, PRINT_MAC(pBssid)));

	val = (UINT32)((pBssid[0]) |
				  (UINT32)(pBssid[1] << 8) |
				  (UINT32)(pBssid[2] << 16) |
				  (UINT32)(pBssid[3] << 24));
	MAC_IO_WRITE32(pAd, RMAC_CB1R0, val);

	val = (UINT32)(pBssid[4]) | (UINT32)(pBssid[5] << 8) | (1 <<16);
	MAC_IO_WRITE32(pAd, RMAC_CB1R1, val);

	return;
}
#endif /* MAC_APCLI_SUPPORT */


VOID MtAsicSetRxGroup(RTMP_ADAPTER *pAd, UINT32 Port, UINT32 Group, BOOLEAN Enable)
{
	UINT32 Value;

	if (Port == HIF_PORT)
	{
		// TODO: shiang-usw, why we use this MARCO here? Cannot use RTMP_IO_READ32()??
		RTMP_MCU_IO_READ32(pAd, RXINF, &Value);

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

		RTMP_MCU_IO_WRITE32(pAd, RXINF, Value);
	}
	 else if (Port == MCU_PORT)
	{
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
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("illegal port (%d\n", Port));
	}
}


#ifdef DMA_SCH_SUPPORT
static CHAR *dma_sch_str[] = {
	"LMAC",
	"ByPass",
	"HyBrid",
	};


/*
    DMA scheduer reservation page assignment
	Q0~Q3: WMM1
	Q4: Management queue
	Q5: MCU CMD
	Q7: Beacon
	Q8: MC/BC
	Q9~Q12: WMM2
	Q13: Management queue
*/
#define MAX_BEACON_SIZE		512
#define MAX_BMCAST_SIZE		1536
#define MAX_BMCAST_COUNT	3
#define MAX_MCUCMD_SIZE	4096 /*must >= MAX_DATA_SIZE */
#define MAX_DATA_SIZE		1792 /* 0xe*128=1792 */
#define MAX_AMSDU_DATA_SIZE	4096
/*
	HYBRID Mode: DMA scheduler would ignore the tx op time information from LMAC, and also use FFA and RSV for enqueue cal.
	BYPASS Mode: Only for Firmware download
*/
INT32 MtAsicDMASchedulerInit(RTMP_ADAPTER *pAd, INT mode)
{
	UINT32 mac_val;
	UINT32 page_size = 128;
	UINT32 page_cnt = 0x1ae;
	INT dma_mode = mode;
#ifdef RTMP_MAC_PCI
	UINT32 mac_restore_val;
#endif /* RTMP_MAC_PCI */

#ifdef CONFIG_FPGA_MODE
	dma_mode = pAd->fpga_ctl.dma_mode;
#endif /* CONFIG_FPGA_MODE */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): DMA Scheduler Mode=%d(%s)\n",
                                        __FUNCTION__, dma_mode,
                                        dma_sch_str[dma_mode]));

#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
	page_size = 256;
#endif /* MT7603_FPGA */

#if defined(MT7636_FPGA)
	page_size = 128;
#endif /* MT7636_FPGA */

	/* Got PSE P0 MAX Quota */
#ifdef RTMP_MAC_PCI
	HW_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
	HW_IO_READ32(pAd, 0x80120, &mac_val);
	page_cnt = (mac_val & 0x0fff0000) >> 16;
	HW_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#endif /* RTMP_MAC_PCI */

	/* Setting PSE page free priority,P0(HIF) > P1(MCU) >  P2Q2(TXS) >  P2Q1(RXV) =  P2Q0(Rxdata)*/
	mac_val = 0x00004037;
	HIF_IO_WRITE32(pAd, FC_FRP,mac_val);

	if (dma_mode == DMA_SCH_BYPASS)
	{
		HIF_IO_WRITE32(pAd, MT_SCH_REG_4, 1<<5);

		/* Disable DMA scheduler */
		MAC_IO_READ32(pAd, AGG_DSCR1, &mac_val);
		mac_val |= 0x80000000;
		MAC_IO_WRITE32(pAd, AGG_DSCR1, mac_val);

#ifdef RTMP_MAC_PCI
		/*
			Wei-Guo's comment:
			2DW/7DW => 0x800C_006C[14:12] = 3'b0
			3DW/8DW =>0x800C_006C[14:12] = 3'b1
		*/
		// In FPGA mode, we need to change tx pad by different DMA scheduler setting!
		HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
		HIF_IO_READ32(pAd, 0xc006c, &mac_val);
		mac_val &= (~(7<<12));
		mac_val |= (1<<12);
		HIF_IO_WRITE32(pAd, 0xc006c, mac_val);
		// After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region
		HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_MAC_PCI */

		// work around for un-sync of TxD between HIF and LMAC
		MAC_IO_READ32(pAd, DMA_DCR1, &mac_val);
		mac_val &= (~(0x7<<8));
		mac_val |= (0x1<<8);
		MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val);
	}

	if ((dma_mode == DMA_SCH_LMAC) || (dma_mode == DMA_SCH_HYBRID))
	{
		UINT32 max_beacon_page_count = MAX_BEACON_SIZE/page_size;
		UINT32 max_bmcast_page_count = MAX_BMCAST_SIZE/page_size;
		UINT32 max_mcucmd_page_count = MAX_MCUCMD_SIZE/page_size;
		UINT32 max_data_page_count = MAX_DATA_SIZE/page_size;
		UINT32 total_restore_val;
		UINT32 mcu_restore_val;
		UINT32 bcn_restore_val;
		UINT32 mbc_restore_val;

		/* Highest Priority:Q7: Beacon > Q8: MC/BC > Q5: MCU CMD */
		mac_val = 0x55555555;
		HIF_IO_WRITE32(pAd, MT_HIGH_PRIORITY_1, mac_val);
		mac_val = 0x78555555;
		HIF_IO_WRITE32(pAd, MT_HIGH_PRIORITY_2, mac_val);

		/* Queue Priority */
		mac_val = 0x2b1a096e;
		HIF_IO_WRITE32(pAd, MT_QUEUE_PRIORITY_1, mac_val);
		mac_val = 0x785f4d3c;
		HIF_IO_WRITE32(pAd, MT_QUEUE_PRIORITY_2, mac_val);

		HIF_IO_WRITE32(pAd, MT_PRIORITY_MASK, 0xffffffff);

		/* Schedule Priority, page size/FFA, FFA = (page_cnt * page_size) */
		mac_val = (2 << 28) | (page_cnt);
		HIF_IO_WRITE32(pAd, MT_SCH_REG_1, mac_val);
		mac_val = MAX_DATA_SIZE / page_size;
		HIF_IO_WRITE32(pAd, MT_SCH_REG_2, mac_val);

		/* Resvervation page */
		if(pAd->CommonCfg.BACapability.field.AmsduEnable==TRUE)
			max_data_page_count = MAX_AMSDU_DATA_SIZE/page_size;

		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_0, max_data_page_count);
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_1, max_data_page_count);
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_2, max_data_page_count);
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_3, max_data_page_count);
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_4, max_data_page_count);
		total_restore_val = max_data_page_count*5;

		mcu_restore_val = max_mcucmd_page_count;
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_5, mcu_restore_val);
		total_restore_val += mcu_restore_val;

#ifdef CONFIG_AP_SUPPORT
		if (pAd->ApCfg.BssidNum  > 1)
		{
			bcn_restore_val = max_beacon_page_count*(pAd->ApCfg.BssidNum - 1) + max_data_page_count;
		} else {
			bcn_restore_val = max_data_page_count;
		}
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_7, bcn_restore_val);
#endif /* CONFIG_AP_SUPPORT */
#ifdef CONFIG_STA_SUPPORT
		bcn_restore_val = max_data_page_count;
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_7, bcn_restore_val);
#endif /* CONFIG_STA_SUPPORT */
		total_restore_val += bcn_restore_val;

		mbc_restore_val = max_bmcast_page_count*MAX_BMCAST_COUNT + max_data_page_count;
		HIF_IO_WRITE32(pAd, MT_PAGE_CNT_8, mbc_restore_val);
		total_restore_val += mbc_restore_val;

		/* FFA1 max threshold */
		HIF_IO_WRITE32(pAd, MT_RSV_MAX_THD, (page_cnt - total_restore_val));

		/* Group Threshold */
		HIF_IO_WRITE32(pAd, MT_GROUP_THD_0, page_cnt);
		HIF_IO_WRITE32(pAd, MT_BMAP_0, 0xffff);

		if (dma_mode == DMA_SCH_LMAC)
		{ 	/* config as LMAC prediction mode */
			HIF_IO_WRITE32(pAd, MT_SCH_REG_4, 0x0);
		}

		if (dma_mode == DMA_SCH_HYBRID)
		{	/* config as hybrid mode */
			HIF_IO_WRITE32(pAd, MT_SCH_REG_4, 1<<6);
#if defined(MT7603_FPGA) || defined(MT7628_FPGA)
#ifdef RTMP_PCI_SUPPORT
			/*			Wei-Guo's comment:
						2DW/7DW => 0x800C_006C[14:12] = 3'b0
						3DW/8DW =>0x800C_006C[14:12] = 3'b1
			*/
			// In FPGA mode, we need to change tx pad by different DMA scheduler setting
			HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
			HIF_IO_READ32(pAd, 0xc006c, &mac_val);
			mac_val &= (~(7<<12));
			HIF_IO_WRITE32(pAd, 0xc006c, mac_val);
			// After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region
			HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_PCI_SUPPORT */
#endif /* MT7603_FPGA */

			// Disable TxD padding
			MAC_IO_READ32(pAd, DMA_DCR1, &mac_val);
			mac_val &= (~(0x7<<8));
			MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val);
		}
	}

	//if (MTK_REV_GTE(pAd, MT7603,MT7603E1)) {
		mac_val = 0xfffff;
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_0, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_1, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_2, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_3, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_4, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_5, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_6, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_7, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_8, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_9, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_10, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_11, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_12, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_13, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_14, mac_val);
		HIF_IO_WRITE32(pAd, MT_TXTIME_THD_15, mac_val);
		HIF_IO_READ32(pAd, MT_SCH_REG_4, &mac_val);
		mac_val |= 0x40;
		HIF_IO_WRITE32(pAd, MT_SCH_REG_4, mac_val);
	//}

#if defined(MT7636_FPGA)
	// Strack, pwer on PSE memory power
	HIF_IO_WRITE32(pAd, MT_FPGA_PSE_SET_0, 0xffffffff);
	HIF_IO_WRITE32(pAd, MT_FPGA_PSE_SET_1, 0xffffffff);
	HIF_IO_READ32(pAd, MT_FPGA_PSE_CLIENT_CNT, &mac_val);
	mac_val |= (1 << 1);
	HIF_IO_WRITE32(pAd, MT_FPGA_PSE_CLIENT_CNT, mac_val);
#endif /* #if defined(MT7603_FPGA) */

	return TRUE;
}
#endif /* DMA_SCH_SUPPORT */


VOID MtAsicSetBARTxCntLimit(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count)
{
	UINT32 Value;

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

VOID MtAsicSetRTSTxCntLimit(RTMP_ADAPTER *pAd, BOOLEAN Enable, UINT32 Count)
{
	UINT32 Value, RTS_retry;

	RTMP_IO_READ32(pAd, AGG_MRCR, &Value);

	RTS_retry = GET_RTS_RTY_CNT_LIMIT(Value);	
	if (!Enable)
		Count = 0;

	if (RTS_retry != Count) {
		Value &= ~RTS_RTY_CNT_LIMIT_MASK;
		Value |= RTS_RTY_CNT_LIMIT(Count);
		RTMP_IO_WRITE32(pAd, AGG_MRCR, Value);
	}
}

VOID MtAsicSetTxSClassifyFilter(RTMP_ADAPTER *pAd, UINT32 Port, UINT8 DestQ,
								UINT32 AggNums, UINT32 Filter)
{
	UINT32 Value;

	if (Port == TXS2HOST)
	{
		MAC_IO_READ32(pAd, DMA_TCFR1, &Value);
		Value &= ~TXS2H_BIT_MAP_MASK;
		Value |= TXS2H_BIT_MAP(Filter);
		Value &= ~TXS2H_AGG_CNT_MASK;
		Value |= TXS2H_AGG_CNT(AggNums);
		if (DestQ == 0)
			Value &= ~TXS2H_QID;
		else
			Value |= TXS2H_QID;
		MAC_IO_WRITE32(pAd, DMA_TCFR1, Value);

	}
	else if (Port == TXS2MCU)
	{
		MAC_IO_READ32(pAd, DMA_TCFR0, &Value);
		Value &= ~TXS2M_BIT_MAP_MASK;
		Value |= TXS2M_BIT_MAP(Filter);
		Value &= ~TXS2M_AGG_CNT_MASK;
		Value |= TXS2M_AGG_CNT(AggNums);
		Value &= ~TXS2M_QID_MASK;
		Value |= TXS2M_QID(DestQ);
		MAC_IO_WRITE32(pAd, DMA_TCFR0, Value);
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Unknow Port(%d)\n", __FUNCTION__, Port));
	}
}


/*
 * Init TxD short format template which will copy by PSE-Client to LMAC
 */
static INT MtAsicSetTmacInfoTemplate(RTMP_ADAPTER *pAd)
{
	UINT32 dw[5];
	TMAC_TXD_2 *dw2 = (TMAC_TXD_2 *)(&dw[0]);
	TMAC_TXD_3 *dw3 = (TMAC_TXD_3 *)(&dw[1]);
	TMAC_TXD_5 *dw5 = (TMAC_TXD_5 *)(&dw[3]);

	NdisZeroMemory((UCHAR *)(&dw[0]), sizeof(dw));

	dw2->htc_vld = 0;
	dw2->frag = 0;
	dw2->max_tx_time = 0;
	dw2->fix_rate = 0;

	dw3->remain_tx_cnt = MT_TX_SHORT_RETRY;
	dw3->sn_vld = 0;
	dw3->pn_vld = 0;

	dw5->pid = PID_DATA_AMPDU;
	dw5->tx_status_fmt = 0;
	dw5->tx_status_2_host = 0; // Disable TxS
	dw5->bar_sn_ctrl = 0; //HW
#if defined(CONFIG_STA_SUPPORT) && defined(MT7628)
    dw5->pwr_mgmt= TMI_PM_BIT_CFG_BY_SW; // SW
#else
	dw5->pwr_mgmt = TMI_PM_BIT_CFG_BY_HW; // HW
#endif /* defined(CONFIG_STA_SUPPORT) && defined(MT7628) */

#ifdef RTMP_MAC_PCI
	HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, 0x80080000);
#endif /* RTMP_PCI_SUPPORT */

	/* For short format */
	// TODO: shiang-usw, for windows, need to change the CR mapping table for 0xc0000 to 0x800c0000
	HIF_IO_WRITE32(pAd, 0xc0040, dw[0]);
	HIF_IO_WRITE32(pAd, 0xc0044, dw[1]);
	HIF_IO_WRITE32(pAd, 0xc0048, dw[2]);
	HIF_IO_WRITE32(pAd, 0xc004c, dw[3]);
	HIF_IO_WRITE32(pAd, 0xc0050, dw[4]);

#ifdef RTMP_MAC_PCI
	// After change the Tx Padding CR of PCI-E Client, we need to re-map for PSE region
	HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_PSE_BASE_ADDR);
#endif /* RTMP_PCI_SUPPORT */

	return TRUE;
}


VOID MtAsicInitMac(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	/* Preparation of TxD DW2~DW6 when we need run 3DW format */
	MtAsicSetTmacInfoTemplate(pAd);

	/* A-MPDU BA WinSize control */
	MAC_IO_READ32(pAd, AGG_AWSCR, &mac_val);
	mac_val &= ~WINSIZE0_MASK;
	mac_val |= WINSIZE0(4);
	mac_val &= ~WINSIZE1_MASK;
	mac_val |= WINSIZE1(5);
	mac_val &= ~WINSIZE2_MASK;
	mac_val |= WINSIZE2(8);
	mac_val &= ~WINSIZE3_MASK;
	mac_val |= WINSIZE3(10);
	MAC_IO_WRITE32(pAd, AGG_AWSCR, mac_val);

	MAC_IO_READ32(pAd, AGG_AWSCR1, &mac_val);
	mac_val &= ~WINSIZE4_MASK;
	mac_val |= WINSIZE4(16);
	mac_val &= ~WINSIZE5_MASK;
	mac_val |= WINSIZE5(20);
	mac_val &= ~WINSIZE6_MASK;
	mac_val |= WINSIZE6(21);
	mac_val &= ~WINSIZE7_MASK;
	mac_val |= WINSIZE7(42);
	MAC_IO_WRITE32(pAd, AGG_AWSCR1, mac_val);

	/* A-MPDU Agg limit control */
	MAC_IO_READ32(pAd, AGG_AALCR, &mac_val);
	mac_val &= ~AC0_AGG_LIMIT_MASK;
	mac_val |= AC0_AGG_LIMIT(21);
	mac_val &= ~AC1_AGG_LIMIT_MASK;
	mac_val |= AC1_AGG_LIMIT(21);
	mac_val &= ~AC2_AGG_LIMIT_MASK;
	mac_val |= AC2_AGG_LIMIT(21);
	mac_val &= ~AC3_AGG_LIMIT_MASK;
	mac_val |= AC3_AGG_LIMIT(21);
	MAC_IO_WRITE32(pAd, AGG_AALCR, mac_val);

	MAC_IO_READ32(pAd, AGG_AALCR1, &mac_val);
	mac_val &= ~AC10_AGG_LIMIT_MASK;
	mac_val |= AC10_AGG_LIMIT(21);
	mac_val &= ~AC11_AGG_LIMIT_MASK;
	mac_val |= AC11_AGG_LIMIT(21);
	mac_val &= ~AC12_AGG_LIMIT_MASK;
	mac_val |= AC12_AGG_LIMIT(21);
	mac_val &= ~AC13_AGG_LIMIT_MASK;
	mac_val |= AC13_AGG_LIMIT(21);
	MAC_IO_WRITE32(pAd, AGG_AALCR1, mac_val);

	/* Vector report queue setting */
	MAC_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val |= BIT13;
	MAC_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

	/* TMR report queue setting */
	MAC_IO_READ32(pAd, DMA_TMCFR0, &mac_val);
	mac_val |= BIT13;//TMR report send to HIF q1.
	mac_val = mac_val & ~(BIT0);
	mac_val = mac_val & ~(BIT1);
	MAC_IO_WRITE32(pAd, DMA_TMCFR0, mac_val);

	/* Configure all rx packets to HIF, except WOL2M packet */
	MAC_IO_READ32(pAd, DMA_RCFR0, &mac_val);
	mac_val = 0x00010000; // drop duplicate
	mac_val |= 0xc0200000; // receive BA/CF_End/Ack/RTS/CTS/CTRL_RSVED
	if (pAd->rx_pspoll_filter)
		mac_val |= 0x00000008; //Non-BAR Control frame to MCU
	MAC_IO_WRITE32(pAd, DMA_RCFR0, mac_val);

	/* Configure Rx Vectors report to HIF */
	MAC_IO_READ32(pAd, DMA_VCFR0, &mac_val);
	mac_val &= (~0x1); // To HIF
	mac_val |= 0x2000; // RxRing 1
	MAC_IO_WRITE32(pAd, DMA_VCFR0, mac_val);

    	/* RMAC dropping criteria for max/min recv. packet length */
    	MAC_IO_READ32(pAd, RMAC_RMACDR, &mac_val);
    	mac_val |= SELECT_RXMAXLEN_20BIT;
    	MAC_IO_WRITE32(pAd, RMAC_RMACDR, mac_val);

	MAC_IO_READ32(pAd, RMAC_MAXMINLEN, &mac_val);
	mac_val &= ~RMAC_DROP_MAX_LEN_MASK;
    	mac_val |= RMAC_DROP_MAX_LEN;
	MAC_IO_WRITE32(pAd, RMAC_MAXMINLEN, mac_val);

    // TODO: shiang-usw, fix me for this function
	/* Enable RX Group to HIF */
	MtAsicSetRxGroup(pAd, HIF_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);
	MtAsicSetRxGroup(pAd, MCU_PORT, RXS_GROUP1|RXS_GROUP2|RXS_GROUP3, TRUE);

	/* AMPDU BAR setting */
	/* Enable HW BAR feature */
	MtAsicSetBARTxCntLimit(pAd, TRUE, 1);

	/* RTS retry setting */
	MtAsicSetRTSTxCntLimit(pAd, TRUE, MT_RTS_RETRY);

	/* Configure the BAR rate setting */
	MAC_IO_READ32(pAd, AGG_ACR, &mac_val);
	mac_val &= (~0xfff00000);
	mac_val &= ~(AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK|AMPDU_NO_BA_RULE);
	mac_val |= AGG_ACR_AMPDU_NO_BA_AR_RULE_MASK;
	MAC_IO_WRITE32(pAd, AGG_ACR, mac_val);

	/* AMPDU Statistics Range Control setting
		0 < agg_cnt - 1 <= range_cr(0),				=> 1
		range_cr(0) < agg_cnt - 1 <= range_cr(4),		=> 2~5
		range_cr(4) < agg_cnt - 1 <= range_cr(14),	=> 6~15
		range_cr(14) < agg_cnt - 1,					=> 16~
	*/
	MAC_IO_READ32(pAd, AGG_ASRCR, &mac_val);
	mac_val =  (0 << 0) | (4 << 8) | (14 << 16);
	MAC_IO_WRITE32(pAd, AGG_ASRCR, mac_val);

	// Enable MIB counters
	MAC_IO_WRITE32(pAd, MIB_MSCR, 0x7fffffff);
	MAC_IO_WRITE32(pAd, MIB_MPBSCR, 0xffffffff);

	/* CCA Setting */
	MAC_IO_READ32(pAd, TMAC_TRCR, &mac_val);
	mac_val &= ~CCA_SRC_SEL_MASK;
	mac_val |= CCA_SRC_SEL(0x2);
	mac_val &= ~CCA_SEC_SRC_SEL_MASK;
	mac_val |= CCA_SEC_SRC_SEL(0x0);
	MAC_IO_WRITE32(pAd, TMAC_TRCR, mac_val);

	/* RCPI include ACK and Data */
	MAC_IO_READ32(pAd, WTBL_OFF_RMVTCR, &mac_val);
	mac_val |= RX_MV_MODE;
	MAC_IO_WRITE32(pAd, WTBL_OFF_RMVTCR, mac_val);

	/* Turn on RX RIFS Mode */
	MAC_IO_READ32(pAd, TMAC_TCR, &mac_val);
	mac_val |= RX_RIFS_MODE;
	MAC_IO_WRITE32(pAd, TMAC_TCR, mac_val);

	/* IOT issue with Realtek at CCK mode */
	mac_val = 0x003000E7;
	MAC_IO_WRITE32(pAd, TMAC_CDTR, mac_val);

	/* IOT issue with Linksys WUSB6300. Cannot receive BA after TX finish */
	mac_val = 0x4;
	MAC_IO_WRITE32(pAd, TMAC_RRCR, mac_val);

	/* send RTS/CTS if agg size >= 2 */
	MAC_IO_READ32(pAd, AGG_PCR1, &mac_val);
	mac_val &= ~RTS_PKT_NUM_THRESHOLD_MASK;
	mac_val |= RTS_PKT_NUM_THRESHOLD(3);
	MAC_IO_WRITE32(pAd, AGG_PCR1, mac_val);

	/* When WAPI + RDG, don't mask ORDER bit  */
	MAC_IO_READ32(pAd, SEC_SCR, &mac_val);
	mac_val &= 0xfffbfffc; //zero bit [18] for ICV error issue.	
	MAC_IO_WRITE32(pAd, SEC_SCR, mac_val);

	/* Enable Spatial Extension for RTS/CTS  */
	MAC_IO_READ32(pAd, TMAC_PCR, &mac_val);
	mac_val |= PTEC_SPE_EN;
	MAC_IO_WRITE32(pAd, TMAC_PCR, mac_val);

	/* Enable Spatial Extension for ACK/BA/CTS */
	MAC_IO_READ32(pAd, TMAC_B0BRR0, &mac_val);
	mac_val |= BSSID00_RESP_SPE_EN;
	MAC_IO_WRITE32(pAd, TMAC_B0BRR0, mac_val);

    if (MTK_REV_GTE(pAd, MT7628, MT7628E2)){
        MAC_IO_READ32(pAd, TMAC_TCR, &mac_val);
        mac_val |= BIT28;//WHQA_00018120
        mac_val |= BIT1;//WHQA_00018121
        mac_val |= BIT0;//WHQA_00018121
        MAC_IO_WRITE32(pAd, TMAC_TCR, mac_val);

        MAC_IO_READ32(pAd, TMAC_TRCR, &mac_val);
        mac_val |= BIT27;//WHQA_00018120
        MAC_IO_WRITE32(pAd, TMAC_TRCR, mac_val);

        MAC_IO_READ32(pAd, AGG_SCR, &mac_val);
        mac_val |= BIT2;//WHQA_00018106
        mac_val |= BIT3;//WHQA_00018107
        mac_val |= BIT4;//WHQA_00018344
        MAC_IO_WRITE32(pAd, AGG_SCR, mac_val);
    }
}

VOID MtAsicSetRxPspollFilter(RTMP_ADAPTER *pAd, CHAR enable)
{
	UINT32 mac_val;
	
	MAC_IO_READ32(pAd, DMA_RCFR0, &mac_val);
	if (enable)
		mac_val |= 0x00000008; //Non-BAR Control frame to MCU
	else
		mac_val &= 0xfffffff7; //Non-BAR Control frame to HIF
		
	MAC_IO_WRITE32(pAd, DMA_RCFR0, mac_val);

}

#if defined(MT7603) || defined(MT7628)
INT32 MtAsicGetThemalSensor(RTMP_ADAPTER *pAd, CHAR type)
{
	/* 0: get temperature; 1: get adc */
	/* Get Thermal sensor adc cal value: 0x80022000 bits(8,14)	*/
	INT32 result=0;
	
	if ((type == 0) || (type == 1)) {
		UINT32 mac_val;
#ifdef RTMP_PCI_SUPPORT
		UINT32 mac_restore_val;
	
		HIF_IO_READ32(pAd, MCU_PCIE_REMAP_2, &mac_restore_val);
		HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, MT_TOP_REMAP_ADDR);
		HIF_IO_READ32(pAd, MT_TOP_REMAP_ADDR_THEMAL, &mac_val);
		result = (mac_val & 0x00007f00) >> 8;
		HIF_IO_WRITE32(pAd, MCU_PCIE_REMAP_2, mac_restore_val);
#endif
		if (type == 0) {
			INT32 g_ucThermoRefAdcVal, g_cThermoSlopeVariation, g_cThermoRefOffset;
	
			if (pAd->EEPROMImage[TEMPERATURE_SENSOR_CALIBRATION] & 0x80)
					g_ucThermoRefAdcVal = pAd->EEPROMImage[TEMPERATURE_SENSOR_CALIBRATION] & THERMO_REF_ADC_VARIATION_MASK;
			else
					g_ucThermoRefAdcVal = 52;
	
			if (pAd->EEPROMImage[THADC_ANALOG_PART] & 0x80) {
				g_cThermoSlopeVariation = pAd->EEPROMImage[THADC_SLOP] & THERMO_SLOPE_VARIATION_MASK;
				if (g_cThermoSlopeVariation > 16)
					g_cThermoSlopeVariation -= 32;
			} else
					g_cThermoSlopeVariation = 0;
				
			g_cThermoRefOffset = pAd->EEPROMImage[THERMAL_COMPENSATION_OFFSET] + 28;
			result = (((result - g_ucThermoRefAdcVal) * (56 + g_cThermoSlopeVariation) )/30) + g_cThermoRefOffset;
		}
	}

	return result;
}

/*
  *  ucation: 0: stop; 1: flush; 2: start
  */
VOID MtAsicACQueue(RTMP_ADAPTER *pAd, UINT8 ucation, UINT8 BssidIdx, UINT32 u4AcQueueMap)
{
	UINT32 ACQCR_0 = 0, ACQCR_1 = 0;
	UINT32 Value_0 = 0, Value_1 = 0;
	UINT8 ucQueueIdx;

	if (ucation > 2)
		return;

	switch (ucation)
	{
		case AC_QUEUE_STOP:
			ACQCR_0 = ARB_TQCR4;
			ACQCR_1 = ARB_TQCR5;
			break;
		case AC_QUEUE_FLUSH:
			ACQCR_0 = ARB_TQCR2;
			ACQCR_1 = ARB_TQCR3;
			break;
		case AC_QUEUE_START:
			ACQCR_0 = ARB_TQCR0;
			ACQCR_1 = ARB_TQCR1;
			break;
	}

	for (ucQueueIdx = 0; ucQueueIdx < 14; ucQueueIdx++)
	{
	        if (u4AcQueueMap & (1 << ucQueueIdx)) {
			switch (ucQueueIdx)
	{
				case 0:
				case 1:
				case 2:
					Value_0 |= (1 << (ucQueueIdx*5 + BssidIdx));
					break;
				case 3:
				case 4:
				case 5:
					Value_0 |= (1 << (ucQueueIdx*5 + BssidIdx + 1));
					break;
				case 6:
					Value_1 |= (1 << (BssidIdx + 26));
					break;
				case 10:
				case 11:
				case 12:
					Value_1 |= (1 << ((ucQueueIdx - 10)*5 + BssidIdx));
					break;
				case 13:
				case 14:
					Value_1 |= (1 << ((ucQueueIdx - 10)*5 + BssidIdx + 1));
					break;
			}
 		}
	}

	if (ACQCR_0 && Value_0) {
		RTMP_IO_WRITE32(pAd, ACQCR_0, Value_0);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Write CR:%x, Value=%x\n", __FUNCTION__, ACQCR_0, Value_0));
	}
	
	if (ACQCR_1 && Value_1) {
		RTMP_IO_WRITE32(pAd, ACQCR_1, Value_1);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: Write CR:%x, Value=%x\n", __FUNCTION__, ACQCR_1, Value_1));
	}
}
#endif /* MT7603 ||MT7628  */




