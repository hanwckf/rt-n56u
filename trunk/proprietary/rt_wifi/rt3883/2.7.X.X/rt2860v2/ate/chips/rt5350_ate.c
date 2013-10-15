/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rt5350_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT5350

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT5350

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern FREQUENCY_ITEM FreqItems3020_Xtal20M[];
extern FREQUENCY_ITEM RtmpFreqItems3020[];
extern UCHAR NUM_OF_3020_CHNL;

#ifdef RTMP_INTERNAL_TX_ALC
extern TX_POWER_TUNING_ENTRY_STRUCT RT5350_TxPowerTuningTable[];
extern UINT32 RT5350_desiredTSSIOverCCK[4];
/* The desired TSSI over OFDM */
extern UINT32 RT5350_desiredTSSIOverOFDM[8];
/* The desired TSSI over HT */
extern UINT32 RT5350_desiredTSSIOverHT[16];
extern UINT32 RT5350_desiredTSSIOverHT40[16];
extern UCHAR CCK_Rate2MCS(PRTMP_ADAPTER pAd);
extern UCHAR OFDM_Rate2MCS(PRTMP_ADAPTER pAd);
extern INT32 TSSIDelta2PowDelta(UINT32 TSSI_x_10000, UINT32 TSSI_ref);

/*
==========================================================================
        Description:
                Get the desired TSSI based on the latest packet
                in ATE mode.

        Arguments:
                pAd
		pBbpR49

        Return Value:
                The desired TSSI
==========================================================================
 */
UINT32 RT5350_ATEGetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd,
	OUT PUCHAR				pBbpR49)
{
	UINT32			desiredTSSI = 0;
	UCHAR			MCS = 0;
	UCHAR			BbpR47 = 0;
	UCHAR			TssiInfo0 = 0;
	UCHAR			TssiInfo1 = 0;
	UCHAR			TssiInfo2 = 0;
	UCHAR			count;
	UCHAR			BBP_Bandwidth = 0;
	UCHAR			ofdm_rate = 0, dot11b_rate = 0;
	
	if (pAd->ate.TxWI.BW == BW_40)
	{
		BBP_Bandwidth = BW_40;
	}
	else
	{
		BBP_Bandwidth = BW_20;
	}

	/* Get TSSI_INFO */
	for (count=0;count<100;count++)
	{
	    ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);

	    if (!(BbpR47 & (1<<2)))
		{
			/* self-cleared when the TSSI_INFO is updated */
			/* Get TSSI_INFO0 = tssi_report[7:0] */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 &= ~0x3;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo0);

			/* If TSSI reading is not within 0x0~0x7C, then treated it as 0. */
			if (TssiInfo0 > 0x7C)
			{
				DBGPRINT(RT_DEBUG_TRACE, ("TSSI: BBP_R49=%X is native value\n", TssiInfo0));
				*pBbpR49 = TssiInfo0 = 0;
			}
			else
			{
				*pBbpR49 = TssiInfo0;
			}

			/* Get TSSI_INFO1 = tssi_report[15:8] */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			BbpR47 &= ~0x3;
			BbpR47 |= 1;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo1);

			switch (TssiInfo1 & 0x03)
			{
				case 0: /* CCK */
					dot11b_rate = (TssiInfo1 >> 4) & 0xF; /* tssi_report[13:12]=11b rate */

					switch(dot11b_rate)
					{
						case 8:
							MCS = 0;        /* Long preamble CCK 1Mbps */
							break;
						case 9:
							MCS = 1;        /* Long preamble CCK 2Mbps */
							break;
						case 10:
							MCS = 2;        /* Long preamble CCK 5.5Mbps */
							break;
						case 11:
							MCS = 3;        /* Long preamble CCK 11Mbps */
							break;
						default:
							MCS = 0;
							break;
					}
					desiredTSSI = RT5350_desiredTSSIOverCCK[MCS];
					DBGPRINT(RT_DEBUG_INFO, ("CCK: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;

				case 1: /* OFDM */
					ofdm_rate = (TssiInfo1 >> 4) & 0xF; /* rssi_15:12]=ofdm_rate */

					switch (ofdm_rate)
					{
						case 0xb:
							MCS = 0; /* 6Mbps Rate */
							break;
						case 0xf:
							MCS = 1; /* 9Mbps Rate */
							break;
						case 0xa:
							MCS = 2; /* 12Mbps Rate */
							break;
						case 0xe:
							MCS = 3; /* 18Mbps Rate */
							break;
						case 0x9:
							MCS = 4; /* 24Mbps Rate */
							break;
						case 0xd:
							MCS = 5; /* 36Mbps Rate */
							break;
						case 0x8:
							MCS = 6; /* 48Mbps Rate */
							break;
						case 0xc:
							MCS = 7; /* 54Mbps Rate */
							break;
						default:
							MCS = 6;
	                        break;
					};
					desiredTSSI = RT5350_desiredTSSIOverOFDM[MCS];
					DBGPRINT(RT_DEBUG_INFO, ("OFDM: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;

				case 2: /* HT */
					DBGPRINT(RT_DEBUG_INFO, ("mixed mode or green-field mode\n"));
					/* Get TSSI_INFO2 = tssi_report[23:16] */
					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
					BbpR47 &= ~0x3;
					BbpR47 |= 1<<1;
					ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

					ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo2);
					MCS = TssiInfo2 & 0x7F; /* tssi_report[22:16]=MCS */

					if ((BBP_Bandwidth == BW_40)
						&& ((MCS == 5) || (MCS == 6) || (MCS == 7)))
					{
						desiredTSSI = RT5350_desiredTSSIOverHT40[MCS];
					}
					else
					{
						desiredTSSI = RT5350_desiredTSSIOverHT[MCS];
					}

					DBGPRINT(RT_DEBUG_INFO, ("HT: desiredTSSI = %d, MCS = %d\n", desiredTSSI, MCS));
					break;
			}
			break;
		}
	}

	/* clear TSSI_UPDATE_REQ first */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 &= ~0x7;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	/* write 1 to enable TSSI_INFO update */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
	BbpR47 |= (1<<2); /* TSSI_UPDATE_REQ */
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47 );

	return desiredTSSI;
}


INT ATE_GET_TSSI(
	IN PRTMP_ADAPTER pAd,
	IN	INT	MODE,
	IN	INT	MCS,
	OUT PUCHAR  pBbpR49)
{
	UCHAR		BbpR47=0;
	UCHAR		TssiInfo0=0;
	UCHAR		TssiInfo1=0;
	UCHAR		TssiInfo2=0;
	UCHAR		i;

	if (IS_RT5350(pAd))
	{
		/* clear TSSI_UPDATE_REQ first */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 &= ~0x7;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);

		/* write 1 to enable TSSI_INFO update */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
		BbpR47 |= (1<<2); /* TSSI_UPDATE_REQ */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
		RtmpOsMsDelay(100);

		/* Get TSSI_INFO0 = tssi_report[7:0] */
		for (i = 0; i < 100; i++)
		{
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
			if (!(BbpR47 & (1 << 2)))
			{ 
				/* self-cleared when the TSSI_INFO is updated */
				/* Get TSSI_INFO0 = tssi_report[7:0] */
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
				BbpR47 &= ~0x3;
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo0);
				
				/* If TSSI reading is not within 0x0~0x7C, then treated it as 0. */
				if (TssiInfo0 > 0x7C)
				{
					DBGPRINT(RT_DEBUG_TRACE, ("TSSI: BBP_R49=%X is native value\n", TssiInfo0));
					*pBbpR49 = TssiInfo0 = 0;
					continue;
				}
				else
				{
					*pBbpR49 = TssiInfo0;
				}
		
				/* Get TSSI_INFO1 = tssi_report[15:8] */
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
				BbpR47 &= ~0x3;
				BbpR47 |= 1;
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo1);

				if ((TssiInfo1 & 0x3) != MODE)
					continue;

				switch (TssiInfo1 & 0x3)
				{
					UCHAR pkt_MCS;

					case MODE_CCK: /* CCK */
						pkt_MCS = CCK_Rate2MCS(pAd);
						DBGPRINT(RT_DEBUG_TRACE, ("CCK: MCS = %d\n", pkt_MCS));
						if (MCS != pkt_MCS)
							continue;
						break;
					case MODE_OFDM: /* OFDM */

						pkt_MCS = OFDM_Rate2MCS(pAd);
						DBGPRINT(RT_DEBUG_TRACE, ("OFDM: MCS = %d\n", pkt_MCS));
						if (MCS != pkt_MCS)
							continue;
						break;
					case MODE_HTMIX: /* HT */
						/* Get TSSI_INFO2 = tssi_report[23:16] */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BbpR47);
						BbpR47 &= ~0x3;
						BbpR47 |= 1<<1;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BbpR47);
		
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &TssiInfo2);
			    			pkt_MCS = TssiInfo2 & 0x7F; /* tssi_report[22:16]=MCS */
						DBGPRINT(RT_DEBUG_TRACE, ("HT: MCS = %d\n", pkt_MCS));
						if (MCS != pkt_MCS) /* tssi_report[22:16]=MCS */
							continue;
						break;
				}
				break;
			}
		}

	}

	return TRUE;
}


INT RT5350_ATETssiCalibrationExtend(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg)
{    
	PATE_INFO pATEInfo = &(pAd->ate);
	INT 		i;
	USHORT	EEPData, EEPData2;                              
	UINT32	TSSI_x_10000[15];
	INT32	TSSI_power_delta[15];
	UCHAR	TSSI_CH1, TSSI_CH7, TSSI_CH13;
	INT		TSSI_CH1_10000, TSSI_CH7_10000, TSSI_CH13_10000;
	TssiDeltaInfo TSSI_x_h, TSSI_x_l;

	if (!IS_RT5350(pAd))                
	{
		DBGPRINT_ERR(("Not support TSSI calibration!!!\n"));
		DBGPRINT_ERR(("Not RT5350!!!\n"));

		return FALSE;
	}
	else
	{                              
		UCHAR	BSSID_ADDR[MAC_ADDR_LEN] = {0x11, 0x22, 0x33, 0x44, 0x55, 0x66};

		pAd->TxPowerCtrl.bInternalTxALC = TRUE;
		RT5350_InitDesiredTSSITable(pAd);

		/* update EEPROM power value to pAd struct */
		RTMPReadChannelPwr(pAd);

		/* start TX at 54Mbps Channel 1 */
		NdisZeroMemory(pATEInfo, sizeof(struct _ATE_INFO));
		pATEInfo->TxCount = 100000;
		pATEInfo->TxLength = 1024;
		COPY_MAC_ADDR(pATEInfo->Addr1, BROADCAST_ADDR);
		COPY_MAC_ADDR(pATEInfo->Addr2, pAd->PermanentAddress);
		COPY_MAC_ADDR(pATEInfo->Addr3, BSSID_ADDR);                     
		
		Set_ATE_TX_MODE_Proc(pAd, "1");		/* MODE_OFDM */
		Set_ATE_TX_MCS_Proc(pAd, "7");		/* 54Mbps */
		Set_ATE_TX_BW_Proc(pAd, "0");		/* 20MHz */
	
		/* read frequency offset from EEPROM */                         
		RT28xx_EEPROM_READ16(pAd, EEPROM_FREQ_OFFSET, EEPData);
		pATEInfo->RFFreqOffset = (UCHAR) (EEPData & 0xff);

		/* set channel 1 power value calibrated DAC */
		pATEInfo->TxPower0 = pAd->TxPower[0].Power;
		pATEInfo->Channel = 1;

		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(200000);	
		ATE_GET_TSSI(pAd, MODE_OFDM, 7, &TSSI_CH1);
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI CALIBRATION: Channel[1] TSSI=%x\n", TSSI_CH1)); 

		if (TSSI_CH1 < 0x18)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[1] TSSI is abnormal, set to 0x18\n")); 
			TSSI_CH1 = 0x18;
		}

		if (TSSI_CH1 > 0x33)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[1] TSSI is abnormal, set to 0x33\n")); 
			TSSI_CH1 = 0x33;
		}

		/* set channel 7 power value calibrated DAC */
		pATEInfo->TxPower0 = pAd->TxPower[6].Power;
		pATEInfo->Channel = 7;

		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(200000);	
		ATE_GET_TSSI(pAd, MODE_OFDM, 7, &TSSI_CH7);
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI CALIBRATION: Channel[7] TSSI=%x\n", TSSI_CH7)); 

		if (TSSI_CH7 < 0x18)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[7] TSSI is abnormal, set to 0x18\n")); 
			TSSI_CH7 = 0x18;
		}

		if (TSSI_CH7 > 0x33)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[7] TSSI is abnormal, set to 0x33\n")); 
			TSSI_CH7 = 0x33;
		}

		/* set channel 13 power value calibrated DAC */ 
		pATEInfo->TxPower0 = pAd->TxPower[12].Power;
		pATEInfo->Channel = 13;

		Set_ATE_Proc(pAd, "TXFRAME"); 
		RTMPusecDelay(200000);	
		ATE_GET_TSSI(pAd, MODE_OFDM, 7, &TSSI_CH13);
		DBGPRINT(RT_DEBUG_TRACE, ("TSSI CALIBRATION: Channel[13] TSSI=%x\n", TSSI_CH13)); 

		if (TSSI_CH13 < 0x18)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[13] TSSI is abnormal, set to 0x18\n")); 
			TSSI_CH13 = 0x18;
		}

		if (TSSI_CH13 > 0x33)
		{
			DBGPRINT_ERR(("TSSI CALIBRATION: Channel[13] TSSI is abnormal, set to 0x33\n")); 
			TSSI_CH13 = 0x33;
		}

		TSSI_CH1_10000 =  TSSI_CH1 * 10000;
		TSSI_CH7_10000 =  TSSI_CH7 * 10000;
		TSSI_CH13_10000 =  TSSI_CH13 * 10000;
		TSSI_x_10000[1] = TSSI_CH1_10000;
		TSSI_x_10000[7] = TSSI_CH7_10000;
		TSSI_x_10000[13] = TSSI_CH13_10000;

		for (i = 2; i < 7; i++)
			TSSI_x_10000[i] = TSSI_CH1_10000 + (i - 1) * ((TSSI_CH7_10000 - TSSI_CH1_10000) / (7 - 1));

		for (i = 8; i <= 14; i++) 
			TSSI_x_10000[i] = TSSI_CH7_10000 + (i - 7) * ((TSSI_CH13_10000 - TSSI_CH7_10000) / (13 - 7));

		for (i = 1; i <= 14; i++)
		{
			TSSI_power_delta[i] = TSSIDelta2PowDelta(TSSI_x_10000[i], TSSI_CH7);	
			DBGPRINT(RT_DEBUG_TRACE, ("TSSI CALIBRATION: Channel[%d] TSSI_x=%d\n", i, TSSI_x_10000[i])); 
			DBGPRINT(RT_DEBUG_TRACE, ("TSSI CALIBRATION: Channel[%d] TSSI_power_delta=%d\n", i, TSSI_power_delta[i])); 
		}
		
		/* 2.4G internal ALC reference value (channel 7) */
		EEPData = 0x0 | (TSSI_CH7_10000 / 10000);
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_REF_OFFSET, EEPData);
		
		/* Channel 2 TSSI delta:Channel 1 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[1];
		TSSI_x_h.delta = TSSI_power_delta[2];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH1_CH2, EEPData);

		/* Channel 4 TSSI delta:Channel 3 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[3];
		TSSI_x_h.delta = TSSI_power_delta[4];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH3_CH4, EEPData);

		/* Channel 6 TSSI delta:Channel 5 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[5];
		TSSI_x_h.delta = TSSI_power_delta[6];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH5_CH6, EEPData);

		/* Channel 8 TSSI delta:Channel 7 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[7];
		TSSI_x_h.delta = TSSI_power_delta[8];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH7_CH8, EEPData);

		/* Channel 10 TSSI delta:Channel 9 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[9];
		TSSI_x_h.delta = TSSI_power_delta[10];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH9_CH10, EEPData);

		/* Channel 12 TSSI delta:Channel 11 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[11];
		TSSI_x_h.delta = TSSI_power_delta[12];
		EEPData = TSSI_x_h.delta << 4 | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_DELTA_CH11_CH12, EEPData);

		/* Channel 14 TSSI delta:Channel 13 TSSI delta */
		TSSI_x_l.delta = TSSI_power_delta[13];
		TSSI_x_h.delta = TSSI_power_delta[14];

		RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_DELTA_CH13_CH14 + 1, EEPData2);
		EEPData = (EEPData2 << 8) | (TSSI_x_h.delta << 4) | TSSI_x_l.delta;
		RT28xx_EEPROM_WRITE16(pAd,  EEPROM_TSSI_DELTA_CH13_CH14, EEPData);
	}

	/* disable legacy ALC and set TSSI enabled and TSSI extend mode to EEPROM */
	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_ENABLE, EEPData);
	/* disable legacy ALC */
	EEPData &= ~(1 << 1);
	/* enable TSSI */
	EEPData |= (1 << 13);
	RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_ENABLE, EEPData);
	RTMPusecDelay(10);

	RT28xx_EEPROM_READ16(pAd, EEPROM_TSSI_MODE_EXTEND, EEPData);
	/* set extended TSSI mode */
	EEPData |= (1 << 15);
	RT28xx_EEPROM_WRITE16(pAd, EEPROM_TSSI_MODE_EXTEND, EEPData);
	RTMPusecDelay(10);

	return TRUE;
}
#endif /* RTMP_INTERNAL_TX_ALC */


/*
==========================================================================
    Description:

	AsicSwitchChannel() dedicated for RT5350 ATE.
    
==========================================================================
*/
VOID RT5350ATEAsicSwitchChannel(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UINT32 Value = 0;
	UINT32 step=0;
	CHAR TxPwer = 0, TxPwer2 = 0;
	UCHAR index = 0, BbpValue = 0, Channel = 0;
	/* added to prevent RF register reading error */
	UCHAR RFValue = 0, RFValue2 = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

	for (index = 0; index < NUM_OF_3020_CHNL; index++)
	{
		/* Please don't change "RtmpFreqItems3020" to "FreqItems3020" ! */
		if (Channel == RtmpFreqItems3020[index].Channel)
		{
			/* programming channel parameters */
			step = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

			/* Programming channel parameters */
			if(step & (1<<20))
			{ 
				/* Xtal=40M */
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, RtmpFreqItems3020[index].N);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, RtmpFreqItems3020[index].K);
				RFValue = 0x9F;
			}
			else
			{
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R08, FreqItems3020_Xtal20M[index].N);
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R09, FreqItems3020_Xtal20M[index].K);
				RFValue = 0x1F;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R13, (UCHAR)RFValue);

			RFValue = 0x4A;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R11, (UCHAR)RFValue);

			RFValue = 0x46;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R12, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R30, (PUCHAR)&RFValue);

			if (pATEInfo->TxWI.BW == BW_20)
			{
				RFValue &= ~(0x06); /* 20MBW tx_h20M=0,rx_h20M=0 */
			}
			else
			{
				RFValue |= 0x06; /* 40MBW tx_h20M=1,rx_h20M=1 */
			}

			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R30, (UCHAR)RFValue);

			if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
			{
				RFValue = 0xC0;
			}
			else
			{
				RFValue = 0x80;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R32, (UCHAR)RFValue);

			RFValue = 0x04;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, (UCHAR)RFValue);

			if (pATEInfo->TxWI.PHYMODE == MODE_CCK)
			{
				RFValue = 0x47;
			}
			else
			{
				RFValue = 0x43;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R55, (UCHAR)RFValue);

			RFValue = 0xC2;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R56, (UCHAR)RFValue);

			switch (Channel)
			{
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
					RFValue = 0x0B;
					break;
				case 8:
				case 9:
					RFValue = 0x0A;
					break;
				case 10:
					RFValue = 0x09;
					break;
				case 11:
					RFValue = 0x08;
					break;
				case 12:
				case 13:
					RFValue = 0x07;
					break;
				case 14:
					RFValue = 0x06;
					break;
				default:
					RFValue = 0x0B;
					break;
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R59, (UCHAR)RFValue);

			RFValue2 = pATEInfo->TxPower0 | (1 << 7); /* bit 7 = 1 */
			RFValue2 &=  ~(1 << 6); /* bit 6 = 0 */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, RFValue2);

			RFValue = 0x04;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R53, (UCHAR)RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);

			RTMPusecDelay(2000);

			/* set BW */
			/* RF_R24 is reserved bits */
			RFValue = 0; 
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)RFValue);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);					

			/* Enable RF tuning, this must be in the last, RF_R03=RF_R07. */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R03, (PUCHAR)&RFValue);
			RFValue = RFValue | 0x80; /* bit 7=vcocal_en */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R03, (UCHAR)RFValue);

			RtmpOsMsDelay(2);

			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R49, RFValue & 0xfe); /* clear update flag */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, (PUCHAR)&RFValue);

			/* Antenna */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R01, (PUCHAR)&RFValue);
			RFValue = (RFValue & 0x03) | 0x01;
			if (pATEInfo->RxAntennaSel == 1)
			{
				RFValue = RFValue | 0x04; /* rx0_en */
			}
			else if (pATEInfo->RxAntennaSel == 2)
			{
				RFValue = RFValue | 0x10; /* rx1_en */
			}
			else 
			{
				RFValue = RFValue | 0x14; /* rx0_en and rx1_en */
			}

			if (pATEInfo->TxAntennaSel == 1)
			{
				RFValue = RFValue | 0x08; /* tx0_en */
			}
			else if (pATEInfo->TxAntennaSel == 2)
			{
				RFValue = RFValue | 0x20; /* tx1_en */
			}
			else
			{
				RFValue = RFValue | 0x28; /* tx0_en and tx1_en */
			}
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R01, (UCHAR)RFValue);

			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
			BbpValue = BbpValue & 0x24; /* clear bit 0,1,3,4,6,7 */

			if (pATEInfo->RxAntennaSel == 1)
			{
				BbpValue &= ~0x3; /* BBP_R3[1:0]=00 (ADC0) */
			}
			else if (pATEInfo->RxAntennaSel == 2)
			{
				BbpValue &= ~0x3; 
				BbpValue |= 0x1; /* BBP_R3[1:0]=01 (ADC1) */
			}
			else
			{
				/* RXANT_NUM >= 2, must turn on Bit 0 and 1 for all ADCs */
				BbpValue |= 0x3;  /* BBP_R3[1:0]=11 (All ADCs) */
				if (pAd->Antenna.field.RxPath == 3)
					BbpValue |= (1 << 4);
				else if (pAd->Antenna.field.RxPath == 2)
					BbpValue |= (1 << 3);  /* BBP_R3[3]=1 (2R) */
				else if (pAd->Antenna.field.RxPath == 1)
					BbpValue |= (0x0);
			}

			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

			if (pATEInfo->TxWI.BW == BW_20)
			{
				/* set BBP R4 = 0x40 for BW = 20 MHz */
				BbpValue = 0x40;
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
			}
			else
			{
				/* set BBP R4 = 0x50 for BW = 40 MHz */
				BbpValue = 0x50;
				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BbpValue);
			}

			/* latch channel for future usage */
			pAd->LatchRfRegs.Channel = Channel;
			break;
		}
	}

	/* Change BBP setting during switch from a->g, g->a */
	if (Channel <= 14)
	{
		UINT32 TxPinCfg = 0x00050F0A;/* 2007.10.09 by Brian : 0x0005050A ==> 0x00050F0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Gary : 2010/0721 */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R86, 0x38);

		/* Rx High power VGA offset for LNA select */
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

		/* 2.4 G band selection PIN */
		RTMP_IO_READ32(pAd, TX_BAND_CFG, &Value);
		Value &= (~0x6);
		Value |= (0x04);
		RTMP_IO_WRITE32(pAd, TX_BAND_CFG, Value);

		if (pATEInfo->TxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFFFF3;
		}
		else if (pATEInfo->TxAntennaSel == 2)
		{
			TxPinCfg &= 0xFFFFFFFC;
		}
		else
		{
			TxPinCfg &= 0xFFFFFFFF;
		}

		if (pATEInfo->RxAntennaSel == 1)
		{
			TxPinCfg &= 0xFFFFF3FF;
		}
		else if (pATEInfo->RxAntennaSel == 2)
		{
			TxPinCfg &= 0xFFFFFCFF;
		}
		else
		{
			TxPinCfg &= 0xFFFFFFFF;
		}
		
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);
	}

	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1);  

	Value = (*((volatile u32 *)(RALINK_SYSCTL_BASE + 0x10)));

	if (Value & (1 << 20))
	{ 
		/* Xtal=40M */
		DBGPRINT(RT_DEBUG_TRACE, ("RT5350:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		RtmpFreqItems3020[index].N, 
		RtmpFreqItems3020[index].K, 
		RtmpFreqItems3020[index].R));
	}
	else
	{
		DBGPRINT(RT_DEBUG_TRACE, ("RT5350Xtal20M:SwitchChannel#%d(RFIC=%d, Pwr0=%d, Pwr1=%d, %dT), N=0x%02X, K=0x%02X, R=0x%02X\n",
		Channel, 
		pAd->RfIcType, 
		TxPwer,
		TxPwer2,
		pAd->Antenna.field.TxPath,
		FreqItems3020_Xtal20M[index].N, 
		FreqItems3020_Xtal20M[index].K, 
		FreqItems3020_Xtal20M[index].R));
	}
}


INT RT5350ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CHAR TxPower = 0;
	UCHAR RFValue = 0;

#ifdef RALINK_QA
	if ((pATEInfo->bQATxStart == TRUE) || (pATEInfo->bQARxStart == TRUE))
	{
		return 0;
	}
	else
#endif /* RALINK_QA */
	if (index == 0)
	{
		TxPower = pATEInfo->TxPower0;
	}
	else
	{
		DBGPRINT_ERR(("%s : Only TxPower0 is adjustable !\n", __FUNCTION__));
		DBGPRINT_ERR(("%s : TxPower%d is out of range !\n", __FUNCTION__, index));
		return -1;
	}

	RFValue = pATEInfo->TxPower0 | (1 << 7); /* bit 7 = 1 */
	RFValue &=  ~(1 << 6); /* bit 6 = 0 */
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, RFValue);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d, RFValue=%x)\n", __FUNCTION__, index, TxPower, RFValue));

	return 0;
}	


VOID RT5350ATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	/* R66 should be set according to Channel. */
	if (pATEInfo->Channel <= 14)
	{	
		/* BG band */
		if (pATEInfo->TxWI.BW == BW_20)
		{
			R66 = (UCHAR)(0x1C + (LNAGain*2));
		}
		else
		{
			R66 = (UCHAR)(0x24 + (LNAGain*2));
		}
	}
	else
	{	
		/* A band. Not supported. */
		DBGPRINT_ERR(("%s :Ch=%d\n", __FUNCTION__, pATEInfo->Channel));
		DBGPRINT_ERR(("%s :5 GHz band not supported !\n", __FUNCTION__));		

		return;
	}

	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);


	return;
}


VOID RT5350ATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, /* ExtraPwrOverTxPwrCfg8 = 0, */ExtraPwrOverTxPwrCfg9 = 0;

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

	DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


/* 
==========================================================================
    Description:
        Set RT5350 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT5350_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT powerIndex;
	UCHAR value = 0;
	UCHAR BBPCurrentBW;
	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if (BBPCurrentBW == 0)
	{
		pATEInfo->TxWI.BW = BW_20;
	}
	else
	{
		pATEInfo->TxWI.BW = BW_40;
 	}

	if ((pATEInfo->TxWI.PHYMODE == MODE_CCK) && (pATEInfo->TxWI.BW == BW_40))
	{
		DBGPRINT_ERR(("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\n"));
		DBGPRINT_ERR(("Bandwidth switch to 20!!\n"));
		pATEInfo->TxWI.BW = BW_20;
	}

	if (pATEInfo->TxWI.BW == BW_20)
	{
		if (pATEInfo->Channel <= 14)
		{
			/* BW=20;G band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
		else
		{
			/* BW=20;A band */
 			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
 			{
				if (pAd->Tx20MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx20MPwrCfgABand[powerIndex]);	
 				RtmpOsMsDelay(5);				
 			}
		}

		/* set BW = 20 MHz */
 		value = 0x40;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		value = 0; /* RF_R24 is reserved bits */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		/* Rx filter */
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);

		/* BW = 20 MHz */
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x16 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x0A;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x11 */
		value = 0x13;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);

		if (pATEInfo->Channel == 14)
		{
			INT TxMode = pATEInfo->TxWI.PHYMODE;

			if (TxMode == MODE_CCK)
			{
				/* when Channel==14 && Mode==CCK && BandWidth==20M, BBP R4 bit5=1 */
 				ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
				value |= 0x20; /* set bit5=1 */
 				ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);				
			}
		}
	}
	/* If bandwidth = 40M, set RF Reg4 bit 21 = 0. */
	else if (pATEInfo->TxWI.BW == BW_40)
	{
		if (pATEInfo->Channel <= 14)
		{
			/* BW=40;G band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgGBand[powerIndex] == 0xffffffff)
					continue;

				/* TX_PWR_CFG_0 ~ TX_PWR_CFG_4 */
				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgGBand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}
		}
		else
		{
			/* BW=40;A band */
			for (powerIndex=0; powerIndex<MAX_TXPOWER_ARRAY_SIZE; powerIndex++)
			{
				if (pAd->Tx40MPwrCfgABand[powerIndex] == 0xffffffff)
					continue;

				RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + powerIndex*4, pAd->Tx40MPwrCfgABand[powerIndex]);	
				RtmpOsMsDelay(5);				
			}		

			if ((pATEInfo->TxWI.PHYMODE >= 2) && (pATEInfo->TxWI.MCS == 7))
			{
    			value = 0x28;
    			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R67, value);
			}
		}

		/* Set BBP R4 bit[4:3]=1:0 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &value);
		value &= (~0x18);
		value |= 0x10;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, value);

		value = 0x00;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R24, (UCHAR)value);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R31, 0x80);

		/* BW = 40 MHz */
		/* Set BBP R68=0x0B to improve Rx sensitivity. */
		value = 0x0B;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R68, value);
		/* Set BBP R69=0x16 */
		value = 0x12;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R69, value);
		/* Set BBP R70=0x08 */
		value = 0x0A;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R70, value);
		/* Set BBP R73=0x11 */
		value = 0x13;
 		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R73, value);
	}

	return TRUE;
}	


INT	RT5350_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR offset = 0;
	UCHAR RFValue2 = 0;

	RFFreqOffset = simple_strtol(arg, 0, 10);

	if (RFFreqOffset >= 96)
	{
		DBGPRINT_ERR(("Set_ATE_TX_FREQ_OFFSET_Proc::Out of range(0 ~ 95).\n"));
		return FALSE;
	}

	pATEInfo->RFFreqOffset = RFFreqOffset;

	/* set RF frequency offset */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R17, (PUCHAR)&RFValue);

	RFValue2 = pATEInfo->RFFreqOffset & 0x7F; /* bit7 = 0 */

	if (RFValue2 > RFValue)
	{
		for (offset = 1; offset <= (RFValue2 - RFValue); offset++)
		{
			RtmpOsMsDelay(1);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue + offset));
		}
	}	
	else
	{
		for (offset = 1; offset <= (RFValue - RFValue2); offset++)
		{
			RtmpOsMsDelay(1);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R17, (UCHAR)(RFValue - offset));
		}
	}

	return TRUE;
}


VOID RT5350ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT			i, j, maxTxPwrCnt=5;
	CHAR		DeltaPwr = 0;
	BOOLEAN		bAutoTxAgc = FALSE;
	UCHAR		TssiRef, *pTssiMinusBoundary, *pTssiPlusBoundary, TxAgcStep;
	UCHAR		BbpR49 = 0, idx;
	PCHAR		pTxAgcCompensate;
	ULONG		TxPwr[maxTxPwrCnt];
	CHAR		Value;
#ifdef RTMP_INTERNAL_TX_ALC
	/*
		TotalDeltaPower: (non-positive number) including
		the transmit power controlled by the MAC and the BBP R1
	*/
	CHAR TotalDeltaPower = 0, TuningTableIndex = 0;     
	UINT32 desiredTSSI = 0, currentTSSI = 0, room_up = 0, room_down = 0;
	PTX_POWER_TUNING_ENTRY_STRUCT pTxPowerTuningEntry = NULL;
	UCHAR RFValue = 0;   
	BOOLEAN bKeepRF = FALSE;
	static UCHAR LastChannel = 0;
#endif /* RTMP_INTERNAL_TX_ALC */

	/* Get Tx Rate Offset Table which from eeprom 0xDEh ~ 0xEFh */
	if (pATEInfo->TxWI.BW == BW_40)
	{
		if (pATEInfo->Channel > 14)
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
		if (pATEInfo->Channel > 14)
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

	/* Get temperature compensation Delta Power Value */
#ifdef RTMP_INTERNAL_TX_ALC
	/* Locate the internal Tx ALC tuning entry */
	if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
	{
		/* In ATE mode, the period of power compensation is 1 second. */
/*		if (pATEInfo->OneSecPeriodicRound % 4 == 0) */
		{
			desiredTSSI = RT5350_ATEGetDesiredTSSI(pAd, &BbpR49);

			room_down = (desiredTSSI >> 5);	  
			room_up = (desiredTSSI >> 2);	  

			currentTSSI = BbpR49 * 10000;

			DBGPRINT(RT_DEBUG_TRACE, ("DesiredTSSI = %d, CurrentTSSI = %d (Range: %d ~ %d, BBP_R49=0x%X)\n",  
			desiredTSSI, currentTSSI, desiredTSSI-room_up,  
			desiredTSSI+room_down, BbpR49)); 

			if (LastChannel != pATEInfo->Channel)
			{
				pAd->TxPowerCtrl.idxTxPowerTable = 0;
			}

			if (desiredTSSI-room_up > currentTSSI)
			{
				pAd->TxPowerCtrl.idxTxPowerTable++;
			}
			else if (desiredTSSI+room_down < currentTSSI) 
			{
				pAd->TxPowerCtrl.idxTxPowerTable--;
			}
			else
			{
				bKeepRF = TRUE;
			}

			LastChannel = pATEInfo->Channel;

			/* To reduce the time for TSSI compensated to target value */
			TuningTableIndex = pAd->TxPowerCtrl.idxTxPowerTable
								+ pAd->TxPower[pATEInfo->Channel-1].Power;

			if (TuningTableIndex < LOWERBOUND_TX_POWER_TUNING_ENTRY)
			{
				TuningTableIndex = LOWERBOUND_TX_POWER_TUNING_ENTRY;
			} 

			if (TuningTableIndex >= UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd))
			{
				TuningTableIndex = UPPERBOUND_TX_POWER_TUNING_ENTRY(pAd);
			}

			/* Valid pAd->TxPowerCtrl.idxTxPowerTable: -30 ~ 61 */
			pTxPowerTuningEntry = &RT5350_TxPowerTuningTable[TuningTableIndex + TX_POWER_TUNING_ENTRY_OFFSET];
			pAd->TxPowerCtrl.RF_TX_ALC = pTxPowerTuningEntry->RF_TX_ALC;
			pAd->TxPowerCtrl.MAC_PowerDelta = pTxPowerTuningEntry->MAC_PowerDelta;

			if (bKeepRF == FALSE)
			{
				/* Tx power adjustment over RF is needed */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_R49, (PUCHAR)(&RFValue)); /* TX0_ALC */
				RFValue &= ~0x3F; /* clear RF_R49[5:0] */
				RFValue |= pAd->TxPowerCtrl.RF_TX_ALC;

				/* The valid range of the RF R49 (<5:0>tx0_alc<5:0>) is 0x00~0x27 */
				if ((RFValue & 0x3F) > 0x27) 
				{
					RFValue = ((RFValue & ~0x3F) | 0x27);
				}
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_R49, (UCHAR)RFValue); /* TX0_ALC */

				DBGPRINT(RT_DEBUG_TRACE, ("RF_R49 = %u\n", RFValue));			  
			}

			DBGPRINT(RT_DEBUG_TRACE, ("Pwr0 = 0x%02x\n", (pAd->TxPower[pATEInfo->Channel-1].Power)));			  
			DBGPRINT(RT_DEBUG_TRACE, ("TuningTableIndex = %d\n", TuningTableIndex));			  

			/* Tx power adjustment over MAC */
			TotalDeltaPower = pAd->TxPowerCtrl.MAC_PowerDelta;

			DBGPRINT(RT_DEBUG_TRACE, ("Current index of tuning table = %d {RF_TX_ALC = 0x%02x, MAC_PowerDelta = %d}\n",
					(TuningTableIndex +  TX_POWER_TUNING_ENTRY_OFFSET),
					pAd->TxPowerCtrl.RF_TX_ALC,
					pAd->TxPowerCtrl.MAC_PowerDelta));

			DBGPRINT(RT_DEBUG_TRACE, ("\n\n"));

		}
	}
#endif /* RTMP_INTERNAL_TX_ALC */

	/* Legacy Tx power compensation for temperature variation based on TSSI. */
	/* Do it per 4 seconds. */
	if (pATEInfo->OneSecPeriodicRound % 4 == 0)
	{
		if (pATEInfo->Channel <= 14)
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
			/* BbpR49 is unsigned char. */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);

			/* (p) TssiPlusBoundaryG[0] = 0 = (m) TssiMinusBoundaryG[0] */
			/* compensate: +4     +3   +2   +1    0   -1   -2   -3   -4 * steps */
			/* step value is defined in pAd->TxAgcStepG for tx power value */

			/* [4]+1+[4]   p4     p3   p2   p1   o1   m1   m2   m3   m4 */
			/* ex:         0x00 0x15 0x25 0x45 0x88 0xA0 0xB5 0xD0 0xF0
			   above value are examined in mass factory production */
			/*             [4]    [3]  [2]  [1]  [0]  [1]  [2]  [3]  [4] */

			/* plus is 0x10 ~ 0x40, minus is 0x60 ~ 0x90 */
			/* if value is between p1 ~ o1 or o1 ~ s1, no need to adjust tx power */
			/* if value is 0x65, tx power will be -= TxAgcStep*(2-1) */

			if (BbpR49 > pTssiMinusBoundary[1])
			{
				/* Reading is larger than the reference value. */
				/* Check for how large we need to decrease the Tx power. */
				for (idx = 1; idx < 5; idx++)
				{
					/* Found the range. */
					if (BbpR49 <= pTssiMinusBoundary[idx])  
						break;
				}

				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate. */
/*				if (R3 > (ULONG) (TxAgcStep * (idx-1))) */
					*pTxAgcCompensate = -(TxAgcStep * (idx-1));
/*				else */
/*					*pTxAgcCompensate = -((UCHAR)R3); */
				
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49 < pTssiPlusBoundary[1])
			{
				/* Reading is smaller than the reference value. */
				/* Check for how large we need to increase the Tx power. */
				for (idx = 1; idx < 5; idx++)
				{
					/* Found the range. */
					if (BbpR49 >= pTssiPlusBoundary[idx])   
						break;
				}

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate. */
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				DeltaPwr += (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("   Tx Power, BBP R1=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					BbpR49, TssiRef, TxAgcStep, 0));
			}
		}
	}
	else
	{
		if (pATEInfo->Channel <= 14)
		{
			bAutoTxAgc         = pAd->bAutoTxAgcG;
			pTxAgcCompensate   = &pAd->TxAgcCompensateG;
		}
		else
		{
			bAutoTxAgc         = pAd->bAutoTxAgcA;
			pTxAgcCompensate   = &pAd->TxAgcCompensateA;
		}

		if (bAutoTxAgc)
			DeltaPwr += (*pTxAgcCompensate);
	}

	/* Reset different new tx power for different TX rate. */
	for (i=0; i<maxTxPwrCnt; i++)
	{
		if (TxPwr[i] != 0xffffffff)
		{
			for (j=0; j<8; j++)
			{
				Value = (CHAR)((TxPwr[i] >> j*4) & 0x0F); /* 0 ~ 15 */

#ifdef RTMP_INTERNAL_TX_ALC
				/*
					The upper bounds of the MAC 0x1314~0x1324 
					are variable when the STA uses the internal Tx ALC.
				*/
				if (pAd->TxPowerCtrl.bInternalTxALC == TRUE)
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
							DBGPRINT_ERR(("%s: unknown register = 0x%X\n", 
								__FUNCTION__, 
							(TX_PWR_CFG_0 + (i * 4))));
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

				/* fill new value to CSR offset */
				TxPwr[i] = (TxPwr[i] & ~(0x0000000F << j*4)) | (Value << j*4);
			}

			/* write tx power value to CSR */
			/* TX_PWR_CFG_0 (8 tx rate) for	TX power for OFDM 12M/18M
											TX power for OFDM 6M/9M
											TX power for CCK5.5M/11M
											TX power for CCK1M/2M */
			/* TX_PWR_CFG_1 ~ TX_PWR_CFG_4 */
			RTMP_IO_WRITE32(pAd, TX_PWR_CFG_0 + (i << 2), TxPwr[i]);
		}
	}

}

struct _ATE_CHIP_STRUCT RALINK5350 =
{
	/* functions */
	.ChannelSwitch = RT5350ATEAsicSwitchChannel,
	.TxPwrHandler = RT5350ATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = RT5350_ATETssiCalibrationExtend,
	.RxVGAInit = RT5350ATERxVGAInit,
	.AsicSetTxRxPath = NULL,
	.AdjustTxPower = RT5350ATEAsicAdjustTxPower,
	.AsicExtraPowerOverMAC = RT5350ATEAsicExtraPowerOverMAC,
	
	/* command handlers */
	.Set_BW_Proc = RT5350_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT5350_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = TRUE,
	.bBBPStoreTXCARRSUPP = TRUE,	
	.bBBPStoreTXCONT = TRUE,
	.bBBPLoadATESTOP = TRUE,
};

#endif /* RT5350 */

/* End of rt5350.c */

