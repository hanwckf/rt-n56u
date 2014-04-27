/*

*/


#include "rt_config.h"


#ifdef RTMP_MAC

/* BBP register initialization set*/
REG_PAIR   BBPRegTable[] = {
	{BBP_R65,		0x2C},		/* fix rssi issue*/
	{BBP_R66,		0x38},	/* Also set this default value to pAd->BbpTuning.R66CurrentValue at initial*/
	{BBP_R68,		0x0B},  /* improve Rx sensitivity. */
	{BBP_R69,		0x12},
	{BBP_R70,		0xa},	/* BBP_R70 will change to 0x8 in ApStartUp and LinkUp for rt2860C, otherwise value is 0xa*/
	{BBP_R73,		0x10},
	{BBP_R81,		0x37},
	{BBP_R82,		0x62},
	{BBP_R83,		0x6A},
	{BBP_R84,		0x99},	/* 0x19 is for rt2860E and after. This is for extension channel overlapping IOT. 0x99 is for rt2860D and before*/
	{BBP_R86,		0x00},	/* middle range issue, Rory @2008-01-28 	*/
	{BBP_R91,		0x04},	/* middle range issue, Rory @2008-01-28*/
	{BBP_R92,		0x00},	/* middle range issue, Rory @2008-01-28*/
	{BBP_R103,		0x00}, 	/* near range high-power issue, requested from Gary @2008-0528*/
	{BBP_R105,		0x05},	/* 0x05 is for rt2860E to turn on FEQ control. It is safe for rt2860D and before, because Bit 7:2 are reserved in rt2860D and before.*/
#ifdef DOT11_N_SUPPORT
	{BBP_R106,		0x35},	/* Optimizing the Short GI sampling request from Gray @2009-0409*/
#endif /* DOT11_N_SUPPORT */
};
#define	NUM_BBP_REG_PARMS	(sizeof(BBPRegTable) / sizeof(REG_PAIR))


NDIS_STATUS NICInitBBP(RTMP_ADAPTER *pAd)
{
	INT Index = 0;
	UCHAR R0 = 0xff;
	
	/* Read BBP register, make sure BBP is up and running before write new data*/
	if (rtmp_bbp_is_ready(pAd)== FALSE)
		return NDIS_STATUS_FAILURE;

	Index = 0;

	/* Initialize BBP register to default value*/
	for (Index = 0; Index < NUM_BBP_REG_PARMS; Index++)
	{

		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd,
				BBPRegTable[Index].Register,
				BBPRegTable[Index].Value);
	}

	/* re-config specific BBP registers for individual chip */
	if (pAd->chipCap.pBBPRegTable)
	{
		REG_PAIR *pbbpRegTb = pAd->chipCap.pBBPRegTable;
		
		for (Index = 0; Index < pAd->chipCap.bbpRegTbSize; Index++)
		{
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd,
					pbbpRegTb[Index].Register,
					pbbpRegTb[Index].Value);
			DBGPRINT(RT_DEBUG_TRACE, ("BBP_R%d=0x%x\n", 
					pbbpRegTb[Index].Register, 
					pbbpRegTb[Index].Value));
		}
	}

	if (pAd->chipOps.AsicBbpInit != NULL)
		pAd->chipOps.AsicBbpInit(pAd);

	/*
		For rt2860E and after, init BBP_R84 with 0x19. This is for extension channel overlapping IOT.
		RT3090 should not program BBP R84 to 0x19, otherwise TX will block.
		3070/71/72,3090,3090A( are included in RT30xx),3572,3390
	*/
	if (((pAd->MACVersion & 0xffff) != 0x0101) &&
		!(IS_RT30xx(pAd)|| IS_RT3572(pAd) || IS_RT5390(pAd) || IS_RT5392(pAd) || IS_RT3290(pAd)))
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R84, 0x19);


	if (pAd->MACVersion == 0x28600100)
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, 0x16);
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, 0x12);
	}

	return NDIS_STATUS_SUCCESS;
	
}


INT rtmp_bbp_get_temp(struct _RTMP_ADAPTER *pAd, CHAR *temp_val)
{
	BBP_R49_STRUC bbp_val;

	bbp_val.byte = 0;
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &bbp_val.byte);
	*temp_val = (CHAR)bbp_val.byte;

	pAd->curr_temp = (bbp_val & 0xff);

	return TRUE;
}


INT 	rtmp_bbp_tx_comp_init(RTMP_ADAPTER *pAd, INT adc_insel, INT tssi_mode)
{
	UCHAR bbp_val, rf_val;

	
	/* Set BBP_R47 */
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &bbp_val);
	bbp_val &= 0xe7;
	bbp_val |= ((tssi_mode << 3) & 0x18);
	bbp_val |= 0x80;
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, bbp_val);

	/*  Set RF_R27 */
	RT30xxReadRFRegister(pAd, RF_R27, &rf_val);
	rf_val &= 0x3f;
	rf_val |= ((adc_insel << 6) & 0xc0);
	RT30xxWriteRFRegister(pAd, RF_R27, rf_val);
	DBGPRINT(RT_DEBUG_TRACE, ("[temp. compensation] Set RF_R27 to 0x%x\n", rf_val));
}


INT rtmp_bbp_set_txdac(struct _RTMP_ADAPTER *pAd, INT tx_dac)
{
	UCHAR val, old_val = 0;

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &old_val);
	val = old_val & (~0x18);
	switch (tx_dac)
	{
		case 2:
			val |= 0x10;
			break;
		case 1:
			val |= 0x08;
			break;
		case 0:
		default:
			break;
	}

	if (val != old_val) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, val);
	}

	return TRUE;
}


INT rtmp_bbp_set_rxpath(struct _RTMP_ADAPTER *pAd, INT rxpath)
{
	UCHAR val = 0;

	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &val);
	val &= (~0x18);
	if(rxpath == 3)
		val |= (0x10);
	else if(rxpath == 2)
		val |= (0x8);
	else if(rxpath == 1)
		val |= (0x0);
	RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, val);

	return TRUE;
}


INT rtmp_bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, INT ext_ch)
{
	UCHAR val, old_val = 0;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &old_val);
	val = old_val;
	switch (ext_ch)
	{
		case EXTCHA_BELOW:
			val |= (0x20);
			break;
		case EXTCHA_ABOVE:
		case EXTCHA_NONE:
			val &= (~0x20);
			break;
	}

	if (val != old_val)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, val);


	return TRUE;
}


INT rtmp_bbp_set_bw(struct _RTMP_ADAPTER *pAd, INT bw)
{
	UCHAR val, old_val = 0;
	BOOLEAN bstop = FALSE;
	UINT32 Data, MTxCycle, macStatus;


	if (bw != pAd->CommonCfg.BBPCurrentBW)
		bstop = TRUE;

	if (bstop)
	{
		/* Disable MAC Tx/Rx */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
		Data &= (~0x0C);
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);

		/* Check MAC Tx/Rx idle */
		for (MTxCycle = 0; MTxCycle < 10000; MTxCycle++)
		{
			RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
			if (macStatus & 0x3)
				RTMPusecDelay(50);
			else
				break;
		}
	}

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &old_val);
	val = (old_val & (~0x18));
	switch (bw)
	{
		case BW_20:
			val &= (~0x18);
			break;
		case BW_40:
			val |= (0x10);
			break;
		case BW_10:
			val |= 0x08;
			break;	
	}

	if (val != old_val) {
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, val);
	}

	if (bstop)
	{
		/* Enable MAC Tx/Rx */
		RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &Data);
		Data |= 0x0C;
		RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, Data);
	}

	pAd->CommonCfg.BBPCurrentBW = bw;
	
	return TRUE;
}


INT rtmp_bbp_set_mmps(struct _RTMP_ADAPTER *pAd, BOOLEAN ReduceCorePower)
{
	UCHAR bbp_val, org_val;
	
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &org_val);
	bbp_val = org_val;
	if (ReduceCorePower)
		bbp_val |= 0x04;
	else
		bbp_val &= ~0x04;

	if (bbp_val != org_val)
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, bbp_val);

	return TRUE;
}


NDIS_STATUS AsicBBPWriteWithRxChain(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR bbpId,
	IN CHAR bbpVal,
	IN RX_CHAIN_IDX rx_ch_idx)
{
	UCHAR idx = 0, val = 0;

	if (((pAd->MACVersion & 0xf0000000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
		return NDIS_STATUS_SUCCESS;
	}
	
	while (rx_ch_idx != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;
		
		if (rx_ch_idx & 0x01)
		{
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &val);
			val = (val & (~0x60)) | (idx << 5);

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, val);
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, bbpId, bbpVal);
			}
#endif /* RTMP_MAC_PCI */

			DBGPRINT(RT_DEBUG_INFO, 
					("%s(Idx):Write(R%d,val:0x%x) to Chain(0x%x, idx:%d)\n",
						__FUNCTION__, bbpId, bbpVal, rx_ch_idx, idx));
		}
		rx_ch_idx >>= 1;
		idx++;
	}

	return NDIS_STATUS_SUCCESS;
}


NDIS_STATUS AsicBBPReadWithRxChain(
	IN RTMP_ADAPTER *pAd, 
	IN UCHAR bbpId, 
	IN CHAR *pBbpVal,
	IN RX_CHAIN_IDX rx_ch_idx)
{
	UCHAR idx, val;

	if (((pAd->MACVersion & 0xffff0000) < 0x28830000) || 
		(pAd->Antenna.field.RxPath == 1))
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, pBbpVal);
		return NDIS_STATUS_SUCCESS;
	}

	idx = 0;
	while(rx_ch_idx != 0)
	{
		if (idx >= pAd->Antenna.field.RxPath)
			break;

		if (rx_ch_idx & 0x01)
		{
			val = 0;
			RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R27, &val);
			val = (val & (~0x60)) | (idx << 5);

#ifdef RTMP_MAC_PCI
			if (IS_PCI_INF(pAd) || IS_RBUS_INF(pAd))
			{
				RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R27, val);
				RTMP_BBP_IO_READ8_BY_REG_ID(pAd, bbpId, pBbpVal);
			}
#endif /* RTMP_MAC_PCI */
			break;
		}
		rx_ch_idx >>= 1;
		idx++;
	}

	return NDIS_STATUS_SUCCESS;
}


INT rtmp_bbp_get_agc(struct _RTMP_ADAPTER *pAd, CHAR *agc, RX_CHAIN_IDX idx)
{
	return AsicBBPReadWithRxChain(pAd, agc, idx);
}


INT rtmp_bbp_set_agc(struct _RTMP_ADAPTER *pAd, UCHAR agc, RX_CHAIN_IDX idx)
{
	return AsicBBPWriteWithRxChain(pAd, BBP_R66, agc, idx);
}


INT rtmp_bbp_set_filter_coefficient_ctrl(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	UCHAR bbp_val = 0, org_val = 0;

	if (Channel == 14)
	{
		/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &org_val);
		bbp_val = org_val;
		if (WMODE_EQUAL(pAd->CommonCfg.PhyMode, WMODE_B))
			bbp_val |= 0x20;
		else
			bbp_val &= (~0x20);

		if (bbp_val != org_val)
			RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, bbp_val);
	}

	return TRUE;
}


UCHAR rtmp_bbp_get_random_seed(RTMP_ADAPTER *pAd)
{
	UCHAR value1, value2, value3, value4, value5;

	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R50, &value1);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R51, &value2);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R52, &value3);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R53, &value4);
	RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R54, &value5);

	return (value1^value2^value3^value4^value5);
}


INT rtmp_bbp_is_ready(struct _RTMP_ADAPTER *pAd)
{
	INT idx = 0;
	UCHAR val;
	
	do 
	{
		RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R0, &val);
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return FALSE;
		DBGPRINT(RT_DEBUG_TRACE, ("BBP version = %x\n", val));
	} while ((++idx < 20) && ((val == 0xff) || (val == 0x00)));

	return (((val == 0xff) || (val == 0x00)) ? FALSE : TRUE);
}


#ifdef PRE_ANT_SWITCH

#endif /* PRE_ANT_SWITCH */


#ifdef CFO_TRACK
#ifdef CONFIG_AP_SUPPORT
INT rtmp_cfo_track(RTMP_ADAPTER *pAd, MAC_TABLE_ENTRY *pEntry, INT lastClient)
{
	/* CFO Tracking */
	if (IS_RT3883(pAd))
	{
		if (pAd->MacTab.Size !=1 || pAd->CommonCfg.CFOTrack==0)
		{
			/* Set to default */
			RT3883_AsicSetFreqOffset(pAd, pAd->RfFreqOffset);
		}
		else if ((lastClient < MAX_LEN_OF_MAC_TABLE) && (lastClient >=1) && 
			pAd->CommonCfg.CFOTrack < 8 && 
			pEntry->freqOffsetValid)
		{
			/* Track CFO */
			SHORT foValue, offset = pEntry->freqOffset;
			UCHAR RFValue;

			RT30xxReadRFRegister(pAd, RF_R17, (PUCHAR)&RFValue);
			RFValue &= 0x7F;

			if (offset > 32)
				offset = 32;
			else if (offset < -32)
				offset = -32;

			foValue = RFValue - (offset/16);
			if (foValue < 0)
				foValue = 0;
			else if (foValue > 0x5F)
				foValue = 0x5F;

			if (foValue != RFValue)
				RT3883_AsicSetFreqOffset(pAd, foValue);

			/* If CFOTrack!=1 then keep updating until CFOTrack==8 */
			if (pAd->CommonCfg.CFOTrack != 1)
				pAd->CommonCfg.CFOTrack++;

			pEntry->freqOffsetValid = FALSE;
		}
	}

	return TRUE;
}
#endif /* CONFIG_AP_SUPPORT */
#endif /* CFO_TRACK */


#endif /* RTMP_MAC */

