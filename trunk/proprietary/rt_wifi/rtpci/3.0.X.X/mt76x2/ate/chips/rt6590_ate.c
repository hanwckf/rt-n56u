/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2011, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt65xx_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT6590

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT65xx

#include "rt_config.h"

#ifndef RLT_RF
#error "You Should Enable compile flag RLT_RF for this chip"
#endif /* RLT_RF */

extern MT76x0_BBP_Table MT76x0_BPP_SWITCH_Tab[];
extern UCHAR MT76x0_BPP_SWITCH_Tab_Size;

extern VOID SelectBandMT76x0(PRTMP_ADAPTER pAd, UCHAR Channel);
extern VOID SetRfChFreqParametersMT76x0(PRTMP_ADAPTER pAd, UCHAR Channel);

static INT ate_bbp_set_ctrlch(struct _RTMP_ADAPTER *pAd, INT ext_ch)
{
/*	PATE_INFO pATEInfo = &(pAd->ate); */
	UINT32 agc, agc_r0 = 0;
	UINT32 be, be_r0 = 0;


	RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc_r0);
	agc = agc_r0 & (~0x300);
	RTMP_BBP_IO_READ32(pAd, TXBE_R0, &be_r0);
	be = (be_r0 & (~0x03));

#ifdef DOT11_VHT_AC
#endif /* DOT11_VHT_AC */
	{
		switch (ext_ch)
		{
			case EXTCHA_BELOW:
				agc |= 0x100;
				be |= 0x01;
				break;
			case EXTCHA_ABOVE:
				agc &= (~0x300);
				be &= (~0x03);
				break;
			case EXTCHA_NONE:
			default:
				agc &= (~0x300);
				be &= (~0x03);
				break;
		}
	}
	if (agc != agc_r0)
		RTMP_BBP_IO_WRITE32(pAd, AGC1_R0, agc);

	if (be != be_r0)
		RTMP_BBP_IO_WRITE32(pAd, TXBE_R0, be);

	return TRUE;
}


static VOID MT76x0ATE_ChipBBPAdjust(RTMP_ADAPTER *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR rf_bw, ext_ch;

#ifdef DOT11_N_SUPPORT
/*	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE) *//* ralink debug */
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = pATEInfo->TxWI.TXWI_N.BW;
		ext_ch = EXTCHA_NONE;
	}

/*	rtmp_bbp_set_bw(pAd, rf_bw); */
	bbp_set_bw(pAd, rf_bw);

	/* Tx/Rx : control channel setting */
	rtmp_mac_set_ctrlch(pAd, ext_ch);
	ate_bbp_set_ctrlch(pAd, ext_ch);
}


/*
	==========================================================================
    Description:

	AsicSwitchChannel() dedicated for MT76x0 ATE.
    
	==========================================================================
*/
static VOID MT76x0ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Index;
	UINT32 rf_phy_mode, rf_bw = RF_BW_20;
	UCHAR Channel = 0;
	UCHAR RFValue = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* set bandwidth */
	MT76x0ATE_ChipBBPAdjust(pAd);

	DBGPRINT(RT_DEBUG_TRACE,
		(" [%s] Channel = %d, pATEInfo->TxWI.TXWI_N.BW = %d , pATEInfo->RFFreqOffset = %d, "
		"pATEInfo->TxPower0 = %d, pATEInfo->TxPower1 = %d\n", 
		__FUNCTION__, Channel, pATEInfo->TxWI.TXWI_N.BW, pATEInfo->RFFreqOffset,
		pATEInfo->TxPower0, pATEInfo->TxPower1));

	if (Channel > 14)
		rf_phy_mode = RF_A_BAND;
	else
		rf_phy_mode = RF_G_BAND;

	if (pATEInfo->TxWI.TXWI_N.BW == BW_80)
		rf_bw = RF_BW_80;
	else if (pATEInfo->TxWI.TXWI_N.BW == BW_40)
		rf_bw = RF_BW_40;
	else
		rf_bw = RF_BW_20;

	/*
		Configure 2.4/5GHz before accessing other MAC/BB/RF registers
	*/
	SelectBandMT76x0(pAd, Channel);

	/*
		Set RF channel frequency parameters (Rdiv, N, K, D and Ksd)
	*/
	SetRfChFreqParametersMT76x0(pAd, Channel);

	/* 
		vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. 
	*/
	rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
	RFValue = ((RFValue & ~0x80) | 0x80); 
	rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);

	for (Index = 0; Index < MT76x0_BPP_SWITCH_Tab_Size; Index++)
	{
		if (((rf_phy_mode | rf_bw) & MT76x0_BPP_SWITCH_Tab[Index].BwBand) == (rf_phy_mode | rf_bw))
		{
			if ((MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register == AGC1_R8))
			{
				UINT32 eLNAgain = (MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value & 0x0000FF00) >> 8;

				if (Channel > 14)
				{
					if (Channel < 100)
						eLNAgain -= (pAd->ALNAGain0*2);
					else if (Channel < 137)
						eLNAgain -= (pAd->ALNAGain1*2);
					else
						eLNAgain -= (pAd->ALNAGain2*2);
				}
				else
				{
					eLNAgain -= (pAd->BLNAGain*2);
				}
				
				RTMP_BBP_IO_WRITE32(pAd, MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register,
						(MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value&(~0x0000FF00))|(eLNAgain << 8));
			}
			else
			{
				RTMP_BBP_IO_WRITE32(pAd, MT76x0_BPP_SWITCH_Tab[Index].RegDate.Register,
					MT76x0_BPP_SWITCH_Tab[Index].RegDate.Value);
			}
		}
	}


	if (Channel != pAd->LatchRfRegs.Channel)
	{
		MT76x0ATE_Calibration(pAd, Channel, FALSE, TRUE);
	}

	/* must not remove it */
	pAd->LatchRfRegs.Channel = Channel; /* Channel latch */

	RtmpusecDelay(100000);

	return;
}


static INT MT76x0ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 MacValue = 0;
	USHORT value = 0;
	CHAR TxPower = 0;
	CHAR bw_power_delta = 0;
/*	UCHAR Channel = pATEInfo->Channel; */

#ifdef RALINK_QA
	if (pATEInfo->bQAEnabled == TRUE)
	{
		return 0;
	}
#endif /* RALINK_QA */

	if (pATEInfo->TxWI.TXWI_N.BW == BW_80)
	{
		/* get 80 MHz power delta from e2p D3h */
		RT28xx_EEPROM_READ16(pAd, 0xD3, value);
		/* sanity check */
		if ((value & 0xFF) != 0xFF)
		{
			if ((value & 0x80)) /* bit7: Enable/Disable */
				bw_power_delta = (value & 0x3F);

			if ((value & 0x40) == 0) /* bit6: sign bit */
				bw_power_delta = (-1) * bw_power_delta;
		}
	}
	else if (pATEInfo->TxWI.TXWI_N.BW == BW_40)
	{
		/* get 40 MHz power delta from e2p 51h */
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA+1, value);
		/* sanity check */
		if ((value & 0xFF) != 0xFF)
		{
			if ((value & 0x80)) /* bit7: Enable/Disable */
				bw_power_delta = (value & 0x3F);

			if ((value & 0x40) == 0) /* bit6: sign bit */
				bw_power_delta = (-1) * bw_power_delta;
		}
	}

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &MacValue);

	if (index == 0)
	{
		TxPower = pATEInfo->TxPower0;
#ifdef MT76x0_TSSI_CAL_COMPENSATION
		/* when TSSI is enabled, 20MHz/40MHz power delta is performed on MAC 0x13B0 */
		if (((pATEInfo->TxWI.TXWI_N.BW == BW_80) || (pATEInfo->TxWI.TXWI_N.BW == BW_40)) && (pATEInfo->bAutoTxAlc == TRUE))
		{
			TxPower += (bw_power_delta);
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
		MacValue &= (~0x0000003F);
		MacValue |= TxPower;
	}
	else if (index == 1)
	{
		TxPower = pATEInfo->TxPower1;
#ifdef MT76x0_TSSI_CAL_COMPENSATION
		if (((pATEInfo->TxWI.TXWI_N.BW == BW_80) || (pATEInfo->TxWI.TXWI_N.BW == BW_40)) && (pATEInfo->bAutoTxAlc == TRUE))
		{
			TxPower += (bw_power_delta);
		}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
		MacValue &= (~0x00003F00);
		MacValue |= (TxPower << 8);
	}
	else
	{
		DBGPRINT_ERR(("%s : Only TxPower0 and TxPower1 are adjustable !\n", __FUNCTION__));
		DBGPRINT_ERR(("%s : TxPower%d is out of range !\n", __FUNCTION__, index));
		return -1;
	}

	MacValue |= (0x2F << 16);
	MacValue |= (0x2F << 24);
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, MacValue);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d)\n", __FUNCTION__, index, TxPower));
	
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	/* when TSSI is disabled, 20MHz/80MHz power delta is performed on MAC 0x13B4 */
	if (pATEInfo->bAutoTxAlc == FALSE)
	{
		if ((pATEInfo->TxWI.TXWI_N.BW == BW_80) || (pATEInfo->TxWI.TXWI_N.BW == BW_40))
		{
			CHAR diff;

			/* clean up MAC 0x13B4 bit[5:0] */
			RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MacValue);
			MacValue &= 0xFFFFFFC0;
			diff = bw_power_delta;

			/* MAC 0x13B4 is 6-bit signed value */
			if (bw_power_delta < 0)
			{				
				MacValue |= 0x20;
			}

			if (diff > 20)
				diff = 20;

			MacValue |= (diff & 0x1F);
			RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MacValue); 
		}

		DBGPRINT(RT_DEBUG_TRACE, ("bw_power_delta=%d, MAC 0x13B4 is 0x%08x\n", bw_power_delta, MacValue));
	}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */

	return 0;
}


/* 
	==========================================================================
    Description:
        Set RT6590 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
static INT	MT76x0_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pATEInfo->TxWI.TXWI_N.BW = BW_20;
	}
	else if (BBPCurrentBW == 1)
	{
		pATEInfo->TxWI.TXWI_N.BW = BW_40;
 	}
	else if (BBPCurrentBW == 2)
	{
		pATEInfo->TxWI.TXWI_N.BW = BW_80;
	}
	else
	{	
		return FALSE;
	}
	
	return TRUE;
}


/* 
	==========================================================================
    Description:
        Set MT76x0 ATE RF central frequency offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
	==========================================================================
*/
static INT	MT76x0_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset > 191 /* 0xBF */)
	{
		DBGPRINT_ERR(("%s::Out of range! (Value=%d)\n", __FUNCTION__, RFFreqOffset));
		DBGPRINT_ERR(("freq. offset range is 0~191.\n"));
		return FALSE;
	}
	
	pATEInfo->RFFreqOffset = RFFreqOffset;
	RFValue = (UCHAR)(pATEInfo->RFFreqOffset & 0x000000FF); 
	RFValue = min((INT)RFValue, 0xBF);
	rlt_rf_write(pAd, RF_BANK0, RF_R22, RFValue);

	return TRUE;
}


#ifdef MT76x0_TSSI_CAL_COMPENSATION
/* TSSI-related */
typedef struct _MCSPOWER_PAMODE_TABLE
{
	CHAR MCS_Power:6;
	CHAR Reserved1:2;

	UCHAR RF_PA_Mode;
}   MCSPOWER_PAMODE_TABLE, *PMCSPOWER_PAMODE_TABLE;

typedef struct  _TSSI_COMPENSATION_TABLE
{
	MCSPOWER_PAMODE_TABLE MCS32;
	MCSPOWER_PAMODE_TABLE CCK[4];
	MCSPOWER_PAMODE_TABLE OFDM[8];
	MCSPOWER_PAMODE_TABLE STBC[8];
	MCSPOWER_PAMODE_TABLE HT[16];
	MCSPOWER_PAMODE_TABLE VHT[2][10];
}   TSSI_COMPENSATION_TABLE, *PTSSI_COMPENSATION_TABLE;

TSSI_COMPENSATION_TABLE AteTssiTable;

static INT32 TSSI_DELTA_PRE;
static SHORT TSSI_Linear0;
static SHORT TSSI_Linear1;
static CHAR Current_TSSI_DC;
static CHAR TargetPower0;
static CHAR TargetPower1;
static CHAR TargetPA_mode0;
static CHAR TargetPA_mode1;
static UCHAR INFO_1;
static UCHAR INFO_2;
static UCHAR INFO_3;
static UCHAR TSSI_Tx_Mode;
static UCHAR CurrentPower0;
static UCHAR CurrentPower1;

#define DEFAULT_BO              4
#define LIN2DB_ERROR_CODE       (-10000)
static INT16 lin2dBd(
	IN	unsigned short linearValue)
{
    short exp;
    unsigned int mantisa;
    int app,dBd;

	/* Default backoff ; to enhance leading bit searching time */
	mantisa = linearValue << DEFAULT_BO;
	exp = -(DEFAULT_BO);

	/* Leading bit searching */
	if (mantisa < (0x8000))
	{
		while (mantisa < (0x8000))
		{
			mantisa = mantisa << 1; /* no need saturation */
			exp--;
			if (exp < -20)
			{
				DBGPRINT_ERR(("input too small\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
	else 
	{
		while (mantisa > (0xFFFF))
		{
			mantisa = mantisa >> 1; /* no need saturation */
			exp ++;
			if (exp > 20)
			{
				DBGPRINT_ERR(("input too large\n"));
				DBGPRINT_ERR(("exponent = %d\n",exp));

				return LIN2DB_ERROR_CODE;
			}
		}
	}
/*	printk("exp=0d%d,mantisa=0x%x\n",exp,mantisa); */

	if (mantisa <= 47104)
	{
		app=(mantisa+(mantisa>>3)+(mantisa>>4)-38400); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}
	else
	{
		app=(mantisa-(mantisa>>3)-(mantisa>>6)-23040); /* S(15,0) */
		if (app<0)
		{
			app=0;
		}
	}

	dBd=((15+exp)<<15)+app; /*since 2^15=1 here */
/*	printk("dBd1=%d\n",dBd); */
	dBd=(dBd<<2)+(dBd<<1)+(dBd>>6)+(dBd>>7);
	dBd=(dBd>>10); /* S10.5 */
/*	printk("app=%d,dBd=%d,dBdF=%f\n",app,dBd,(double)dBd/32); */

	return(dBd);
}


VOID MT76x0ATE_MakeUpTssiTable(
	IN RTMP_ADAPTER *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32   MAC_Value;

	/* MCS POWER */
	RTMP_IO_READ32(pAd, TX_PWR_CFG_0, &MAC_Value);
	AteTssiTable.CCK[0].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.CCK[0].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.CCK[0].MCS_Power -= 64;
	AteTssiTable.CCK[1].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.CCK[1].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.CCK[1].MCS_Power -= 64;
	AteTssiTable.CCK[2].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.CCK[2].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.CCK[2].MCS_Power -= 64;
	AteTssiTable.CCK[3].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.CCK[3].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.CCK[3].MCS_Power -= 64;
	AteTssiTable.OFDM[0].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.OFDM[0].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[0].MCS_Power -= 64;
	AteTssiTable.OFDM[1].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.OFDM[1].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[1].MCS_Power -= 64;
	AteTssiTable.OFDM[2].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.OFDM[2].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[2].MCS_Power -= 64;
	AteTssiTable.OFDM[3].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.OFDM[3].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[3].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &MAC_Value);
	AteTssiTable.OFDM[4].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.OFDM[4].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[4].MCS_Power -= 64;
	AteTssiTable.OFDM[5].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.OFDM[5].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[5].MCS_Power -= 64;
	AteTssiTable.OFDM[6].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.OFDM[6].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[6].MCS_Power -= 64;
	AteTssiTable.HT[0].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.HT[0].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[0].MCS_Power -= 64;
	AteTssiTable.VHT[0][0].MCS_Power = AteTssiTable.HT[0].MCS_Power;
	AteTssiTable.MCS32.MCS_Power = AteTssiTable.HT[0].MCS_Power;
	if (AteTssiTable.MCS32.MCS_Power & 0x20) /* > 32 */
		AteTssiTable.MCS32.MCS_Power -= 64;
	AteTssiTable.HT[1].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.HT[1].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[1].MCS_Power -= 64;
	AteTssiTable.VHT[0][1].MCS_Power = AteTssiTable.HT[1].MCS_Power;
	AteTssiTable.HT[2].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.HT[2].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[2].MCS_Power -= 64;
	AteTssiTable.VHT[0][2].MCS_Power = AteTssiTable.HT[2].MCS_Power;
	AteTssiTable.HT[3].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.HT[3].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[3].MCS_Power -= 64;
	AteTssiTable.VHT[0][3].MCS_Power = AteTssiTable.HT[3].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &MAC_Value);
	AteTssiTable.HT[4].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.HT[4].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[4].MCS_Power -= 64;
	AteTssiTable.VHT[0][4].MCS_Power = AteTssiTable.HT[4].MCS_Power;
	AteTssiTable.HT[5].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.HT[5].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[5].MCS_Power -= 64;
	AteTssiTable.VHT[0][5].MCS_Power = AteTssiTable.HT[5].MCS_Power;
	AteTssiTable.HT[6].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.HT[6].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[6].MCS_Power -= 64;
	AteTssiTable.VHT[0][6].MCS_Power = AteTssiTable.HT[6].MCS_Power;
	AteTssiTable.HT[8].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.HT[8].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[8].MCS_Power -= 64;
	AteTssiTable.VHT[1][0].MCS_Power = AteTssiTable.HT[8].MCS_Power;
	AteTssiTable.HT[9].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.HT[9].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[9].MCS_Power -= 64;
	AteTssiTable.VHT[1][1].MCS_Power = AteTssiTable.HT[9].MCS_Power;
	AteTssiTable.HT[10].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.HT[10].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[10].MCS_Power -= 64;
	AteTssiTable.VHT[1][2].MCS_Power = AteTssiTable.HT[10].MCS_Power;
	AteTssiTable.HT[11].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.HT[11].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[11].MCS_Power -= 64;
	AteTssiTable.VHT[1][3].MCS_Power = AteTssiTable.HT[11].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_3, &MAC_Value);
	AteTssiTable.HT[12].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.HT[12].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[12].MCS_Power -= 64;
	AteTssiTable.VHT[1][4].MCS_Power = AteTssiTable.HT[12].MCS_Power;
	AteTssiTable.HT[13].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.HT[13].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[13].MCS_Power -= 64;
	AteTssiTable.VHT[1][5].MCS_Power = AteTssiTable.HT[13].MCS_Power;
	AteTssiTable.HT[14].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.HT[14].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[14].MCS_Power -= 64;
	AteTssiTable.VHT[1][6].MCS_Power = AteTssiTable.HT[14].MCS_Power;
	AteTssiTable.STBC[0].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.STBC[0].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[0].MCS_Power -= 64;
	AteTssiTable.STBC[1].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.STBC[1].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[1].MCS_Power -= 64;
	AteTssiTable.STBC[2].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.STBC[2].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[2].MCS_Power -= 64;
	AteTssiTable.STBC[3].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.STBC[3].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[3].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &MAC_Value);
	AteTssiTable.STBC[4].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.STBC[4].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[4].MCS_Power -= 64;
	AteTssiTable.STBC[5].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.STBC[5].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[5].MCS_Power -= 64;
	AteTssiTable.STBC[6].MCS_Power = (CHAR)((MAC_Value&0x3F00)>>8);
	if (AteTssiTable.STBC[6].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[6].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_7, &MAC_Value);
	AteTssiTable.OFDM[7].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.OFDM[7].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.OFDM[7].MCS_Power -= 64;
	AteTssiTable.HT[7].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.HT[7].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[7].MCS_Power -= 64;
	AteTssiTable.VHT[0][7].MCS_Power = AteTssiTable.HT[7].MCS_Power;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_8, &MAC_Value);
	AteTssiTable.HT[15].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.HT[15].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.HT[15].MCS_Power -= 64;
	AteTssiTable.VHT[1][7].MCS_Power = AteTssiTable.HT[15].MCS_Power;
	AteTssiTable.VHT[0][8].MCS_Power = (CHAR)((MAC_Value&0x3F0000)>>16);
	if (AteTssiTable.VHT[0][8].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.VHT[0][8].MCS_Power -= 64;
	AteTssiTable.VHT[0][9].MCS_Power = (CHAR)((MAC_Value&0x3F000000)>>24);
	if (AteTssiTable.VHT[0][9].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.VHT[0][9].MCS_Power -= 64;

	RTMP_IO_READ32(pAd, TX_PWR_CFG_9, &MAC_Value);
	AteTssiTable.STBC[7].MCS_Power = (CHAR)(MAC_Value&0x3F);
	if (AteTssiTable.STBC[7].MCS_Power & 0x20) /* > 32 */
		AteTssiTable.STBC[7].MCS_Power -= 64;

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[0].MCS_Power = %d\n", AteTssiTable.CCK[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[1].MCS_Power = %d\n", AteTssiTable.CCK[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[2].MCS_Power = %d\n", AteTssiTable.CCK[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[3].MCS_Power = %d\n", AteTssiTable.CCK[3].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[0].MCS_Power = %d\n", AteTssiTable.OFDM[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[1].MCS_Power = %d\n", AteTssiTable.OFDM[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[2].MCS_Power = %d\n", AteTssiTable.OFDM[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[3].MCS_Power = %d\n", AteTssiTable.OFDM[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[4].MCS_Power = %d\n", AteTssiTable.OFDM[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[5].MCS_Power = %d\n", AteTssiTable.OFDM[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[6].MCS_Power = %d\n", AteTssiTable.OFDM[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[7].MCS_Power = %d\n", AteTssiTable.OFDM[7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[0].MCS_Power = %d\n", AteTssiTable.STBC[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[1].MCS_Power = %d\n", AteTssiTable.STBC[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[2].MCS_Power = %d\n", AteTssiTable.STBC[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[3].MCS_Power = %d\n", AteTssiTable.STBC[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[4].MCS_Power = %d\n", AteTssiTable.STBC[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[5].MCS_Power = %d\n", AteTssiTable.STBC[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[6].MCS_Power = %d\n", AteTssiTable.STBC[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[7].MCS_Power = %d\n", AteTssiTable.STBC[7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[0].MCS_Power = %d\n", AteTssiTable.HT[0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[1].MCS_Power = %d\n", AteTssiTable.HT[1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[2].MCS_Power = %d\n", AteTssiTable.HT[2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[3].MCS_Power = %d\n", AteTssiTable.HT[3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[4].MCS_Power = %d\n", AteTssiTable.HT[4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[5].MCS_Power = %d\n", AteTssiTable.HT[5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[6].MCS_Power = %d\n", AteTssiTable.HT[6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[7].MCS_Power = %d\n", AteTssiTable.HT[7].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[8].MCS_Power = %d\n", AteTssiTable.HT[8].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[9].MCS_Power = %d\n", AteTssiTable.HT[9].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[10].MCS_Power = %d\n", AteTssiTable.HT[10].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[11].MCS_Power = %d\n", AteTssiTable.HT[11].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[12].MCS_Power = %d\n", AteTssiTable.HT[12].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[13].MCS_Power = %d\n", AteTssiTable.HT[13].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[14].MCS_Power = %d\n", AteTssiTable.HT[14].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[15].MCS_Power = %d\n", AteTssiTable.HT[15].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][0].MCS_Power = %d\n", AteTssiTable.VHT[0][0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][1].MCS_Power = %d\n", AteTssiTable.VHT[0][1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][2].MCS_Power = %d\n", AteTssiTable.VHT[0][2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][3].MCS_Power = %d\n", AteTssiTable.VHT[0][3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][4].MCS_Power = %d\n", AteTssiTable.VHT[0][4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][5].MCS_Power = %d\n", AteTssiTable.VHT[0][5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][6].MCS_Power = %d\n", AteTssiTable.VHT[0][6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][7].MCS_Power = %d\n", AteTssiTable.VHT[0][7].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][8].MCS_Power = %d\n", AteTssiTable.VHT[0][8].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][9].MCS_Power = %d\n", AteTssiTable.VHT[0][9].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][0].MCS_Power = %d\n", AteTssiTable.VHT[1][0].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][1].MCS_Power = %d\n", AteTssiTable.VHT[1][1].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][2].MCS_Power = %d\n", AteTssiTable.VHT[1][2].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][3].MCS_Power = %d\n", AteTssiTable.VHT[1][3].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][4].MCS_Power = %d\n", AteTssiTable.VHT[1][4].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][5].MCS_Power = %d\n", AteTssiTable.VHT[1][5].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][6].MCS_Power = %d\n", AteTssiTable.VHT[1][6].MCS_Power));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][7].MCS_Power = %d\n", AteTssiTable.VHT[1][7].MCS_Power));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.MCS32.MCS_Power = %d\n", AteTssiTable.MCS32.MCS_Power));

	/* PA MODE */
	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG0, &MAC_Value);
	AteTssiTable.CCK[0].RF_PA_Mode = (UCHAR)(MAC_Value&0x3);
	AteTssiTable.CCK[1].RF_PA_Mode = (UCHAR)((MAC_Value&0xC)>>2);
	AteTssiTable.CCK[2].RF_PA_Mode = (UCHAR)((MAC_Value&0x30)>>4);
	AteTssiTable.CCK[3].RF_PA_Mode = (UCHAR)((MAC_Value&0xC0)>>6);
	AteTssiTable.OFDM[0].RF_PA_Mode = (UCHAR)((MAC_Value&0x300)>>8);
	AteTssiTable.OFDM[1].RF_PA_Mode = (UCHAR)((MAC_Value&0xC00)>>10);
	AteTssiTable.OFDM[2].RF_PA_Mode = (UCHAR)((MAC_Value&0x3000)>>12);
	AteTssiTable.OFDM[3].RF_PA_Mode = (UCHAR)((MAC_Value&0xC000)>>14);
	AteTssiTable.OFDM[4].RF_PA_Mode = (UCHAR)((MAC_Value&0x30000)>>16);
	AteTssiTable.OFDM[5].RF_PA_Mode = (UCHAR)((MAC_Value&0xC0000)>>18);
	AteTssiTable.OFDM[6].RF_PA_Mode = (UCHAR)((MAC_Value&0x300000)>>20);
	AteTssiTable.OFDM[7].RF_PA_Mode = (UCHAR)((MAC_Value&0xC00000)>>22);
	AteTssiTable.MCS32.RF_PA_Mode = (UCHAR)((MAC_Value&0x3000000)>>24);

	RTMP_IO_READ32(pAd, RF_PA_MODE_CFG1, &MAC_Value);
	AteTssiTable.HT[0].RF_PA_Mode = (UCHAR)(MAC_Value&0x3);
	AteTssiTable.VHT[0][0].RF_PA_Mode = AteTssiTable.HT[0].RF_PA_Mode;
	AteTssiTable.VHT[1][0].RF_PA_Mode = AteTssiTable.HT[0].RF_PA_Mode;
	AteTssiTable.HT[1].RF_PA_Mode = (UCHAR)((MAC_Value&0xC)>>2);
	AteTssiTable.VHT[0][1].RF_PA_Mode = AteTssiTable.HT[1].RF_PA_Mode;
	AteTssiTable.VHT[1][1].RF_PA_Mode = AteTssiTable.HT[1].RF_PA_Mode;
	AteTssiTable.HT[2].RF_PA_Mode = (UCHAR)((MAC_Value&0x30)>>4);
	AteTssiTable.VHT[0][2].RF_PA_Mode = AteTssiTable.HT[2].RF_PA_Mode;
	AteTssiTable.VHT[1][2].RF_PA_Mode = AteTssiTable.HT[2].RF_PA_Mode;
	AteTssiTable.HT[3].RF_PA_Mode = (UCHAR)((MAC_Value&0xC0)>>6);
	AteTssiTable.VHT[0][3].RF_PA_Mode = AteTssiTable.HT[3].RF_PA_Mode;
	AteTssiTable.VHT[1][3].RF_PA_Mode = AteTssiTable.HT[3].RF_PA_Mode;
	AteTssiTable.HT[4].RF_PA_Mode = (UCHAR)((MAC_Value&0x300)>>8);
	AteTssiTable.VHT[0][4].RF_PA_Mode = AteTssiTable.HT[4].RF_PA_Mode;
	AteTssiTable.VHT[1][4].RF_PA_Mode = AteTssiTable.HT[4].RF_PA_Mode;
	AteTssiTable.HT[5].RF_PA_Mode = (UCHAR)((MAC_Value&0xC00)>>10);
	AteTssiTable.VHT[0][5].RF_PA_Mode = AteTssiTable.HT[5].RF_PA_Mode;
	AteTssiTable.VHT[1][5].RF_PA_Mode = AteTssiTable.HT[5].RF_PA_Mode;
	AteTssiTable.HT[6].RF_PA_Mode = (UCHAR)((MAC_Value&0x3000)>>12);
	AteTssiTable.VHT[0][6].RF_PA_Mode = AteTssiTable.HT[6].RF_PA_Mode;
	AteTssiTable.VHT[1][6].RF_PA_Mode = AteTssiTable.HT[6].RF_PA_Mode;
	AteTssiTable.HT[7].RF_PA_Mode = (UCHAR)((MAC_Value&0xC000)>>14);
	AteTssiTable.VHT[0][7].RF_PA_Mode = AteTssiTable.HT[7].RF_PA_Mode;
	AteTssiTable.VHT[1][7].RF_PA_Mode = AteTssiTable.HT[7].RF_PA_Mode;
	AteTssiTable.HT[8].RF_PA_Mode = (UCHAR)((MAC_Value&0x30000)>>16);
	AteTssiTable.VHT[0][8].RF_PA_Mode = AteTssiTable.HT[8].RF_PA_Mode;
	AteTssiTable.VHT[1][8].RF_PA_Mode = AteTssiTable.HT[8].RF_PA_Mode;
	AteTssiTable.HT[9].RF_PA_Mode = (UCHAR)((MAC_Value&0xC0000)>>18);
	AteTssiTable.VHT[0][9].RF_PA_Mode = AteTssiTable.HT[9].RF_PA_Mode;
	AteTssiTable.VHT[1][9].RF_PA_Mode = AteTssiTable.HT[9].RF_PA_Mode;
	AteTssiTable.HT[10].RF_PA_Mode = (UCHAR)((MAC_Value&0x300000)>>20);
	AteTssiTable.HT[11].RF_PA_Mode = (UCHAR)((MAC_Value&0xC00000)>>22);
	AteTssiTable.HT[12].RF_PA_Mode = (UCHAR)((MAC_Value&0x3000000)>>24);
	AteTssiTable.HT[13].RF_PA_Mode = (UCHAR)((MAC_Value&0xC000000)>>26);
	AteTssiTable.HT[14].RF_PA_Mode = (UCHAR)((MAC_Value&0x30000000)>>28);
	AteTssiTable.HT[15].RF_PA_Mode = (UCHAR)((MAC_Value&0xC0000000)>>30);

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[0].RF_PA_Mode = %d\n", AteTssiTable.CCK[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[1].RF_PA_Mode = %d\n", AteTssiTable.CCK[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[2].RF_PA_Mode = %d\n", AteTssiTable.CCK[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.CCK[3].RF_PA_Mode = %d\n", AteTssiTable.CCK[3].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[0].RF_PA_Mode = %d\n", AteTssiTable.OFDM[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[1].RF_PA_Mode = %d\n", AteTssiTable.OFDM[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[2].RF_PA_Mode = %d\n", AteTssiTable.OFDM[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[3].RF_PA_Mode = %d\n", AteTssiTable.OFDM[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[4].RF_PA_Mode = %d\n", AteTssiTable.OFDM[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[5].RF_PA_Mode = %d\n", AteTssiTable.OFDM[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[6].RF_PA_Mode = %d\n", AteTssiTable.OFDM[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.OFDM[7].RF_PA_Mode = %d\n", AteTssiTable.OFDM[7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[0].RF_PA_Mode = %d\n", AteTssiTable.STBC[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[1].RF_PA_Mode = %d\n", AteTssiTable.STBC[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[2].RF_PA_Mode = %d\n", AteTssiTable.STBC[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[3].RF_PA_Mode = %d\n", AteTssiTable.STBC[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[4].RF_PA_Mode = %d\n", AteTssiTable.STBC[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[5].RF_PA_Mode = %d\n", AteTssiTable.STBC[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[6].RF_PA_Mode = %d\n", AteTssiTable.STBC[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.STBC[7].RF_PA_Mode = %d\n", AteTssiTable.STBC[7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[0].RF_PA_Mode = %d\n", AteTssiTable.HT[0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[1].RF_PA_Mode = %d\n", AteTssiTable.HT[1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[2].RF_PA_Mode = %d\n", AteTssiTable.HT[2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[3].RF_PA_Mode = %d\n", AteTssiTable.HT[3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[4].RF_PA_Mode = %d\n", AteTssiTable.HT[4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[5].RF_PA_Mode = %d\n", AteTssiTable.HT[5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[6].RF_PA_Mode = %d\n", AteTssiTable.HT[6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[7].RF_PA_Mode = %d\n", AteTssiTable.HT[7].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[8].RF_PA_Mode = %d\n", AteTssiTable.HT[8].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[9].RF_PA_Mode = %d\n", AteTssiTable.HT[9].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[10].RF_PA_Mode = %d\n", AteTssiTable.HT[10].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[11].RF_PA_Mode = %d\n", AteTssiTable.HT[11].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[12].RF_PA_Mode = %d\n", AteTssiTable.HT[12].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[13].RF_PA_Mode = %d\n", AteTssiTable.HT[13].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[14].RF_PA_Mode = %d\n", AteTssiTable.HT[14].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.HT[15].RF_PA_Mode = %d\n", AteTssiTable.HT[15].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][0].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][1].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][2].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][3].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][4].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][5].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][6].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[0][7].RF_PA_Mode = %d\n", AteTssiTable.VHT[0][7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][0].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][0].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][1].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][1].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][2].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][2].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][3].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][3].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][4].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][4].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][5].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][5].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][6].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][6].RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.VHT[1][7].RF_PA_Mode = %d\n", AteTssiTable.VHT[1][7].RF_PA_Mode));

	DBGPRINT(RT_DEBUG_TRACE, ("ATE TSSI: AteTssiTable.MCS32.RF_PA_Mode = %d\n", AteTssiTable.MCS32.RF_PA_Mode));
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));

	return;
}


static VOID MT76x0ATE_GetTargetPower(
	IN RTMP_ADAPTER *pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 MAC_Value;
	USHORT index;
	UCHAR Tx_Rate;
	CHAR Eas_power_adj = 0;

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &MAC_Value);
	CurrentPower0 = (UCHAR)(MAC_Value & 0x3F);
	CurrentPower1 = (UCHAR)((MAC_Value & 0x3F00) >> 8);

	TSSI_Tx_Mode = (INFO_1 & 0x7);
	Eas_power_adj = (INFO_3 & 0xF);

	if (TSSI_Tx_Mode == 0)
	{
		/* 0: 1 Mbps, 1: 2 Mbps, 2: 5.5 Mbps, 3: 11 Mbps */
		Tx_Rate = ((INFO_1 & 0x60) >> 5);

		TargetPower0 = (CHAR)(CurrentPower0 + AteTssiTable.CCK[Tx_Rate].MCS_Power);
		TargetPA_mode0 = (CHAR) AteTssiTable.CCK[Tx_Rate].RF_PA_Mode;

		if (pAd->Antenna.field.TxPath >= 2)
		{ 
			/* 2T */
			TargetPower1 = (CHAR)(CurrentPower1 + AteTssiTable.CCK[Tx_Rate].MCS_Power);
			TargetPA_mode1 = AteTssiTable.CCK[Tx_Rate].RF_PA_Mode;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("==> CCK Mode :: TargetPower0 = %d\n", TargetPower0));
	}
	else if (TSSI_Tx_Mode == 1)
	{
		Tx_Rate = ((INFO_1 & 0xF0) >> 4);
		if (Tx_Rate == 0xB)
			index = 0;
		else if (Tx_Rate == 0xF)
			index = 1;
		else if (Tx_Rate == 0xA)
			index = 2;
		else if (Tx_Rate == 0xE)
			index = 3;
		else if (Tx_Rate == 0x9)
			index = 4;
		else if (Tx_Rate == 0xD)
			index = 5;
		else if (Tx_Rate == 0x8)
			index = 6;
		else if (Tx_Rate == 0xC)
			index = 7;

		TargetPower0 = (CHAR)(CurrentPower0 + AteTssiTable.OFDM[index].MCS_Power);
		TargetPA_mode0 = AteTssiTable.OFDM[index].RF_PA_Mode;

		if (pAd->Antenna.field.TxPath >= 2)
		{ 
			/* 2T */
			TargetPower1 = (CHAR)(CurrentPower1 + AteTssiTable.OFDM[index].MCS_Power);
			TargetPA_mode1 = AteTssiTable.OFDM[index].RF_PA_Mode;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("==> OFDM Mode :: TargetPower0 = %d\n", TargetPower0));
	}
	else if (TSSI_Tx_Mode == 4)
	{
		Tx_Rate = (INFO_2 & 0x0F);

		TargetPower0 = (CHAR)(CurrentPower0 + AteTssiTable.VHT[0][Tx_Rate].MCS_Power);
		TargetPA_mode0 = (CHAR) AteTssiTable.VHT[0][Tx_Rate].RF_PA_Mode;
	}
	else
	{
		Tx_Rate = (INFO_2 & 0x7F);

		if (Tx_Rate == 32) /* MCS32 */
		{
			TargetPower0 = (CHAR)(CurrentPower0 + AteTssiTable.MCS32.MCS_Power);
			TargetPA_mode0 = AteTssiTable.MCS32.RF_PA_Mode;

			if (pAd->Antenna.field.TxPath >= 2)
			{ 
				/* 2T */
				TargetPower1 = (CHAR)(CurrentPower1 + AteTssiTable.MCS32.MCS_Power);
				TargetPA_mode1 = AteTssiTable.MCS32.RF_PA_Mode;
			}
		}
		else
		{
			TargetPower0 = (CHAR)(CurrentPower0 + AteTssiTable.HT[Tx_Rate].MCS_Power);
			TargetPA_mode0 = AteTssiTable.HT[Tx_Rate].RF_PA_Mode;

			if (pAd->Antenna.field.TxPath >= 2)
			{ 
				/* 2T */
				TargetPower1 = (CHAR)(CurrentPower1 + AteTssiTable.HT[Tx_Rate].MCS_Power);
				TargetPA_mode1 = AteTssiTable.HT[Tx_Rate].RF_PA_Mode;
			}
		}

		DBGPRINT(RT_DEBUG_TRACE, ("==> HT Mode :: TargetPower0 = %d\n", TargetPower0));
	}

	return;
}


static VOID MT76x0ATE_EstimateDeltaPower(
	IN PRTMP_ADAPTER pAd, 
	OUT INT32 *tssi_delta0)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32 tssi_slope0=0;
	INT32 tssi_offset0=0;
	INT32 tssi_target=0, tssi_delta_tmp;
	INT32 tssi_meas=0;
	INT32 tssi_dc;
	INT32 pkt_type_delta=0, bbp_6db_power=0;
	UINT32 BBP_Value;
	USHORT EE_Value;
	CHAR idx = 0;

	/* a. tssi_dc gotten from Power on calibration */
	/* b. Read Slope: u.2.6 */
	/* c. Read offset: s.3.4 */
	if (pATEInfo->Channel > 14)
	{
		for (idx = 0; idx < 7; idx++)
		{
			if ((pATEInfo->Channel <= pAd->chipCap.tssi_5G_channel_boundary[idx])
				|| (pAd->chipCap.tssi_5G_channel_boundary[idx] == 0))
			{
				tssi_slope0 = pAd->chipCap.tssi_slope_5G[idx];
				tssi_offset0 = pAd->chipCap.tssi_offset_5G[idx];
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_5G_channel_boundary[%d] = %d\n", idx, pAd->chipCap.tssi_5G_channel_boundary[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope_5G[%d] = %d\n", idx, pAd->chipCap.tssi_slope_5G[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset_5G[%d] = %d\n", idx, pAd->chipCap.tssi_offset_5G[idx]));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope = %d\n", tssi_slope0));
				DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset = %d\n", tssi_offset0));
				break;
			}
		}
		if (idx == 7)
		{
			tssi_slope0 = pAd->chipCap.tssi_slope_5G[idx];
			tssi_offset0 = pAd->chipCap.tssi_offset_5G[idx];
			DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_5G_channel_boundary[%d] = %d\n", idx, pAd->chipCap.tssi_5G_channel_boundary[idx]));
		}
	}
	else
	{
		tssi_slope0 = pAd->chipCap.tssi_slope_2G;
		tssi_offset0 = pAd->chipCap.tssi_offset_2G;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> 1) tssi_offset0 = %d (0x%x)\n", tssi_offset0, tssi_offset0));

	if (pATEInfo->Channel > 14)
	{
		if ((tssi_offset0 >= 0x40) && (tssi_offset0 <= 0x7F))
			tssi_offset0 = tssi_offset0 - 0x100;
		else
			tssi_offset0 = (tssi_offset0 & 0x80) ?  tssi_offset0 - 0x100 : tssi_offset0;
	}
	else
	{
		tssi_offset0 = (tssi_offset0 & 0x80) ?  tssi_offset0 - 0x100 : tssi_offset0;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> EstimateDeltaPower\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TargetPower0 = %d\n", TargetPower0));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear0 = %d\n", TSSI_Linear0));
	DBGPRINT(RT_DEBUG_TRACE, ("==> Current_TSSI_DC = %d\n", Current_TSSI_DC));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_slope0 = %d\n", tssi_slope0));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_offset0 = %d\n", tssi_offset0));

	/* d. */
	/* Cal delta0 */
	tssi_target = TargetPower0 << 12;
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = TargetPower0*4096) = %d\n", tssi_target));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TargetPA_mode0 = %d\n", TargetPA_mode0));

	switch (TargetPA_mode0)
	{
		case 0: 
			tssi_target = tssi_target;
			DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target) = %d\n", tssi_target));
			break;
		case 1: 
			if (pATEInfo->Channel > 14)
			{
				tssi_target = tssi_target + 0;
				DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + 0) = %d\n", tssi_target));
			}
			else
			{
				tssi_target = tssi_target + 29491; /* 3.6 * 8192 */
				DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + 29491) = %d\n", tssi_target));
			}
			break;
		default: 
			tssi_target = tssi_target +  4424; /* 0.54 * 8192 */
			DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target +  4424) = %d\n", tssi_target));
			break;
	}

	RTMP_BBP_IO_READ32(pAd, CORE_R1, &BBP_Value);

	switch (TSSI_Tx_Mode)
	{
		case 0:
			if (IS_MT7630(pAd))
			{ 
				/* MT7630E */
				if (BBP_Value&0x20)
					pkt_type_delta = 18841; /* 23 * 8192 */
				else
					pkt_type_delta = 12288; /* 5 * 8192 */
			}
			else
			{ 
				/* Others */
				if (BBP_Value&0x20)
					pkt_type_delta = 6554; /* 0.8 * 8192 */
				else
					pkt_type_delta = 0; /* 0 * 8192 */
				}
			break;
		default:
			pkt_type_delta = 0;
	}

	tssi_target = tssi_target + pkt_type_delta;
	RTMP_BBP_IO_READ32(pAd, TXBE_R4, &BBP_Value);
	DBGPRINT(RT_DEBUG_TRACE, ("==> TXBE_R4 = 0x%X\n", BBP_Value));

	switch ((BBP_Value & 0x3))
	{
		case 0: 
			bbp_6db_power = 0; 
			break;
		case 1: 
			bbp_6db_power = -49152; 
			break; /* -6 dB*8192 */
		case 2: 
			bbp_6db_power = -98304; 
			break; /* -12 dB*8192 */
		case 3: 
			bbp_6db_power = 49152; 
			break; /*6 dB*8192 */
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> bbp_6db_power = %d\n", bbp_6db_power));
	tssi_target = tssi_target + bbp_6db_power;
	DBGPRINT(RT_DEBUG_TRACE, ("==> (tssi_target = tssi_target + bbp_6db_power) = %d\n", tssi_target));

	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_target = %f\n", tssi_target / 8192));
	tssi_dc = Current_TSSI_DC;
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear0 = %d\n", TSSI_Linear0)); 
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_dc = %d\n", tssi_dc)); 
	tssi_meas = lin2dBd((TSSI_Linear0 - tssi_dc));
	DBGPRINT(RT_DEBUG_TRACE, ("==> Linear to dB = %d\n", tssi_meas)); 

	tssi_meas = tssi_meas *tssi_slope0;
	DBGPRINT(RT_DEBUG_TRACE, ("==> dB x slope = %d\n", tssi_meas));

	if (pATEInfo->Channel > 14)
		tssi_meas += ((tssi_offset0 - 50) << 10); /* 5G: offset s4.3 */
	else
		tssi_meas += (tssi_offset0 << 9); /* 2G: offset s3.4 */
	DBGPRINT(RT_DEBUG_TRACE, ("==> measure db = %d %f\n", tssi_meas, tssi_meas/8192));

	tssi_delta_tmp = tssi_target - tssi_meas;
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_Linear0 = %d\n", TSSI_Linear0));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta_tmp = %d\n", tssi_delta_tmp));

	if (pATEInfo->Channel > 14)
	{
		if ((TSSI_Linear0 > 254) && (tssi_delta_tmp > 0)) /* upper saturate */
			tssi_delta_tmp = 0;
	}
	else
	{
		if ((TSSI_Linear0 > 126) && (tssi_delta_tmp > 0)) /* upper saturate */
			tssi_delta_tmp = 0;
		if (((TSSI_Linear0 - tssi_dc) < 1) && (tssi_delta_tmp < 0)) /* lower saturate */
			tssi_delta_tmp = 0;
	}

	/* stablize the compensation value */
	/* if previous compensation result is better than current, skip the compensation */
	if (((TSSI_DELTA_PRE ^ tssi_delta_tmp) < 0) 
			&& ((tssi_delta_tmp < 4096) && (tssi_delta_tmp > -4096))
			&& ((TSSI_DELTA_PRE < 4096) && (TSSI_DELTA_PRE > -4096)))
	{
		if ((tssi_delta_tmp>0) && ((tssi_delta_tmp +TSSI_DELTA_PRE) <= 0))
			tssi_delta_tmp = 0;
		else if ((tssi_delta_tmp<0) && ((tssi_delta_tmp +TSSI_DELTA_PRE) > 0))
			tssi_delta_tmp = 0;
		else
			TSSI_DELTA_PRE = tssi_delta_tmp;
	}
	else
	{
		TSSI_DELTA_PRE = tssi_delta_tmp;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("==> TSSI_DELTA_PRE = %d\n", TSSI_DELTA_PRE));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta_tmp = %d\n", tssi_delta_tmp));

	/* make the compensate value to the nearest compensate code */
	tssi_delta_tmp = tssi_delta_tmp + ((tssi_delta_tmp > 0 ) ? 2048 : -2048);
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));
	tssi_delta_tmp = tssi_delta_tmp >> 12;
	DBGPRINT(RT_DEBUG_TRACE, ("==> delta db = %d\n", tssi_delta_tmp));

	*tssi_delta0 = *tssi_delta0 + tssi_delta_tmp;
	DBGPRINT(RT_DEBUG_TRACE, ("==> *tssi_delta0 = %d\n", *tssi_delta0));

	if (*tssi_delta0 > 31)
		*tssi_delta0 = 31;
	else if (*tssi_delta0 < -32)
		*tssi_delta0 = -32;

	DBGPRINT(RT_DEBUG_TRACE, ("\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("==> tssi_delta0 = %d\n", *tssi_delta0));
	DBGPRINT(RT_DEBUG_TRACE, ("\n"));

	return;
}


static VOID MT76x0ATE_IntTxAlcProcess(
	IN RTMP_ADAPTER *pAd)
{
	INT32 tssi_delta0;
	UINT32 MAC_Value;
	CHAR tssi_write;

	MT76x0ATE_GetTargetPower(pAd);

	RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &MAC_Value);
	DBGPRINT(RT_DEBUG_TRACE, ("(0x13B4) Before compensation 0x%08X\n", MAC_Value));
	tssi_delta0 = (CHAR)(MAC_Value & 0x3F);

	if ((tssi_delta0 & 0x20))
		tssi_delta0 -= 0x40;

	MT76x0ATE_EstimateDeltaPower(pAd, &tssi_delta0);
	tssi_write = tssi_delta0;

	MAC_Value &= 0xFFFFFFC0;
	MAC_Value |= (tssi_write & 0x3F);
	DBGPRINT(RT_DEBUG_TRACE, ("(0x13B4) After compensation 0x%08X\n", MAC_Value));
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_1, MAC_Value);

	return;
}


VOID MT76x0ATE_TSSI_DC_Calibration(
	IN  RTMP_ADAPTER *pAd)
{
	UCHAR RF_Value;
	UINT32 MAC_Value, BBP_Value;
	USHORT i = 0;

	pAd->hw_cfg.cent_ch = pAd->ate.Channel;

	if (pAd->hw_cfg.cent_ch > 14)
	{
		rlt_rf_read(pAd, RF_BANK0, RF_R67, &RF_Value);
		RF_Value &= 0xF0;
		rlt_rf_write(pAd, RF_BANK0, RF_R67, RF_Value);
	}

	/* Enable 9-bit I channel ADC and get TSSI DC point from BBP */
	{
		/* Bypass ADDA controls */
		MAC_Value = 0x60002237;
		RTMP_IO_WRITE32(pAd, RLT_RF_SETTING_0, MAC_Value);
		MAC_Value = 0xFFFFFFFF;
		RTMP_IO_WRITE32(pAd, RLT_RF_BYPASS_0, MAC_Value);

		/**********************************************************************/
		/* BBP Soft Reset */
		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value |= 0x00000001;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);

		RtmpusecDelay(1);

		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);
		/**********************************************************************/

		if (pAd->hw_cfg.cent_ch > 14)
		{
			/* EXT TSSI */
			/* Set avg mode on Q channel */
			BBP_Value = 0x00080055;
			RTMP_IO_WRITE32(pAd, CORE_R34, BBP_Value);
		}
		else
		{
			/* Set avg mode on I channel */
			BBP_Value = 0x00080050;
			RTMP_IO_WRITE32(pAd, CORE_R34, BBP_Value);
		}

		/* Enable TX with 0 DAC inputs */
		BBP_Value = 0x80000000;
		RTMP_IO_WRITE32(pAd, TXBE_R6, BBP_Value);

		/* Wait until avg done */
		do
		{
			RTMP_IO_READ32(pAd, CORE_R34, &BBP_Value);

			if ((BBP_Value&0x10) == 0)
				break;

			i++;
			if (i >= 100)
				break;

			RtmpusecDelay(10);

		} while(TRUE);

		/* Read TSSI value */
		RTMP_IO_READ32(pAd, CORE_R35, &BBP_Value);
		pAd->chipCap.tssi_current_DC = (CHAR)(BBP_Value&0xFF);

		/* stop bypass ADDA */
		/* MAC_Value = 0x0; */
		/* rtmp.HwMemoryWriteDword(RA_RF_SETTING_0, MAC_Value, 4); */
		MAC_Value = 0x0;
		RTMP_IO_WRITE32(pAd, RLT_RF_BYPASS_0, MAC_Value);

		/* Stop TX */
		BBP_Value = 0x0;
		RTMP_IO_WRITE32(pAd, TXBE_R6, BBP_Value);

		/**********************************************************************/
		/* BBP Soft Reset */
		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value |= 0x00000001;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);

		RtmpusecDelay(1);

		RTMP_IO_READ32(pAd, CORE_R4, &BBP_Value);
		BBP_Value &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, CORE_R4, BBP_Value);
		/**********************************************************************/
	}

	/* Restore */
	{
		if (pAd->hw_cfg.cent_ch > 14)
		{
			/* EXT TSSI */
			/* Reset tssi_cal */
			rlt_rf_read(pAd, RF_BANK0, RF_R67, &RF_Value);
			RF_Value &= 0xF0;
			RF_Value |= 0x04;
			rlt_rf_write(pAd, RF_BANK0, RF_R67, RF_Value);

		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s(): Current_TSSI_DC = %d\n", __FUNCTION__, pAd->chipCap.tssi_current_DC));
}


VOID MT76x0ATE_Enable9BitIchannelADC(
	IN PRTMP_ADAPTER pAd, 
	IN BOOLEAN bForDcCal)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 BBP_Value;
	INT32 wait = 0;

	if (pATEInfo->Channel > 14)
	{
		/* EXT TSSI */
		/* Set avg mode on Q channel */
		BBP_Value = 0x00080055;
	}
	else
	{
		/* 1. Set avg mode on I channel */
		BBP_Value = 0x00080050;
	}
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, BBP_Value);

	if (bForDcCal)
	{
		UINT32 OrgTxCount = pATEInfo->TxCount;

		/* Send 5 packets (write TX information) */
		pATEInfo->TxCount = 5;
		Set_ATE_Proc(pAd, "TXFRAME");
		pATEInfo->TxCount = OrgTxCount;
	}

	/* 2. Wait until avg done (read per 100us) */
	do
	{
		RTMP_BBP_IO_READ32(pAd, CORE_R34, &BBP_Value);

		if ((BBP_Value & 0x10) == 0)
			break;

		wait++;
		if (wait >= 100)
			break;

		RtmpusecDelay(100);

	} while (TRUE);

	ASSERT(wait < 100);
	ASSERT((BBP_Value & 0x10) == 0);
	if ((BBP_Value & 0x10) != 0)
	{
		BBP_Value &= ~(1 << 4);
		RTMP_BBP_IO_WRITE32(pAd, CORE_R34, BBP_Value);
		return;
	}

	/* 3. Read TSSI value */
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &BBP_Value);
	TSSI_Linear0 = (CHAR)(BBP_Value & 0xFF);
	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : CORE_R35 = 0x%X\n", BBP_Value));
	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : TSSI_Linear0 = (CHAR)(BBP_Value&0xFF) = 0x%X\n"
		, TSSI_Linear0));

	if (pATEInfo->Channel > 14)
	{
		TSSI_Linear0 = TSSI_Linear0 + 128; /* 5G: add total TSSI readings by 128 */
		DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : TSSI_Linear0 = TSSI_Linear0 + 128 = 0x%X\n"
			, TSSI_Linear0));
	}

	/* 4. Set Packet Info#1 mode */
	BBP_Value = 0x00080041;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, BBP_Value);

	/* 5. Read Info #1 */
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &BBP_Value);
	INFO_1 = (UCHAR)(BBP_Value&0xFF);

	/* 6. Set Packet Info#2 mode */
	BBP_Value = 0x00080042;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, BBP_Value);

	/* 7. Read Info #2 */
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &BBP_Value);
	INFO_2 = (UCHAR)(BBP_Value&0xFF);

	/* 8. Set Packet Info#3 mode */
	BBP_Value = 0x00080043;
	RTMP_BBP_IO_WRITE32(pAd, CORE_R34, BBP_Value);

	/* 9. Read Info #3 */
	RTMP_BBP_IO_READ32(pAd, CORE_R35, &BBP_Value);
	INFO_3 = (UCHAR)(BBP_Value&0xFF);

	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : TSSI_Linear0 = 0x%X\n", TSSI_Linear0));
	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : INFO_1 = 0x%X\n", INFO_1));
	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : INFO_2 = 0x%X\n", INFO_2));
	DBGPRINT(RT_DEBUG_TRACE, ("Enable9BitIchannelADC : INFO_3 = 0x%X\n", INFO_3));

	return;
}


static VOID MT76x0ATETssiCompensation(
	IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 parameter = 0;
	INT32 BandWidthSel = pATEInfo->TxWI.TXWI_N.BW;

	MT76x0ATE_Enable9BitIchannelADC(pAd, FALSE);	
	MT76x0ATE_IntTxAlcProcess(pAd);

	if ((pATEInfo->Channel > 14) || (BandWidthSel == BW_80))
	{
		/* only 2.4G needs DPD Calibration */
		/* BW_80 does not allowed in 2.4G band */
		DBGPRINT(RT_DEBUG_TRACE, ("channel = %d, bandwidth = %d\n"
				, pATEInfo->Channel, BandWidthSel));
		DBGPRINT(RT_DEBUG_TRACE, ("only support DPD calibration for 2.4G band\n"));
		return;	
	}

	parameter = pATEInfo->Channel;
	parameter |= (BandWidthSel << 8);
	CHIP_CALIBRATION(pAd, DPD_CALIBRATION, parameter);

	return;
}
#endif /* MT76x0_TSSI_CAL_COMPENSATION */


static VOID MT76x0ATE_AsicExtraPowerOverMAC(
	IN PRTMP_ADAPTER pAd)
{
	UINT32 ExtraPwrOverMAC = 0;
	UINT32 ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg9 = 0;

	/* 
		For OFDM_54 and HT_MCS_7, extra fill the corresponding register value into MAC 0x13D4 
		bit 21:16 -> HT MCS=7, VHT 2SS MCS=7
		bit 5:0 -> OFDM 54
	*/
	RTMP_IO_READ32(pAd, TX_PWR_CFG_1, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x00003F00) >> 8; /* Get Tx power for OFDM 54 */
	RTMP_IO_READ32(pAd, TX_PWR_CFG_2, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg7 |= (ExtraPwrOverMAC & 0x00003F00) << 8; /* Get Tx power for HT MCS 7 */			
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_7, ExtraPwrOverTxPwrCfg7);

	/* 
		For STBC_MCS_7, extra fill the corresponding register value into MAC 0x13DC 
		bit 5:0 -> HT/VHT STBC MCS=7
	*/
	RTMP_IO_READ32(pAd, TX_PWR_CFG_4, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg9 |= (ExtraPwrOverMAC & 0x00003F00) >> 8; /* Get Tx power for STBC MCS 7 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_9, ExtraPwrOverTxPwrCfg9);
	
	DBGPRINT(RT_DEBUG_INFO, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


VOID MT76x0ATE_VCO_CalibrationMode3(
	IN RTMP_ADAPTER *pAd)
{
	UCHAR RFValue = 0, Mode = 0;

	rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
	Mode = (RFValue & 0xF0);	
	if (Mode == 0x30)
	{
		DBGPRINT(RT_DEBUG_TRACE, ("%s - Calibration Mode: Open loop, closed loop, and amplitude\n", __FUNCTION__));
		/*
			Calibration Mode - Open loop, closed loop, and amplitude:
			B0.R06.[0]: 0
			B0.R06.[3:1] bp_close_code: 100
			B0.R05.[7:0] bp_open_code: 00
			B0.R04.[2:0] cal_bits: 000
			B0.R03.[2:0] startup_time: 011
			B0.R03.[6:4] settle_time: 111 (from MT7650E3_WiFi_RF_CR_20121202.xls)

		*/
		rlt_rf_read(pAd, RF_BANK0, RF_R06, &RFValue);
		RFValue &= ~(0x0F);
		RFValue |= (0x08);
		rlt_rf_write(pAd, RF_BANK0, RF_R06, RFValue);

		rlt_rf_read(pAd, RF_BANK0, RF_R05, &RFValue);
		if (RFValue != 0)
		{
			RFValue = 0;
			rlt_rf_write(pAd, RF_BANK0, RF_R05, RFValue);
		}

		rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
		RFValue &= ~(0x07);
		rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);

		rlt_rf_read(pAd, RF_BANK0, RF_R03, &RFValue);
		RFValue &= ~(0x77);
		RFValue |= (0x73);
		rlt_rf_write(pAd, RF_BANK0, RF_R03, RFValue);

		rlt_rf_read(pAd, RF_BANK0, RF_R04, &RFValue);
		RFValue = ((RFValue & ~0x80) | 0x80); 
		rlt_rf_write(pAd, RF_BANK0, RF_R04, RFValue);
	}
	
	return;
}


/* This function vesion is specific for Cameo */
/* It must be replaced by formal one for other customers */
VOID MT76x0ATE_Calibration(
	IN RTMP_ADAPTER *pAd,
	IN UCHAR Channel,
	IN BOOLEAN bPowerOn,
	IN BOOLEAN bFullCal)
{
	UINT32 MacReg = 0, reg_val = 0, reg_tx_alc = 0;

	/* MT7610 is 5G band only */
	if (IS_MT7610(pAd))
	{
		if (Channel <= 14)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("error - 7610: Channel = %d\n", Channel));
			return;
		}
	}

	/* MT7630 is 2.4G band only */
	if (IS_MT7630(pAd))
	{
		if (Channel > 14)
		{
			DBGPRINT(RT_DEBUG_TRACE, ("error - 7630: Channel = %d\n", Channel));
			return;
		}
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s - Channel = %d, bPowerOn = %d, bFullCal = %d\n", __FUNCTION__, Channel, bPowerOn, bFullCal));

	RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &reg_tx_alc); /* We need to restore 0x13b0 after calibration. */
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, 0x0);
	RtmpusecDelay(500);
	
	RTMP_IO_READ32(pAd, 0x2124, &reg_val); /* We need to restore 0x2124 after calibration. */
	MacReg = 0xFFFFFF7E; /* Disable 0x2704, 0x2708 controlled by MAC. */
	RTMP_IO_WRITE32(pAd, 0x2124, MacReg);

	if (bPowerOn)
	{
		/*
			Do Power on calibration.
			The calibration sequence is very important, please do NOT change it.
			1 XTAL Setup (already done in AsicRfInit)
			2 R-calibration
			3 VCO calibration
		*/

		/*
			2 R-calibration 
		*/
		CHIP_CALIBRATION(pAd, R_CALIBRATION, 0x0);

		MT76x0ATE_VCO_CalibrationMode3(pAd);
		RtmpusecDelay(1);
	}

	/*
		Do calibration.
		The calibration sequence is very important, please do NOT change it.
		1  RX DCOC calibration
		2  LC tank calibration
		3  TX Filter BW --> not ready yet @20121003
		4  RX Filter BW --> not ready yet @20121003
		5  TX RF LOFT 
		6  TX I/Q
		7  TX Group Delay		
		8  RX I/Q
		9  RX Group Delay
		10 TX 2G DPD
		11 TX 2G IM3 --> not ready yet @20121016
		12 TSSI Zero Reference --> not ready yet @20121016
		13 RX DCOC calibration
	*/
	if (bFullCal)
	{
		/*
			1. RXDC Calibration parameter
				0:Back Ground Disable
		*/
		CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION, 0);

		/*
			2. LC-Calibration parameter
				Bit[0:7]
					0: 2G
					1: 5G + External PA
					2: 5G + Internal PA
				Bit[8:15]
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (Channel > 14)
		{
			// TODO: check PA setting from EEPROM @20121016
			CHIP_CALIBRATION(pAd, LC_CALIBRATION, 0x1);
		}
		else
			CHIP_CALIBRATION(pAd, LC_CALIBRATION, 0x0);

		/*
			3,4. BW-Calibration
				Bit[0:7] (0:RX, 1:TX)
				Bit[8:15] (0:BW20, 1:BW40, 2:BW80)
				Bit[16:23]
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/

		/*
			5. RF LOFT-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration

		*/
		if (Channel > 14)
		{
			CHIP_CALIBRATION(pAd, LOFT_CALIBRATION, 0x1);
		}
		else
			CHIP_CALIBRATION(pAd, LOFT_CALIBRATION, 0x0);

		/*
			6. TXIQ-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (Channel > 14)
		{
			CHIP_CALIBRATION(pAd, TXIQ_CALIBRATION, 0x1);
		}
		else
		{
			CHIP_CALIBRATION(pAd, TXIQ_CALIBRATION, 0x0);
		}
		/*			
			7. TX Group-Delay Calibation parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (Channel > 14)
		{
			CHIP_CALIBRATION(pAd, TX_GROUP_DELAY_CALIBRATION, 0x1);
		}
		else
		{
			CHIP_CALIBRATION(pAd, TX_GROUP_DELAY_CALIBRATION, 0x0);
		}
		/*
			8. RXIQ-Calibration parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
					
			9. RX Group-Delay Calibation parameter
				Bit[0:7] (0:G-Band, 1: A-Band)
				Bit[8:15] 
					0: Full Calibration
					1: Partial Calibration
					2: G-Band Full Calibration + Save
					3: A-Band (Low) Full Calibration + Save
					4: A-Band (Mid) Full Calibration + Save
					5: A-Band (High) Full Calibration + Save
					6: G-Band Restore Calibration
					7: A-Band (Low) Restore Calibration
					8: A-Band (Mid) Restore Calibration
					9: A-Band (High) Restore Calibration
		*/
		if (Channel > 14)
		{
			CHIP_CALIBRATION(pAd, RXIQ_CALIBRATION, 0x1);
			CHIP_CALIBRATION(pAd, RX_GROUP_DELAY_CALIBRATION, 0x1);
		}
		else
		{
			CHIP_CALIBRATION(pAd, RXIQ_CALIBRATION, 0x0);
			CHIP_CALIBRATION(pAd, RX_GROUP_DELAY_CALIBRATION, 0x0);
		}
		/* 
			10. TX 2G DPD - Only 2.4G needs to do DPD Calibration. 
		*/
		if (Channel <= 14)
			CHIP_CALIBRATION(pAd, DPD_CALIBRATION, 0x0);
	}
	else
	{
	}

	/*
		14. RXDC Calibration parameter
			1:Back Ground Enable
	*/
	CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION, 1);
	RtmpusecDelay(100000); // TODO: check response packet from FW
	
	/* Restore 0x2124 & TX_ALC_CFG_0 after calibration completed */
	RTMP_IO_WRITE32(pAd, 0x2124, reg_val);
	RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, reg_tx_alc);
}


#ifdef SINGLE_SKU_V2
static VOID MT76x0ATE_CalculateTxpower(
	IN  BOOLEAN bMinus,
	IN  USHORT InputTxpower,
	IN  USHORT DeltaTxpower,
	IN  UCHAR *pTxpower1,
	IN  UCHAR *pTxpower2)
{
	UCHAR t1, t2;
	
	if (bMinus == FALSE)
	{
		if (InputTxpower & 0x20)
		{
			t1 = (UCHAR)((InputTxpower & 0x3F) +  DeltaTxpower);			
			if (t1 > 0x3F)
				t1 = 0x3F;
		}
		else
		{
			t1 = (InputTxpower & 0x1F) + (DeltaTxpower);
			if (t1 > 0x1F)
				t1 = 0x1F;
		}

		if (InputTxpower & 0x2000)
		{
			t2 = (UCHAR)(((InputTxpower & 0x3F00) >> 8) + DeltaTxpower);
			if (t2 > 0x3F)
				t2 = 0x3F;
		}
		else
		{
			t2 = ((InputTxpower & 0x1F00) >> 8) + (DeltaTxpower);
			if (t2 > 0x1F)
				t2 = 0x1F;
		}			
	}
	else
	{
		if (InputTxpower & 0x20)
		{
			t1 = (InputTxpower & 0x3F) - (DeltaTxpower);
			if (t1 > 0x3F)
				t1 = 0x3F;
		}
		else
		{
			t1 = (InputTxpower & 0x1F) - (DeltaTxpower);
			if (t1 > 0x1F)
				t1 = 0x1F;
		}

		if (InputTxpower & 0x2000)
		{
			t2 = ((InputTxpower & 0x3F00) >> 8) - (DeltaTxpower);
			if (t2 > 0x3F)
				t2 = 0x3F;
		}
		else
		{
			t2 = ((InputTxpower & 0x1F00) >> 8) - (DeltaTxpower);
			if (t2 > 0x1F)
				t2 = 0x1F;
		}			
	}
	*pTxpower1 = t1;
	*pTxpower2 = t2;
}

#define ATE_EEPROM_TXPOWER_BYRATE_STBC	(0xEC)
#define ATE_EEPROM_TXPOWER_BYRATE_5G	(0x120)

/* Read per-rate Tx power */
VOID MT76x0AteReadTxPwrPerRate(
	IN RTMP_ADAPTER *pAd)
{
	UINT32 data;
	USHORT value;
	UCHAR TxPwrBw40ABand, TxPwrBw80ABand, TxPwrBw40GBand;
	UCHAR t1, t2, t3, t4;
	BOOLEAN bMinusBw40ABand = FALSE, bMinusBw80ABand = FALSE,bMinusBw40GBand = FALSE;

    DBGPRINT(RT_DEBUG_TRACE, ("%s() -->\n", __FUNCTION__));
	
	/* ATE does not implement bw delta power in per-rate registers */
	bMinusBw40ABand = FALSE;
	bMinusBw80ABand = FALSE;
	bMinusBw40GBand = FALSE;
	TxPwrBw40ABand = 0;
	TxPwrBw80ABand = 0;
	TxPwrBw40GBand = 0;
	
	RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_BYRATE_20MHZ_2_4G, value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 2), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> OFDM 12M/18M
		bit 21:16 -> OFDM 6M/9M
		bit 13:8 -> CCK 5.5M/11M
		bit 5:0 -> CCK 1M/2M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx20MPwrCfgGBand[0] = data; /* TX_PWR_CFG_0, MAC 0x1314 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx20MPwrCfgGBand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 4), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 6), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> HT MCS=2,3, VHT 1SS MCS=2,3
		bit 21:16 -> HT MCS=0,1, VHT 1SS MCS=0,1
		bit 13:8 -> OFDM 48M
		bit 5:0 -> OFDM 24M/36M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[0] = data; /* TX_PWR_CFG_1, MAC 0x1318 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 8), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	/*
		bit 13:8 -> HT MCS=6, VHT 1SS MCS=6
		bit 5:0 -> MCS=4,5, VHT 1SS MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[1] = data; /* TX_PWR_CFG_2, MAC 0x131C */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[1] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 14), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t3, &t4);
	/* 
		bit 29:24 -> HT/VHT STBC MCS=2, 3
		bit 21:16 -> HT/VHT STBC MCS=0, 1
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx40MPwrCfgGBand[2] = data; /* TX_PWR_CFG_3, MAC 0x1320 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[2] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (EEPROM_TXPOWER_BYRATE_20MHZ_2_4G + 16), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40GBand, value, TxPwrBw40GBand, &t1, &t2);
	/* 
		bit 13:8 -> HT/VHT STBC MCS=6
		bit 5:0 -> HT/VHT STBC MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgGBand[3] = data; /* TX_PWR_CFG_4, MAC 0x1324 */			
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgGBand[3] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, ATE_EEPROM_TXPOWER_BYRATE_5G, value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> OFDM 12M/18M
		bit 21:16 -> OFDM 6M/9M
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx20MPwrCfgABand[0] = data; /* TX_PWR_CFG_0, MAC 0x1314 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx20MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_5G + 2), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_5G + 4), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> HT MCS=2,3, VHT 1SS MCS=2,3
		bit 21:16 -> HT MCS=0,1, VHT 1SS MCS=0,1
		bit 13:8 -> OFDM 48M
		bit 5:0 -> OFDM 24M/36M
	*/
	data = (t4 << 24) | (t3 << 16) | (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[0] = data; /* TX_PWR_CFG_1, MAC 0x1318 */			
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_5G + 6), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
	/*
		bit 13:8 -> HT MCS=6, VHT 1SS MCS=6
		bit 5:0 -> MCS=4,5, VHT 1SS MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[1] = data; /* TX_PWR_CFG_2, MAC 0x131C */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[1] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_STBC), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t3, &t4);
	/* 
		bit 29:24 -> HT/VHT STBC MCS=2, 3
		bit 21:16 -> HT/VHT STBC MCS=0, 1
	*/
	data = (t4 << 24) | (t3 << 16); 
	pAd->Tx40MPwrCfgABand[2] = data; /* TX_PWR_CFG_3, MAC 0x1320 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[2] = 0x%X\n", __FUNCTION__, data));

	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_STBC + 2), value);
	MT76x0ATE_CalculateTxpower(bMinusBw40ABand, value, TxPwrBw40ABand, &t1, &t2);
	/* 
		bit 13:8 -> HT/VHT STBC MCS=6
		bit 5:0 -> HT/VHT STBC MCS=4,5
	*/
	data = (t2 << 8) | t1; 
	pAd->Tx40MPwrCfgABand[3] = data; /* TX_PWR_CFG_4, MAC 0x1324 */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx40MPwrCfgABand[3] = 0x%X\n", __FUNCTION__, data));
	
	RT28xx_EEPROM_READ16(pAd, (ATE_EEPROM_TXPOWER_BYRATE_5G + 12), value);
	MT76x0ATE_CalculateTxpower(bMinusBw80ABand, value, TxPwrBw80ABand, &t3, &t4);
	/* 
		bit 29:24 -> VHT 1SS MCS=9
		bit 21:16 -> VHT 1SS MCS=8
	*/
	data = (t3 << 24) | (t3 << 16); 
#ifdef DOT11_VHT_AC
	pAd->Tx80MPwrCfgABand[0] = data; /* TX_PWR_CFG_8, MAC 0x13D8 */
#endif /* DOT11_VHT_AC */
	DBGPRINT_RAW(RT_DEBUG_TRACE, ("%s: Tx80MPwrCfgABand[0] = 0x%X\n", __FUNCTION__, data));

	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0, pAd->Tx20MPwrCfgABand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_1, pAd->Tx40MPwrCfgABand[0]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_2, pAd->Tx40MPwrCfgABand[1]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_3, pAd->Tx40MPwrCfgABand[2]);
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_4, pAd->Tx40MPwrCfgABand[3]);
#ifdef DOT11_VHT_AC
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, pAd->Tx80MPwrCfgABand[0]);
#endif /* DOT11_VHT_AC */

	/* It is done in ATEPeriodicExec */ 
/*	MT76x0ATE_AsicExtraPowerOverMAC(pAd); */
	
    DBGPRINT(RT_DEBUG_TRACE, ("%s: <--\n", __FUNCTION__));
}
#endif /* SINGLE_SKU_V2 */

#ifdef RTMP_MAC_PCI
struct _ATE_CHIP_STRUCT RALINK6590 =
{
	/* functions */
	.ChannelSwitch = MT76x0ATEAsicSwitchChannel,
	.TxPwrHandler = MT76x0ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = NULL,
	.AsicSetTxRxPath = NULL,
#ifdef MT76x0_TSSI_CAL_COMPENSATION
	.AdjustTxPower = MT76x0ATETssiCompensation,
#else
	.AdjustTxPower = NULL,
#endif /* MT76x0_TSSI_CAL_COMPENSATION */
	.AsicExtraPowerOverMAC = MT76x0ATE_AsicExtraPowerOverMAC,
	.TemperCompensation = NULL,
	
	/* command handlers */
	.Set_BW_Proc = MT76x0_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = MT76x0_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,	
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,
};
#endif /* RTMP_MAC_PCI */


#endif /* RT65xx */

