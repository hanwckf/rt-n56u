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
	rt6352_ate.c

	Abstract:
	Specific ATE funcitons and variables for RT6352

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef RT6352

#include "rt_config.h"

#ifndef RTMP_RF_RW_SUPPORT
#error "You Should Enable compile flag RTMP_RF_RW_SUPPORT for this chip"
#endif /* RTMP_RF_RW_SUPPORT */

extern REG_PAIR RT6352_RFCentralRegTable[];
extern REG_PAIR RT6352_RFChannelRegTable[];
extern REG_PAIR RT6352_RFDCCalRegTable[];
extern REG_PAIR_BW RT6352_RFDCCal_BW[];
extern REG_PAIR RT6352_BBPRegTable[];
extern REG_PAIR RT6352_BBP_GLRT[];
extern REG_PAIR_BW RT6352_BBP_GLRT_BW[];
extern RT635x_FREQUENCY_ITEM FreqItems6352[];

extern UCHAR RT6352_NUM_RF_CENTRAL_REG_PARMS;
extern UCHAR RT6352_NUM_RF_CHANNEL_REG_PARMS;
extern UCHAR RT6352_NUM_RF_DCCAL_REG_PARMS;
extern UCHAR RT6352_NUM_RF_DCCAL_BW;
extern UCHAR RT6352_NUM_BBP_REG_PARMS;
extern UCHAR RT6352_NUM_BBP_GLRT;
extern UCHAR RT6352_NUM_BBP_GLRT_BW;
extern UCHAR NUM_OF_6352_CHNL;

#define MDSM_NORMAL_TX_POWER							0x00
#define MDSM_DROP_TX_POWER_BY_6dBm						0x01
#define MDSM_DROP_TX_POWER_BY_12dBm						0x02
#define MDSM_ADD_TX_POWER_BY_6dBm						0x03
#define MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK		0x03

/* not used yet */
VOID RT635xATEFilterCalibration(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR FilterTarget = 0x00;
	UCHAR RFValue, BBPValue;
	UCHAR CalRF57_PassBand = 0x00;
	UCHAR CalRF57_StopBand = 0x00;
	UINT loop = 0, count = 0, loopcnt = 0, ReTry = 0;
	UCHAR tx_agc_fc = 0x00;

	/* Rx filter coef. offset */
	//pATEInfo->rx_agc_fc_offset20M = 0x08;
	//pATEInfo->rx_agc_fc_offset40M = 0x04;

	//pATEInfo->CaliBW20RfR24 = 0x10;
	//pATEInfo->CaliBW40RfR24 = 0x10;

	/* Enable abb_test */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
	RFValue &= ~0xC0;
	RFValue |= 0x40;
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);

	do
	{
		if (loop == 1)
		{
			/*
 			 * tx_h20M = 20MHz
 			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue |= 0x02;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);

			tx_agc_fc = 0x00;
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R32, &RFValue);
			RFValue &= ~0xF8;
			RFValue |= (tx_agc_fc << 3);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R32, RFValue);

			FilterTarget = 0x12;

			/* When calibrate BW40, BBP mask must set to BW40 */
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
			BBPValue &= ~0x18;
			BBPValue |= 0x10;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
			
			/*
 			 * rx_h20M = 20MHz
 			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue |= 0x04;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);
		}
		else
		{
			/*
 			 * tx_h20M = 10MHz
 			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~0x02;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);

			tx_agc_fc = 0x00;
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R32, &RFValue);
			RFValue &= ~0xF8;
			RFValue |= (tx_agc_fc << 3);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R32, RFValue);

			/*
				The main change is to set 20M BW target value to 0x11.
				That can provide more margin for 20M BW flatness.
			*/
			FilterTarget = 0x11;

			/*
 			 * rx_h20M = 20MHz
 			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~0x04;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);
		}

		/*
 		 * Enable BB loopback
 		 */
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R36, &RFValue);
		RFValue |= 0x02;
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R36, RFValue);

		/*
 		 * transmit tone frequency control of passband test tone
 		 */
		BBPValue = 0x02;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x00;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		/*
 		 * Enable RF calibration
 		 */
		BBPValue = 0x00;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x82;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		ReTry = 0;
	
		do
		{
			RtmpOsMsDelay(1);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
			DBGPRINT(RT_DEBUG_OFF, ("Wait RF calibration done BBP_R0 value = 0x%02x\n", BBPValue));
		} while((ReTry++ < 100 && (BBPValue & 0x80) == 0x80));

		/*
 		 * Read Rx0 signal strnegth for RF loop back RX gain
 		 */
		BBPValue = 0x39;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
		CalRF57_PassBand = BBPValue & 0xFF;

		DBGPRINT(RT_DEBUG_OFF, ("Retry = %d, CalRF57_PassBand = 0x%02x\n", ReTry, CalRF57_PassBand));
		
		/*
 		 * transmit tone frequency control of stopband test tone
 		 */
		BBPValue = 0x02;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
		BBPValue = 0x06;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

		while (TRUE)
		{
			
			/*
 			 * Enable RF calibration
 			 */
			BBPValue = 0x00;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			BBPValue = 0x82;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);

			RtmpOsMsDelay(1);
			
			/*
 		 	 * Read Rx0 signal strnegth for RF loop back RX gain
 		 	 */
			BBPValue = 0x39;
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R159, &BBPValue);
			BBPValue &= 0xFF;
			CalRF57_StopBand = BBPValue;

			DBGPRINT(RT_DEBUG_OFF, ("loopcnt = %d, CalRF57_StopBand = 0x%x,\
					tx_agc_fc = 0x%0x, CalRF57_PassBand = 0x%0x,\
					FilterTarget = 0x%0x, (CalRF57_PassBand - CalRF57_StopBand) = 0x%0x\n",
					loopcnt, CalRF57_StopBand, tx_agc_fc, CalRF57_PassBand, FilterTarget,
					(CalRF57_PassBand - CalRF57_StopBand)));

			if ((CalRF57_PassBand - CalRF57_StopBand) < FilterTarget)
			{
				tx_agc_fc++;
			}
			else if ((CalRF57_PassBand - CalRF57_StopBand) == FilterTarget)
			{
				tx_agc_fc++;
				count++;
			}
			else
			{
				loopcnt = 0;
				break;
			}

			if (loopcnt++ > 100)
			{
				DBGPRINT_ERR(("%s - can not find a valid value, loopcnt = %d\
						stop calibration\n", __FUNCTION__,loopcnt));
				break;
			}

			if (loop == 0)
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
				RFValue &= ~0x02;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);			

			}
			else
			{
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
				RFValue |= 0x02;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R32, &RFValue);
			RFValue &= ~0xF8;
			RFValue |= (tx_agc_fc << 3);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R32, RFValue);
		}

		if (count > 0)
			tx_agc_fc = tx_agc_fc - ((count) ? 1 : 0);

		/* Store for future usage */
		if (loopcnt < 100)
		{
			if (loop++ == 0)
			{
				pATEInfo->CaliBW20RfR24 = tx_agc_fc;
			}
			else
			{
				pATEInfo->CaliBW40RfR24 = tx_agc_fc;
				break;
			}	

		}
		else
			break;

		if (loop == 0)
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue &= ~0x02;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);
		}
		else
		{
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
			RFValue |= 0x02;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);
		}

		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R32, &RFValue);
		RFValue &= ~0xF8;
		RFValue |= (tx_agc_fc << 3);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R32, RFValue);

		count = 0;
	} while (TRUE);

	/*
 	 * Set back to initial state
 	 */
	BBPValue = 0x02;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R158, BBPValue);
	BBPValue = 0x00;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R159, BBPValue);
	
	/*
 	 * Disable BB loopback
 	 */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R36, &RFValue);
	RFValue &= ~0x02;
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R36, RFValue);

	/*
 	 * Set BBP back to BW20
	 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPValue);
	BBPValue &= ~0x18;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);

	/*
 	 * Disable abb_test
 	 */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R30, &RFValue);
	RFValue &= ~0xC0;
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R30, RFValue);

	DBGPRINT(RT_DEBUG_OFF, ("%s CaliBW20RfR24 = 0x%x, CaliBW40RfR24 = 0x%x\n", 
					__FUNCTION__, pATEInfo->CaliBW20RfR24, pATEInfo->CaliBW40RfR24));
}



#ifdef RTMP_INTERNAL_TX_ALC
/* results of TSSI DC calibration */
extern INT32 rt635x_tssi0_dc;
extern INT32 rt635x_tssi1_dc;
extern INT32 rt635x_tssi0_dc_hvga;
extern INT32 rt635x_tssi1_dc_hvga;
extern INT32 rt635x_tssi0_db_hvga;
extern INT32 rt635x_tssi1_db_hvga;

/* TSSI tables */
extern INT32 RT635x_McsPowerOverCCK[4];
extern INT32 RT635x_McsPowerOverOFDM[8];
extern INT32 RT635x_McsPowerOverHT[16];
extern INT32 RT635x_McsPowerOverHTSTBC[8];
extern UCHAR RT635x_RfPaModeOverCCK[4];
extern UCHAR RT635x_RfPaModeOverOFDM[8];
extern UCHAR RT635x_RfPaModeOverHT[16];
/* extern UCHAR RT635x_RfPaModeOverHTSTBC[8]; */

/* read tssi_slope and tssi_offset from eeprom/efuse */
static VOID RT635xATEGetTssiInfo(
	IN PRTMP_ADAPTER			pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	ATE_CHIP_STRUCT *pChip = (pATEInfo->pChipStruct);
	USHORT offset, value;
	CHAR temp;
	
	/* get tssi slope 0 */
	offset = 0x6E;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	pChip->tssi_slope[0] = (INT32)(value & 0x00FF);

	/* get tssi_offset 0 */
	temp = (CHAR)((value & 0xFF00)>>8);
	pChip->tssi_offset[GROUP1_2G][0] = (INT32)temp;

	offset = 0x70;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	pChip->tssi_offset[GROUP2_2G][0] = (INT32)temp;
	temp = (CHAR)((value & 0xFF00)>>8);
	pChip->tssi_offset[GROUP3_2G][0] = (INT32)temp;

	/* get tssi slope 1 */
	offset = 0x72;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	pChip->tssi_slope[1] = (INT32)(value & 0x00FF);

	/* get tssi_offset 1 */
	temp = (CHAR)((value & 0xFF00)>>8);
	pChip->tssi_offset[GROUP1_2G][1] = (INT32)temp;

	offset = 0x74;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	pChip->tssi_offset[GROUP2_2G][1] = (INT32)temp;
	temp = (CHAR)((value & 0xFF00)>>8);
	pChip->tssi_offset[GROUP3_2G][1] = (INT32)temp;

	/* get tx0/tx1 power offset */
	offset = 0x76;
	RT28xx_EEPROM_READ16(pAd, offset, value);
	temp = (CHAR)(value & 0x00FF);
	pChip->ant_pwr_offset[0] = (INT32)(temp * 1024); /* 8192/8 */
	temp = (CHAR)((value & 0xFF00)>>8);
	pChip->ant_pwr_offset[1] = (INT32)(temp * 1024); /* 8192/8 */

	return;
}


static VOID RT635xATETssiCompensation(
	IN	PRTMP_ADAPTER	pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	ATE_CHIP_STRUCT *pChip = (pATEInfo->pChipStruct);
	UINT32 MacValue;
	INT32 base_power[max_ant];
	INT32 tssi_read[max_ant];
	INT32 pa_mode_00, pa_mode_01, pa_mode_11, pa_mode_used;
	INT32 target_power = 0, mcs_power = 0, _comp_power = 0, bbp_6db_power = 0;
	INT32 tssi_dc, tssi_m_dc, tssi_db;
	INT32 pwr, pwr_diff[max_ant], pwr_diff_pre, pkt_type_delta;
	INT32 comp_power[max_ant]; 
	UCHAR RFValue, BBPValue, BBPR4, tssi_use_hvga[max_ant];
	UCHAR wait, ch_group, mcs, pa_mode, chain;
	CHAR temp[max_ant];
	CHAR BBPR49;
	INT32 cur_comp_power;

	/* TSSI compensation */
	/* 0. get base power from 0x13B0 */
	RT635xInitMcsPowerTable(pAd);
	RT635xInitRfPaModeTable(pAd);

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);
	/* bit[5:0] */
	base_power[0] = (MacValue & 0x0000003F);
	/* bit[13:8] */
	base_power[1] = (MacValue & 0x00003F00) >> 8;

	for (chain=0; chain<max_ant; chain++)
	{
		/* just care the chain that we choose */
		if ((pATEInfo->TxAntennaSel != ANT_ALL) 
			&& ((chain + 1) != (UCHAR)pATEInfo->TxAntennaSel))
		{
			continue;
		}
		
		DBGPRINT(RT_DEBUG_TRACE, ("\nchain %d: base_power is %d\n", chain, base_power[chain]));

		if (base_power[0] <= 20)
		{
			tssi_use_hvga[chain] = 1;
		}
		else
		{
			tssi_use_hvga[chain] = 0;
		}
	}

	/* 1. set TSSI mode */
	if (tssi_use_hvga[0] == 1)
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R03, RFValue);
	}
	else
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK5, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R03, RFValue);
	}

	/* for 2T2R */
	if (tssi_use_hvga[1] == 1)
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x11);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R03, RFValue);
	}
	else
	{
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK7, RF_R03, &RFValue);
		RFValue &= (~0x1F);
		RFValue |= (0x08);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R03, RFValue);
	}

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= ~((1<<6)|(1<<5));
	BBPValue |= ((1<<6)|(0<<5));
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R58, 0x00);

	/* enable TSSI for next reading */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	
	if (!(BBPValue & (1<<4)))
	{
		BBPValue |= (1<<4);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);		
	}

/*	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue); */

	/* 3. polling BBP_R47 */
	for (wait=0; wait<200; wait++)
	{
		RTMPusecDelay(100); 
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

		if (!(BBPValue & (1<<4)))
		{
			break;
		}
	}

	ASSERT(wait < 200);
	if ((BBPValue & 0x10) != 0)
		return;

	/* 4. read TSSI */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi0_linear */ 
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[0] = (INT32)BBPR49;  

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x05;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* get tssi1_linear */ 
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
	tssi_read[1] = (INT32)BBPR49;  

	/* 5. read temperature */	
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= 0x04;
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* save temperature */ 
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
/*	pChip->curr_temperature = (INT32)BBPR49; */ 
	pAd->CurrTemperature = (INT32)BBPR49;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: Current Temperature from BBP_R49=0x%x\n", __FUNCTION__, pAd->CurrTemperature));

	/* 6. estimate the target power */
	/* get packet type */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= (~0x07);
	BBPValue |= (0x01);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* read BBP_R178 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R178, &BBPValue);
	/* read BBP_R4 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R4, &BBPR4);
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	switch ((BBPR49 & 0x03))
	{
		case 0: /* CCK */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x03);
			mcs_power = RT635x_McsPowerOverCCK[mcs];
			pa_mode = RT635x_RfPaModeOverCCK[mcs];

			if (BBPValue == 0) /* BBP_R178 == 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 9831; /* 1.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
				pkt_type_delta = 3 * 8192;
				}
			}
			else /* BBP_R178 != 0 */
			{
				if (BBPR4 & (1 << 5)) /* BBP_R4[5] == 1 */
				{
					pkt_type_delta = 18023; /* 2.2 x 8192 */	
				}
				else /* BBP_R4[5] == 0 */
				{
				pkt_type_delta = 819; /* 0.1 x 8192 */	
				}
			}
			break;
		case 1: /* OFDM */
			mcs = (UCHAR)((BBPR49 >> 4) & 0x0F);
			switch (mcs)
			{
				case 0xb: mcs = 0; break;
				case 0xf: mcs = 1; break;
				case 0xa: mcs = 2; break;
				case 0xe: mcs = 3; break;
				case 0x9: mcs = 4; break;
				case 0xd: mcs = 5; break;
				case 0x8: mcs = 6; break;
				case 0xc: mcs = 7; break;
			}
			mcs_power = RT635x_McsPowerOverOFDM[mcs];
			pa_mode = RT635x_RfPaModeOverOFDM[mcs];
			pkt_type_delta = 0;	
			break;
		default: // TODO:STBC
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
			BBPValue &= (~0x07);
			BBPValue |= (0x02);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);
			ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BBPR49);
			mcs = (UCHAR)(BBPR49 & 0x7F);
			mcs_power = RT635x_McsPowerOverHT[mcs];
			pa_mode = RT635x_RfPaModeOverHT[mcs];
			pkt_type_delta = 0;	
			break;
	}

	/* 7. estimate delta power */
	/* read tssi_slope and tssi_offset from efuse */
	RT635xATEGetTssiInfo(pAd);

	ch_group = GET_2G_CHANNEL_GROUP(pATEInfo->Channel);		

	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
	cur_comp_power = MacValue & 0x3f;
	if (cur_comp_power & 0x20) 
		cur_comp_power = cur_comp_power - 0x40;
	
	for (chain=0; chain<max_ant; chain++)
	{
		pa_mode_00 = 0;
		pa_mode_01 = 4915; /* 0.6 * 8192 */
		pa_mode_11 =-6554; /*-0.8 * 8192 */
	  

	pa_mode_used = (pa_mode == 0x00) ? pa_mode_00 :
					(pa_mode == 0x01) ? pa_mode_01 :
					pa_mode_11; /* pa_mode_11 is just default */

		/* just compensate the chain that we choose */
		if ((pATEInfo->TxAntennaSel != ANT_ALL) 
			&& ((chain + 1) != (UCHAR)pATEInfo->TxAntennaSel))
		{
			continue;
		}

		DBGPRINT(RT_DEBUG_TRACE, ("\n============chain %u============\n", chain));

		if (chain == 0)
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi0_dc_hvga;
			else
				tssi_dc = rt635x_tssi0_dc;
		}
		else
		{
			if (tssi_use_hvga[chain] == 1)
				tssi_dc = rt635x_tssi1_dc_hvga;
			else
				tssi_dc = rt635x_tssi1_dc;
		}
		DBGPRINT(RT_DEBUG_TRACE, ("tssi_linear %x\n", tssi_read[chain]));
		tssi_m_dc = tssi_read[chain] - tssi_dc;
		tssi_db = lin2dBd(tssi_m_dc);

		if (tssi_use_hvga[chain])
		{
			if (chain == 0)
				tssi_db = tssi_db - rt635x_tssi0_db_hvga;
			else
				tssi_db = tssi_db - rt635x_tssi1_db_hvga;
		}

		pwr = (tssi_db*(pChip->tssi_slope[chain]))
			+ (pChip->tssi_offset[ch_group][chain] << 9);

		DBGPRINT(RT_DEBUG_TRACE, ("tssi_slope  %x\n", pChip->tssi_slope[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("tssi offset %x\n", pChip->tssi_offset[ch_group][chain]));

		/* read BBP_R1 */
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BBPValue);
		switch ((BBPValue & 0x03))
		{
			case 0: bbp_6db_power = 0; break;
			case 1: bbp_6db_power = -49152; break; /* -6dB x 8192 */
			case 2: bbp_6db_power = -98304; break; /* -12dB x 8192 */
			case 3: bbp_6db_power = 49152; break; /* 6dB x 8192 */
		}

		DBGPRINT(RT_DEBUG_TRACE, ("pa mode                       %8d\n", pa_mode));
		DBGPRINT(RT_DEBUG_TRACE, ("base power                    %8d\n", base_power[0]));
		DBGPRINT(RT_DEBUG_TRACE, ("mcs_power                     %8d\n", mcs_power));
		DBGPRINT(RT_DEBUG_TRACE, ("pa mode used                  %8d\n", pa_mode_used));
		DBGPRINT(RT_DEBUG_TRACE, ("bbp_6db_power                 %8d\n", bbp_6db_power));
		DBGPRINT(RT_DEBUG_TRACE, ("pChip->ant_pwr_offset[chain]  %8d\n", pChip->ant_pwr_offset[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("pkt_type delta                %8d\n", pkt_type_delta));
		target_power = base_power[0] + mcs_power;
		target_power = target_power * 4096;
		target_power = target_power + pa_mode_used + pkt_type_delta
			+ bbp_6db_power + pChip->ant_pwr_offset[chain];
		DBGPRINT(RT_DEBUG_TRACE, ("target_power %d, pwr %d\n", target_power, pwr));
		pwr_diff[chain] = target_power - pwr;

		if ((tssi_read[chain] > 126) && (pwr_diff[chain] > 0)) /* upper saturation */
			pwr_diff[chain] = 0;
		if (((tssi_read[chain] - tssi_dc) < 1) && (pwr_diff[chain] < 0)) /* lower saturation */
			pwr_diff[chain] = 0;

		pwr_diff_pre = pChip->pwr_diff_pre[chain];

		if (((pwr_diff_pre ^ pwr_diff[chain]) < 0)
			&& ((pwr_diff[chain] < 4096) && (pwr_diff[chain] > - 4096))
			&& ((pwr_diff_pre < 4096) && (pwr_diff_pre > - 4096)))
		{
			if ((pwr_diff[chain] > 0) && ((pwr_diff[chain] + pwr_diff_pre) >= 0))
				pwr_diff[chain] = 0;
			else if ((pwr_diff[chain] < 0) && ((pwr_diff[chain] + pwr_diff_pre) < 0))
				pwr_diff[chain] = 0;
			else 
				pwr_diff_pre = pwr_diff[chain];
		}
		else
		{
			pwr_diff_pre = pwr_diff[chain];
		}

		pChip->pwr_diff_pre[chain] = pwr_diff_pre; 

		pwr_diff[chain] = pwr_diff[chain] + ((pwr_diff[chain] > 0) ? 2048 : -2048);
		pwr_diff[chain] = pwr_diff[chain] / 4096;

		if ((pChip->tssi_slope[chain] > 0xa0) && (pChip->tssi_slope[chain] < 0x60))
			pwr_diff[chain] = 0;

		/* get previous comp_power from 0x13B4 */
		RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
		/* chain == 0 ? bit[5:0] : bit[13:8] */
		temp[chain] = MacValue & 0x0000003F;
		DBGPRINT(RT_DEBUG_TRACE, ("temp[%d] is 0x%02x\n", chain, temp[chain]));
		_comp_power = (INT32)SignedExtension6To8(temp[chain]);
		if(chain == 0)
			_comp_power = _comp_power + pwr_diff[chain];
		else if((chain + 1) == (UCHAR)pATEInfo->TxAntennaSel)
			_comp_power = _comp_power + pwr_diff[chain];
		else
			_comp_power = 
				  base_power[chain] 
				 +pwr_diff[chain]
				 -pwr_diff[0];

		DBGPRINT(RT_DEBUG_TRACE, ("pwr_diff is %d\n", pwr_diff[chain]));
		DBGPRINT(RT_DEBUG_TRACE, ("_comp_power is 0x%08x\n", _comp_power));
		if( (chain == 0) || ((chain + 1) == (UCHAR)pATEInfo->TxAntennaSel))
        {
			if (_comp_power > 31)
				_comp_power = 31;
			else if(_comp_power < -31)
				_comp_power = -31;
		}
		else
		{
			if(_comp_power < 0)
				_comp_power = 0;
			else if(_comp_power > 0x3f)
				_comp_power = 0x3f;
		}
		comp_power[chain] = _comp_power; 
	}

	/* 8. write compensation value back */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);

	if((pATEInfo->TxAntennaSel == ANT_ALL) || ((chain + 1) == (UCHAR)pATEInfo->TxAntennaSel))
		MacValue = MacValue | (comp_power[0] & 0x3f);
	else
		MacValue = MacValue | (comp_power[pATEInfo->TxAntennaSel-1] & 0x3f);

	DBGPRINT(RT_DEBUG_TRACE, ("\n================================\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);	

	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue); /* MAC 0x13B0 */
	MacValue = MacValue & (~0x3f00);
	if(pATEInfo->TxAntennaSel == ANT_ALL)
		MacValue = MacValue | ((comp_power[1] & 0x3f) << 8);
	else
		MacValue = MacValue | ((base_power[0] & 0x3f) << 8);

	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B0 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);	

	return;
}
#endif /* RTMP_INTERNAL_TX_ALC */


#ifdef RTMP_TEMPERATURE_COMPENSATION
static VOID RT635xATETemperatureCompensation(
        IN PRTMP_ADAPTER pAd) 
{
	PATE_INFO pATEInfo = &(pAd->ate);
	INT32 base_power[max_ant];
	UINT32 MacValue;
	PCHAR pTxAgcCompensate, pTssiMinusBoundary, pTssiPlusBoundary;
	CHAR TssiRef, TxAgcStep;
	CHAR delta_pwr = 0, idx, BbpR49 = 0;
	BOOLEAN bAutoTxAgc = FALSE;
	UCHAR wait, chain, BBPValue, RFValue, RFB0R2Value;

	/* temperature compensation */
	/* 1. get base power from 0x13B0 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);
	/* bit[5:0] */
	base_power[0] = (MacValue & 0x0000003F);
	/* bit[13:8] */
	base_power[1] = (MacValue & 0x00003F00) >> 8;

	for (chain=0; chain<max_ant; chain++)
	{
		/* just care the chain that we choose */
		if ((pATEInfo->TxAntennaSel != ANT_ALL) 
			&& ((chain + 1) != (UCHAR)pATEInfo->TxAntennaSel))
		{
			continue;
		}
		
		DBGPRINT(RT_DEBUG_TRACE, ("\nchain %d: base_power is %d\n", chain, base_power[chain]));
	}

	/* 2. set BBP R47[4]=1 to enable TSSI for next reading */
	/* enable RF before reading temperature sensor */
	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue);
	RFB0R2Value = RFValue; /* keep it for recovery later */
	RFValue |= 0x10;
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);

	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

	if (!(BBPValue & (1<<4)))
	{
		BBPValue |= (1<<4);
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);		
	}

	/* 3. polling BBP_R47 until BBP R47[4]==0 */
	for (wait=0; wait<200; wait++)
	{
		RTMPusecDelay(2000);
		ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);

		if (!(BBPValue & (1<<4)))
		{
			break;
		}
	}

	if ((BBPValue & 0x10) != 0)
		return;

	/* 4. set BBP R47[3:0]=4 */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R47, &BBPValue);
	BBPValue &= ~(0x0F);
	BBPValue |= (0x04);
	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R47, BBPValue);

	/* 5. fetch temperature reading */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R49, &BbpR49);
	pAd->CurrTemperature = (INT32)BbpR49;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: Current Temperature from BBP_R49=0x%x\n", __FUNCTION__, pAd->CurrTemperature));

	/* recover RF after reading temperature sensor */
	ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFB0R2Value);

	/* TX power compensation for temperature variation based on TSSI. try per 0.1 seconds */
	if (TRUE)
	{
		/* MT7620 is G band only */
		bAutoTxAgc         = pAd->bAutoTxAgcG;
		TssiRef            = pAd->TssiRefG; /* e2p[75h]: the zero reference */
		ASSERT(TssiRef == pAd->TssiCalibratedOffset);
		pTssiMinusBoundary = &pAd->TssiMinusBoundaryG[0];
		pTssiPlusBoundary  = &pAd->TssiPlusBoundaryG[0];
		/* e2p[77h]: reserved */
/*		TxAgcStep          = pAd->TxAgcStepG; */
		TxAgcStep          = 2; /* the unit of MAC 0x13B4 is 0.5 dB */
		pTxAgcCompensate   = &pAd->TxAgcCompensateG;

		/* ATE ALC is not controlled by e2p */
		if (TRUE /* bAutoTxAgc */)
		{
			if (BbpR49 < pTssiMinusBoundary[1])
			{
				/* Reading is larger than the reference value */
				/* check for how large we need to decrease the Tx power */
				for (idx = 1; idx < 8; idx++)
				{
					if (BbpR49 >= pTssiMinusBoundary[idx]) /* the range has been found */
						break;
				}

				/* The index is the step we should decrease, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = -(TxAgcStep * (idx-1));
				delta_pwr = (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("-- Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = -%d\n",
				    BbpR49, TssiRef, TxAgcStep, idx-1));                    
			}
			else if (BbpR49 > pTssiPlusBoundary[1])
			{
				/* Reading is smaller than the reference value */
				/* check for how large we need to increase the Tx power */
				for (idx = 1; idx < 8; idx++)
				{
				    if (BbpR49 <= pTssiPlusBoundary[idx]) /* the range has been found */
			            break;
				}

				/* The index is the step we should increase, idx = 0 means there is nothing to compensate */
				*pTxAgcCompensate = TxAgcStep * (idx-1);
				delta_pwr = (*pTxAgcCompensate);
				DBGPRINT(RT_DEBUG_TRACE, ("++ Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
					    BbpR49, TssiRef, TxAgcStep, idx-1));
			}
			else
			{
				*pTxAgcCompensate = 0;
				DBGPRINT(RT_DEBUG_TRACE, ("  Tx Power, BBP R49=%x, TssiRef=%x, TxAgcStep=%x, step = +%d\n",
						BbpR49, TssiRef, TxAgcStep, 0));
			}
		}
	}
	
	/* adjust compensation value by MP temperature readings(i.e., e2p[77h]) */
	delta_pwr = delta_pwr - pAd->mp_delta_pwr;
	/* delta_pwr (unit: 0.5dB) will be compensated by MAC 0x13B4 */     
	DBGPRINT(RT_DEBUG_TRACE, ("** delta_pwr =%d **\n", delta_pwr));
	DBGPRINT(RT_DEBUG_TRACE, ("%s - delta_pwr = %d, TssiCalibratedOffset = %d, TssiMpOffset = %d\n",
		__FUNCTION__, delta_pwr, pAd->TssiCalibratedOffset, pAd->mp_delta_pwr));

	/* 8-bit representation ==> 6-bit representation (2's complement) */
	if ((delta_pwr & 0x80) == 0x00) /* positive number */
		delta_pwr &= 0x3F;
	else /* 0x80: negative number */
		delta_pwr = (delta_pwr & 0x1F) | 0x20;	

	/* 6. write compensation value into TX_ALG_CFG_1 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue); /* MAC 0x13B4 */	
	MacValue = MacValue & (~0x3f);
	MacValue = MacValue | (delta_pwr & 0x3f);
	DBGPRINT(RT_DEBUG_TRACE, ("\n================================\n"));
	DBGPRINT(RT_DEBUG_TRACE, ("Mac 0x13B4 is 0x%08x\n", MacValue));
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue);

	return;
}
#endif /* RTMP_TEMPERATURE_COMPENSATION */


VOID RT635xATEAsicSwitchChannel(
	IN PRTMP_ADAPTER pAd)
{
	PATE_INFO 	pATEInfo = &(pAd->ate);
	UINT32	 	Value = 0, reg_index = 0; /* BbpReg, Value; */
	CHAR	    TxPwer = 0, TxPwer2 = DEFAULT_RF_TX_POWER; /* Bbp94 = BBPR94_DEFAULT, TxPwer2 = DEFAULT_RF_TX_POWER; */
	UCHAR		index, RFValue, Channel = 0;
	UCHAR		BBPValue;
	
	reg_index = reg_index; /* avoid compile warning */
	RFValue = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &Channel);

	/* fill Tx power value */
	TxPwer = pATEInfo->TxPower0;
	TxPwer2 = pATEInfo->TxPower1;

	DBGPRINT(RT_DEBUG_TRACE,
			(" [%s] Channel = %d\n pATEInfo->TxWI.BW = %d\n pATEInfo->RFFreqOffset = %d\n "
			"pATEInfo->TxPower0 = %d\n pATEInfo->TxPower1 = %d\n", 
			__FUNCTION__, Channel, pATEInfo->TxWI.BW, pATEInfo->RFFreqOffset,
			pATEInfo->TxPower0, pATEInfo->TxPower1));
	
	for (index = 0; index < NUM_OF_6352_CHNL; index++)
	{
		if (Channel == FreqItems6352[index].Channel)
		{
			UINT32 macStatus, saveMacSysCtrl;
			USHORT k_count = 0;

			/* Frequeny plan setting */

			/* Rdiv setting
			 * R13[1:0]
			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R13, &RFValue);
			RFValue = RFValue & (~0x03);
			RFValue |= (FreqItems6352[index].Rdiv & 0x3);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0,RF_R13, RFValue);

			/*  
 			 * N setting
 			 * R21[0], R20[7:0] 
 			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R20, &RFValue);
			RFValue = (FreqItems6352[index].N & 0x00ff);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R20, RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R21, &RFValue);
			RFValue = RFValue & (~0x01);
			RFValue |= ((FreqItems6352[index].N & 0x0100) >> 8);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R21, RFValue);

				
			/* 
			 * K setting 
			 * R16[3:0] (RF PLL freq selection)
			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R16, &RFValue);
			RFValue = RFValue & (~0x0f);
			RFValue |= (FreqItems6352[index].K & 0x0f);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R16, RFValue);

			/* 
			 * D setting 
			 * R22[2:0] (D=15, R22[2:0]=<111>)
			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R22, &RFValue);
			RFValue = RFValue & (~0x07);
			RFValue |= (FreqItems6352[index].D & 0x07);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R22, RFValue);

			/*	
			 * Ksd setting
			 * R21[0], R20[7:0] 
			 */
			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R17, &RFValue);
			RFValue = (FreqItems6352[index].Ksd & 0x000000ff);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R17, RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R18, &RFValue);
			RFValue = ((FreqItems6352[index].Ksd & 0x0000ff00) >> 8);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R18, RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R19, &RFValue);
			RFValue = RFValue & (~0x03);
			RFValue |= ((FreqItems6352[index].Ksd & 0x00030000) >> 16);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R19, RFValue);

			if (pAd->CommonCfg.Chip_VerID > 1)
			{
				/* Default: XO=20MHz , SDM mode */
				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R16, &RFValue);
				RFValue = RFValue & (~0xE0);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R16, RFValue);

				ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R21, &RFValue);
				RFValue |= 0x80;
				ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R21, RFValue);
			}

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R01, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x2);
			else
				RFValue |= 0x2;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R01, RFValue);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x20);
			else
				RFValue |= 0x20;

			if (pAd->Antenna.field.RxPath == 1)
				RFValue &= (~0x02);
			else
				RFValue |= 0x02;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue);


			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R42, &RFValue);
			if (pAd->Antenna.field.TxPath == 1)
				RFValue &= (~0x40);
			else
				RFValue |= 0x40;
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R42, RFValue);

			/* RF for DC Cal BW */
			for (reg_index = 0; reg_index < RT6352_NUM_RF_DCCAL_BW; reg_index++)
			{
				if(pATEInfo->TxWI.BW == RT6352_RFDCCal_BW[reg_index].BW)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RT6352_RFDCCal_BW[reg_index].Register, RT6352_RFDCCal_BW[reg_index].Value);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RT6352_RFDCCal_BW[reg_index].Register, RT6352_RFDCCal_BW[reg_index].Value);
				}
			}

			if (pAd->CommonCfg.Chip_VerID > 1)
			{
				if (pATEInfo->TxWI.BW == BW_20)
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R58, 0x28);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R58, 0x28);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R59, 0x28);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R59, 0x28);
				}
				else
				{
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R58, 0x08);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R58, 0x08);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK5, RF_R59, 0x08);
					ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK7, RF_R59, 0x08);
				}
			}

			{
				/* Save MAC SYS CTRL registers */
				RTMP_IO_READ32(pAd, MAC_SYS_CTRL, &saveMacSysCtrl);

				/* Disable Tx/Rx */
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, 0x00);

				/* Check MAC Tx/Rx idle */
				for (k_count = 0; k_count < 10000; k_count++)
				{
					RTMP_IO_READ32(pAd, MAC_STATUS_CFG, &macStatus);
					if (macStatus & 0x3)
						RTMPusecDelay(50);
					else
						break;
				}

				if (k_count == 10000)
				{
					DBGPRINT(RT_DEBUG_ERROR, ("Wait MAC Status to MAX  !!!\n"));
				}

				if ((pAd->CommonCfg.Chip_VerID > 1) && (pAd->CommonCfg.Chip_E_Number >= 2))
				{
					UINT8 BBPValue;

					/* ADC clcok selection */
					DBGPRINT(RT_DEBUG_ERROR, ("ADC clock selection for E3 !!!\n"));
					{
						if (Channel > 10)
						{
							DBGPRINT(RT_DEBUG_ERROR, ("Apr clock selection !!!\n"));
							RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPValue);
							BBPValue = 0x40;
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPValue);
							RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x00);
#ifdef RT6352_EL_SUPPORT
							if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0xFB);
							}
							else
#endif /* RT6352_EL_SUPPORT */
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x7B);
							}
						}
						else
						{
							DBGPRINT(RT_DEBUG_ERROR, ("Shielding clock selection !!!\n"));
							RTMP_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R30, &BBPValue);
							BBPValue = 0x1F;
							RTMP_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R30, BBPValue);
							RT635xWriteRFRegister(pAd, RF_BANK0, RF_R39, 0x80);
#ifdef RT6352_EL_SUPPORT
							if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0xDB);
							}
							else
#endif /* RT6352_EL_SUPPORT */
							{
								RT635xWriteRFRegister(pAd, RF_BANK0, RF_R42, 0x5B);
							}
						}
					}
				}

				/* Restore MAC SYS CTRL registers */
				RTMP_IO_WRITE32(pAd, MAC_SYS_CTRL, saveMacSysCtrl);
			}

			/* BandWidth Filter Calibration */
			if (pATEInfo->TxWI.BW == BW_20)
			{
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R06, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R06, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R07, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R07, RFValue);
			
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R58, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R59, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[0];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, RFValue);
			}
			else
			{
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R06, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R06, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R07, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R07, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R06, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R06, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R07, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->rx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R07, RFValue);
			
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R58, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R58, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK5, RF_R59, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK5, RF_R59, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R58, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R58, RFValue);
				RT635xReadRFRegister(pAd, RF_BANK7, RF_R59, &RFValue);
				RFValue &= (~0x3F);
				RFValue |= pAd->tx_bw_cal[1];
				RT635xWriteRFRegister(pAd, RF_BANK7, RF_R59, RFValue);
			}

			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R05, 0x40);
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R04, 0x0C);

			ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R04, &RFValue);
			RFValue = ((RFValue & ~0x80) | 0x80); /* vcocal_en (initiate VCO calibration (reset after completion)) - It should be at the end of RF configuration. */
			ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R04, RFValue);
			RtmpOsMsDelay(2);

			DBGPRINT(RT_DEBUG_TRACE, ("RT6352: SwitchChannel#%d(RF=%d, Pwr0=%d, Pwr1=%d, %dT), Rdiv=0x%02X, N=0x%02X, K=0x%02X, D=0x%02X, Ksd=0x%02X\n",
							Channel,
							pAd->RfIcType, 
							TxPwer,
							TxPwer2,
							pAd->Antenna.field.TxPath,
							FreqItems6352[index].Rdiv,
							FreqItems6352[index].N,
							FreqItems6352[index].K,
							FreqItems6352[index].D, 
							FreqItems6352[index].Ksd));

			/* latch channel for future usage */
			pAd->LatchRfRegs.Channel = Channel;
			break;
		}
	}

#ifdef RTMP_INTERNAL_TX_ALC
	if (pATEInfo->bAutoTxAlc == TRUE)
	{
		OS_SEM_LOCK(&(pATEInfo->TssiSemLock));
		RT635xTssiDcCalibration(pAd);
		OS_SEM_UNLOCK(&(pATEInfo->TssiSemLock));	
	}
#endif /* RTMP_INTERNAL_TX_ALC */

	/* DPD_Calibration &  Rx DCOC Calibration*/
	pAd->Tx0_DPD_ALC_tag0 = 0;
	pAd->Tx0_DPD_ALC_tag1 = 0;
	pAd->Tx1_DPD_ALC_tag0 = 0;
	pAd->Tx1_DPD_ALC_tag1 = 0;
	/* Rx DCOC Calibration */		
	RxDCOC_Calibration(pAd);

	/* BBP setting */
	if (pATEInfo->TxWI.BW == BW_20)
	{
		/* set BBP R4 = 0x40 for BW = 20 MHz */
		BBPValue = 0x40;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
	}
	else
	{
		/* set BBP R4 = 0x50 for BW = 40 MHz */
		BBPValue = 0x50;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R4, BBPValue);
	}

	/* BBP setting */
	if (Channel <= 14)
	{
		ULONG	TxPinCfg = 0x00150F0A; /* Gary 2007/08/09 0x050A0A */

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R62, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R63, (0x37 - GET_LNA_GAIN(pAd)));
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R64, (0x37 - GET_LNA_GAIN(pAd)));

		/* Turn off unused PA or LNA when only 1T or 1R */
		if (pAd->Antenna.field.TxPath == 1)
			TxPinCfg &= 0xFFFFFFF3;
		if (pAd->Antenna.field.RxPath == 1)
			TxPinCfg &= 0xFFFFF3FF;

		if (IS_RT6352(pAd))
			TxPinCfg |= 0x100000;

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		/* This job has been done by SetJapanFilter() */
/*		RtmpUpdateFilterCoefficientControl(pAd, Channel); */
	}

	/* BBP for GLRT BW */
	for (reg_index = 0; reg_index < RT6352_NUM_BBP_GLRT_BW; reg_index++)
	{
		if(pATEInfo->TxWI.BW == RT6352_BBP_GLRT_BW[reg_index].BW)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, RT6352_BBP_GLRT_BW[reg_index].Register);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, RT6352_BBP_GLRT_BW[reg_index].Value);
		}
	}

#ifdef RT6352_EL_SUPPORT
	if ((pAd->CommonCfg.PKG_ID == 1) && (pAd->NicConfig2.field.ExternalLNAForG))
	{
		if(pATEInfo->TxWI.BW == BW_20)
		{
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BBP_R141);
			ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, 0x15);
		}
	}
#endif /* RT6352_EL_SUPPORT */

	ATEAsicSetTxRxPath(pAd);

	/* R66 should be set according to channel and bandwidth. */
	/* AGC VGA init value */
	ATE_CHIP_RX_VGA_GAIN_INIT(pAd);

	RtmpOsMsDelay(1); 

}


INT RT635xATETxPwrHandler(
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

	if (pATEInfo->TxWI.BW == BW_40)
	{
		/* get 40 MHz power delta from e2p 50h */
		RT28xx_EEPROM_READ16(pAd, EEPROM_TXPOWER_DELTA, value);
		/* sanity check */
		if ((value & 0xFF) != 0xFF)
		{
			if ((value & 0x80))
				bw_power_delta = (value & 0x3F);

			if ((value & 0x40) == 0)
				bw_power_delta = (-1) * bw_power_delta;
		}
	}

	/* 
		bit<5:0> range from 0x0~0x27
	 */
	RTMP_IO_READ32(pAd, TX_ALG_CFG_0, &MacValue);

	if (index == 0)
	{
		TxPower = pATEInfo->TxPower0;
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
		if ((pATEInfo->TxWI.BW == BW_40) && (pATEInfo->bAutoTxAlc == TRUE))
		{
			TxPower += (bw_power_delta);
/*			TxPower = (TxPower < (0x0A << 1)) ? (0x0A << 1) : TxPower; */
		}
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */
		MacValue &= (~0x0000003F);
		MacValue |= TxPower;
	}
	else if (index == 1)
	{
		TxPower = pATEInfo->TxPower1;
#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
		if ((pATEInfo->TxWI.BW == BW_40) && (pATEInfo->bAutoTxAlc == TRUE))
		{
			TxPower += (bw_power_delta);
/*			TxPower = (TxPower < (0x0A << 1)) ? (0x0A << 1) : TxPower; */
		}
#endif /* RTMP_INTERNAL_TX_ALC || RTMP_TEMPERATURE_COMPENSATION */
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
	RTMP_IO_WRITE32(pAd, TX_ALG_CFG_0, MacValue);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d)\n", __FUNCTION__, index, TxPower));
	
	/* when TSSI is disabled, 20MHz/40MHz power delta is performed on MAC 0x13B4 */
	if (pATEInfo->bAutoTxAlc == FALSE)
	{  
		if (pATEInfo->TxWI.BW == BW_40)
		{
			CHAR diff;

			/* clean up MAC 0x13B4 bit[5:0] */
			RTMP_IO_READ32(pAd, TX_ALG_CFG_1, &MacValue);
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
			RTMP_IO_WRITE32(pAd, TX_ALG_CFG_1, MacValue); 
		}

		DBGPRINT(RT_DEBUG_TRACE, ("bw_power_delta=%d, MAC 0x13B4 is 0x%08x\n", bw_power_delta, MacValue));
	}
	
	return 0;
}	


/* for RT6352 */
VOID RT635xATERxVGAInit(
	IN PRTMP_ADAPTER		pAd)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR R66 = 0;
	CHAR LNAGain = GET_LNA_GAIN(pAd);
	
	if (pATEInfo->Channel <= 14)
	{
		R66 = 0x04 + (2 * LNAGain);
	}
	else
	{
		R66 = 0x04 + (2 * LNAGain);
	}
		
	ATEBBPWriteWithRxChain(pAd, BBP_R66, R66, RX_CHAIN_ALL);

	// TODO:
	/* RX AGC LNA MM Select threshold */
/*	ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R83, 0x7A); */


	return;
}


VOID RT635xATEAsicExtraPowerOverMAC(
	IN	PRTMP_ADAPTER 		pAd)
{
	ULONG	ExtraPwrOverMAC = 0;
	ULONG	ExtraPwrOverTxPwrCfg7 = 0, ExtraPwrOverTxPwrCfg8 = 0, ExtraPwrOverTxPwrCfg9 = 0;

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

	/*  For HT_MCS_15, extra fill the corresponding register value into MAC 0x13DC */
	RTMP_IO_READ32(pAd, 0x1320, &ExtraPwrOverMAC);  
	ExtraPwrOverTxPwrCfg8 |= (ExtraPwrOverMAC & 0x0000FF00) >> 8; /* Get Tx power for HT MCS 15 */
	RTMP_IO_WRITE32(pAd, TX_PWR_CFG_8, ExtraPwrOverTxPwrCfg8);
		
	DBGPRINT(RT_DEBUG_TRACE, ("Offset =0x13D8, TxPwr = 0x%08X, ", (UINT)ExtraPwrOverTxPwrCfg8));
	
	DBGPRINT(RT_DEBUG_TRACE, ("Offset = 0x13D4, TxPwr = 0x%08X, Offset = 0x13DC, TxPwr = 0x%08X\n", 
		(UINT)ExtraPwrOverTxPwrCfg7, 
		(UINT)ExtraPwrOverTxPwrCfg9));
}


VOID RT635xATEAsicGetTxPowerOffset(
	IN PRTMP_ADAPTER 			pAd,
	IN PULONG 					TxPwr)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	CONFIGURATION_OF_TX_POWER_CONTROL_OVER_MAC CfgOfTxPwrCtrlOverMAC;

	DBGPRINT(RT_DEBUG_INFO, ("-->%s\n", __FUNCTION__));

	NdisZeroMemory(&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	CfgOfTxPwrCtrlOverMAC.NumOfEntries = 5; /* MAC 0x1314, 0x1318, 0x131C, 0x1320 and 1324 */

	if (pATEInfo->TxWI.BW == BW_40)
	{
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx40MPwrCfgGBand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx40MPwrCfgGBand[1];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx40MPwrCfgGBand[2];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx40MPwrCfgGBand[3];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx40MPwrCfgGBand[4];
	}
	else
	{
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].MACRegisterOffset = TX_PWR_CFG_0;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[0].RegisterValue = pAd->Tx20MPwrCfgGBand[0];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].MACRegisterOffset = TX_PWR_CFG_1;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[1].RegisterValue = pAd->Tx20MPwrCfgGBand[1];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].MACRegisterOffset = TX_PWR_CFG_2;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[2].RegisterValue = pAd->Tx20MPwrCfgGBand[2];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].MACRegisterOffset = TX_PWR_CFG_3;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[3].RegisterValue = pAd->Tx20MPwrCfgGBand[3];
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].MACRegisterOffset = TX_PWR_CFG_4;
		CfgOfTxPwrCtrlOverMAC.TxPwrCtrlOverMAC[4].RegisterValue = pAd->Tx20MPwrCfgGBand[4];
	}

	NdisCopyMemory(TxPwr, (UCHAR *)&CfgOfTxPwrCtrlOverMAC, sizeof(CfgOfTxPwrCtrlOverMAC));

	DBGPRINT(RT_DEBUG_TRACE, ("<--%s\n", __FUNCTION__));
}


VOID RT635xATEAsicCompensatePowerViaBBP(
	IN 		PRTMP_ADAPTER 		pAd,
	INOUT	PCHAR				pTotalDeltaPower) 
{
	UCHAR		BbpR1 = 0;
	
	DBGPRINT(RT_DEBUG_TRACE, ("%s: <Before BBP R1> TotalDeltaPower = %d dBm\n", __FUNCTION__, *pTotalDeltaPower));

	/* The BBP R1 controls the transmit power for all rates */
	ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpR1);
	BbpR1 &= ~MDSM_BBP_R1_STATIC_TX_POWER_CONTROL_MASK;

	if (*pTotalDeltaPower <= -12)
	{
		*pTotalDeltaPower += 12;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_12dBm;

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: Drop the transmit power by 12 dBm (BBP R1)\n", __FUNCTION__));
	}
	else if ((*pTotalDeltaPower <= -6) && (*pTotalDeltaPower > -12))
	{
		*pTotalDeltaPower += 6;
		BbpR1 |= MDSM_DROP_TX_POWER_BY_6dBm;		

		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);

		DBGPRINT(RT_DEBUG_TRACE, ("%s: Drop the transmit power by 6 dBm (BBP R1)\n", __FUNCTION__));
	}
	else
	{
		/* Control the the transmit power by using the MAC only */
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpR1);
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s: <After BBP R1> TotalDeltaPower = %d dBm, BbpR1 = 0x%02X \n", __FUNCTION__, *pTotalDeltaPower, BbpR1));
}


/* 
==========================================================================
    Description:
        Set RT5572/RT5592 ATE RF BW
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT635x_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
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

	/* Turn on BBP 20MHz mode by request here. */
	if (pATEInfo->TxWI.BW == BW_20)
	{
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


	return TRUE;
}	


VOID RT635xATEAsicSetTxRxPath(
    IN PRTMP_ADAPTER pAd)
{
	PATE_INFO   pATEInfo = &(pAd->ate);
	UCHAR	RFValue1 = 0, RFValue2 = 0, BbpValue = 0;
	UINT32	TxPinCfg = 0;
	UINT32	Value = 0;
	
	if ((pATEInfo->Mode == ATE_TXCONT) || (pATEInfo->Mode == ATE_TXCARR)
		|| (pATEInfo->Mode == ATE_TXCARRSUPP))
	{
		/* store the original value of TX_PIN_CFG */
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));
		
		/*
			Bank0.R01:
				[1] for enable channel 1
				[0] for enable channel 0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R01, &RFValue1);
		RFValue1 &= (~((1 << 1) | (1 << 0))); /* clear bit 1:0 */
		
		/*
			Bank0.R02:
				[5]: enable Tx1
				[4]: enable Tx0
				[1]: enable Rx1
				[0]: enable Rx0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue2);
		RFValue2 &= (~((1 << 5) | (1 << 4) | (1 << 1) | (1 << 0))); /* clear bit 5:4:1:0 */

		BbpValue = 0x30;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, BbpValue);
		/* [4] is set as 0 to disable MAC Tx path selection */
		BbpValue = 0xF7;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, BbpValue);

		/* G band */
		switch (pATEInfo->TxAntennaSel)
		{
			case ANT_ALL: /* both TX0/TX1 */
				TxPinCfg = 0x000C00A0;
				break;
			case ANT_0:	/* TX0 */
				TxPinCfg = 0x000C0020;
				break;
			case ANT_1:	/* TX1 */
				TxPinCfg = 0x000C0080;
				break;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		/* Set TX path, pAd->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
		switch(pAd->Antenna.field.TxPath)
		{
			case 1: /* 1T */
				switch (pATEInfo->TxAntennaSel)
				{
					case ANT_0:
						/* set BBP R1, bit 4:3 = 00 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;		/* 11100111B */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 01  */					
						RFValue2 = RFValue2 | 0x10;
						break;
					case ANT_1:	
						/* set BBP R1, bit 4:3 = 01 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 10 */					
						RFValue2 = RFValue2 | 0x20;
						break;
					default: /* ANT_ALL */		
						/* set BBP R1, bit 4:3 = 10 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 11 */
						RFValue2 = RFValue2 | 0x30;
						break;					
				}
				break;

			case 2: /* 2T */
				switch (pATEInfo->TxAntennaSel)
				{
					case ANT_0:
						/* set BBP R1, bit 4:3 = 00 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;		/* 11100111B */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 01  */					
						RFValue2 = RFValue2 | 0x10;
						break;
					case ANT_1:	
						/* set BBP R1, bit 4:3 = 01 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 10 */					
						RFValue2 = RFValue2 | 0x20;
						break;
					default: /* ANT_ALL */		
						/* set BBP R1, bit 4:3 = 10 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 11 */
						RFValue2 = RFValue2 | 0x30;
					break;						
				}
				break;

			default:
				break;
		}

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R01, RFValue1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue2);
	}
	else if (pATEInfo->Mode == ATE_TXFRAME)
	{
		/* store the original value of TX_PIN_CFG */
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));
		
		/*
			Bank0.R01:
				[1] for enable channel 1
				[0] for enable channel 0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R01, &RFValue1);
		RFValue1 &= (~((1 << 1) | (1 << 0))); /* clear bit 1:0 */
		
		/*
			Bank0.R02:
				[5]: enable Tx1
				[4]: enable Tx0
				[1]: enable Rx1
				[0]: enable Rx0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue2);
		RFValue2 &= (~((1 << 5) | (1 << 4) | (1 << 1) | (1 << 0))); /* clear bit 5:4:1:0 */

		BbpValue = 0x30;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R142, BbpValue);
		/* [4] is set as 0 to disable MAC Tx path selection */
		BbpValue = 0xF7;
		ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R143, BbpValue);

		Value = (pATEInfo->Default_TX_PIN_CFG & (~0x0000000F));

		/* G band */
		switch (pATEInfo->TxAntennaSel)
		{
			case ANT_ALL: /* both TX0/TX1 */
				TxPinCfg = Value | 0x0F;
				break;
			case ANT_0:	/* TX0 */
				TxPinCfg = Value | 0x03;
				break;
			case ANT_1:	/* TX1 */
				TxPinCfg = Value | 0x0C;
				break;
		}

		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, TxPinCfg);

		/* Set TX path, pAd->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
		switch(pAd->Antenna.field.TxPath)
		{
			case 1: /* 1T */
				switch (pATEInfo->TxAntennaSel)
				{
					case ANT_0:
						/* set BBP R1, bit 4:3 = 00 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;		/* 11100111B */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 01  */					
						RFValue2 = RFValue2 | 0x10;
						break;
					case ANT_1:	
						/* set BBP R1, bit 4:3 = 01 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 10 */					
						RFValue2 = RFValue2 | 0x20;
						break;
					default: /* ANT_ALL */		
						/* set BBP R1, bit 4:3 = 10 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 11 */
						RFValue2 = RFValue2 | 0x30;
						break;						
				}
				break;

			case 2: /* 2T */
				switch (pATEInfo->TxAntennaSel)
				{
					case ANT_0:
						/* set BBP R1, bit 4:3 = 00 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;		/* 11100111B */
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 01  */					
						RFValue2 = RFValue2 | 0x10;
						break;
					case ANT_1:	
						/* set BBP R1, bit 4:3 = 01 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;	
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 10 */					
						RFValue2 = RFValue2 | 0x20;
						break;
					default: /* ANT_ALL */		
						/* set BBP R1, bit 4:3 = 10 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R1, &BbpValue);
						BbpValue &= 0xE7;
						BbpValue |= 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R1, BbpValue);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 5:4 = 11 */
						RFValue2 = RFValue2 | 0x30;
					break;						
				}
				break;

			default:
				break;
		}

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R01, RFValue1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue2);
	}
	else if (pATEInfo->Mode == ATE_RXFRAME)
	{
		/* store the original value of TX_PIN_CFG */
		RTMP_IO_READ32(pAd, TX_PIN_CFG, &(pATEInfo->Default_TX_PIN_CFG));
		
		/*
			Bank0.R01:
				[1] for enable channel 1
				[0] for enable channel 0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R01, &RFValue1);
		RFValue1 &= (~((1 << 1) | (1 << 0))); /* clear bit 1:0 */
		
		/*
			Bank0.R02:
				[5]: enable Tx1
				[4]: enable Tx0
				[1]: enable Rx1
				[0]: enable Rx0
		*/
		ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R02, &RFValue2);
		RFValue2 &= (~((1 << 5) | (1 << 4) | (1 << 1) | (1 << 0))); /* clear bit 5:4:1:0 */

		Value = (pATEInfo->Default_TX_PIN_CFG & (~0x0000000F));
		RTMP_IO_WRITE32(pAd, TX_PIN_CFG, Value);

		/* Set RX path, pAd->RxAntennaSel : 0 -> All, 1 -> RX0, 2 -> RX1, 3 -> RX2 */
		switch (pAd->Antenna.field.RxPath)
		{
			case 1:	/* 1R */					
				switch (pATEInfo->RxAntennaSel)
				{
					case ANT_0:	
						/* set BBP R3, bit 4:3:1:0 = 0000 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x00;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						BbpValue = 0x1A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);								

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x12;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xA0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000200; /* force RF RX STBY1 = 1'b1 */
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000200;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 01 */				
						RFValue2 = RFValue2 | 0x01;
						break;
					case ANT_1:								
						/* set BBP R3, bit 4:3:1:0 = 0001 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x01;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

						BbpValue = 0x1A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);	

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x12;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xA0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000100;  /* force RF RX STBY0 = 1'b1 */
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000100;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 10 */
						RFValue2 = RFValue2 | 0x02;
						break;							
					default: /* ANT_ALL */
						/* set BBP R3, bit 4:3:1:0 = 0100 */		
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						BbpValue = 0x9A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);								

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x30;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x30;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xE0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000000;
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000000;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 11 */
						RFValue2 = RFValue2 | 0x03;
						break;
				}
				break;

			case 2:	/* 2R */					
				switch (pATEInfo->RxAntennaSel)
				{
					case ANT_0:	
						/* set BBP R3, bit 4:3:1:0 = 0000 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x00;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						BbpValue = 0x1A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);								

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x12;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xA0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000200; /* force RF RX STBY1 = 1'b1 */
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000200;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 01  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 01 */				
						RFValue2 = RFValue2 | 0x01;
						break;
					case ANT_1:								
						/* set BBP R3, bit 4:3:1:0 = 0001 */
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x01;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);

						BbpValue = 0x1A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);	

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x12;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x10;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xA0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000100;  /* force RF RX STBY0 = 1'b1 */
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000100;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 10  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 10 */
						RFValue2 = RFValue2 | 0x02;
						break;							
					default: /* ANT_ALL */
						/* set BBP R3, bit 4:3:1:0 = 0100 */		
						ATE_BBP_IO_READ8_BY_REG_ID(pAd, BBP_R3, &BbpValue);
						BbpValue &= 0xE4;
						BbpValue |= 0x08;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R3, BbpValue);								

						BbpValue = 0x9A;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R95, BbpValue);								

						BbpValue = BBP_R170;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x30;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R171;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);								
						BbpValue = 0x30;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);								

						BbpValue = BBP_R128;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R195, BbpValue);
						BbpValue = 0xE0;
						ATE_BBP_IO_WRITE8_BY_REG_ID(pAd, BBP_R196, BbpValue);

						Value = 0x00000000;
						RTMP_IO_WRITE32(pAd, RF_BYPASS0, Value);
						Value = 0x00000000;
						RTMP_IO_WRITE32(pAd, RF_CONTROL0, Value);

						/* set RF Bank0 R1, bit 1:0 = 11  */					
						RFValue1 = RFValue1 | 0x03;

						/* set RF Bank0 R2, bit 1:0 = 11 */
						RFValue2 = RFValue2 | 0x03;
					break;
			}
			break;		

			default:
				break;		
		}

		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R01, RFValue1);
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R02, RFValue2);
	}
}


/* 
==========================================================================
    Description:
        Set RT5572/RT5592 ATE RF central frequency offset
        
    Return:
        TRUE if all parameters are OK, FALSE otherwise
==========================================================================
*/
INT	RT635x_Set_ATE_TX_FREQ_OFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg)
{
	PATE_INFO pATEInfo = &(pAd->ate);
	UCHAR RFFreqOffset = 0;
	UCHAR RFValue = 0;
	UCHAR PreRFValue = 0;
	
	RFFreqOffset = simple_strtol(arg, 0, 10);


	pATEInfo->RFFreqOffset = RFFreqOffset;

	ATE_RF_IO_READ8_BY_REG_ID(pAd, RF_BANK0, RF_R12, &RFValue);
	PreRFValue = RFValue;

	/* xo_code (C1 value control) - Crystal calibration */
	RFValue = ((RFValue & ~0xFF) | (pATEInfo->RFFreqOffset & 0xFF)); 
	RFValue = min((INT)RFValue, 0xFF);

	if (PreRFValue != RFValue)
	{
		ATE_RF_IO_WRITE8_BY_REG_ID(pAd, RF_BANK0, RF_R12, RFValue);
	}


	return TRUE;
}

#ifdef RTMP_MAC_PCI
struct _ATE_CHIP_STRUCT RALINK6352 =
{
	/* functions */
	.ChannelSwitch = RT635xATEAsicSwitchChannel,
	.TxPwrHandler = RT635xATETxPwrHandler,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL,
	.RxVGAInit = RT635xATERxVGAInit,
	.AsicSetTxRxPath = RT635xATEAsicSetTxRxPath,
#ifdef RTMP_INTERNAL_TX_ALC
	.AdjustTxPower = RT635xATETssiCompensation,
#endif /* RTMP_INTERNAL_TX_ALC */
#ifdef RTMP_TEMPERATURE_COMPENSATION
	.AdjustTxPower = RT635xATETemperatureCompensation,
#endif /* RTMP_TEMPERATURE_COMPENSATION */
	.AsicExtraPowerOverMAC = RT635xATEAsicExtraPowerOverMAC,

	/* command handlers */
	.Set_BW_Proc = RT635x_Set_ATE_TX_BW_Proc,
	.Set_FREQ_OFFSET_Proc = RT635x_Set_ATE_TX_FREQ_OFFSET_Proc,

	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,	
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,
};
#endif /* RTMP_MAC_PCI */
#endif /* RT6352 */

/* End of rt6352_ate.c */
