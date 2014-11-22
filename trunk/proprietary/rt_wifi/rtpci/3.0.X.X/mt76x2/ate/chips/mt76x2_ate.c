/****************************************************************************
 * Ralink Tech Inc.
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************


	Module Name:
	mt7662_ate.c

	Abstract:
	Specific ATE funcitons and variables for MT7662

	Revision History:
	Who         When          What
	--------    ----------    ----------------------------------------------
*/

#ifdef MT76x2


#include "rt_config.h"

VOID mt76x2_ate_set_tx_rx(
    IN PRTMP_ADAPTER ad,
    IN CHAR tx,
    IN CHAR rx);


extern RTMP_REG_PAIR mt76x2_mac_g_band_cr_table[];
extern UCHAR mt76x2_mac_g_band_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_g_band_external_pa_cr_table[];
extern UCHAR mt76x2_mac_g_band_external_pa_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_g_band_internal_pa_cr_table[];
extern UCHAR mt76x2_mac_g_band_internal_pa_cr_nums;

extern RTMP_REG_PAIR mt76x2_mac_a_band_cr_table[];
extern UCHAR mt76x2_mac_a_band_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_a_band_external_pa_cr_table[];
extern UCHAR mt76x2_mac_a_band_external_pa_cr_nums;
extern RTMP_REG_PAIR mt76x2_mac_a_band_internal_pa_cr_table[];
extern UCHAR mt76x2_mac_a_band_internal_pa_cr_nums;

#define ATE_TX_TARGET_PWR_DEFAULT_VALUE		5
#define MT76x2_TSSI_STABLE_COUNT		3

static UINT32 mt76x2_ate_calibration_delay;
static UINT32 mt76x2_ate_tssi_stable_count;
static UCHAR mt76x2_2G_tx0_pwr_offset_save = 0;
static UCHAR mt76x2_2G_tx1_pwr_offset_save = 0;
static UCHAR mt76x2_5G_tx0_pwr_offset_save = 0;
static UCHAR mt76x2_5G_tx1_pwr_offset_save = 0;

static BOOLEAN mt76x2_tx0_tssi_small_pwr_adjust = FALSE;
static BOOLEAN mt76x2_tx1_tssi_small_pwr_adjust = FALSE;

static void mt76x2_ate_switch_channel(RTMP_ADAPTER *ad)
{
	PATE_INFO pATEInfo = &(ad->ate);
	unsigned int latch_band, band, bw, tx_rx_setting;
	UINT32 ret, i, value, value1, restore_value, loop = 0;
	UCHAR bbp_ch_idx = 0;
	BOOLEAN band_change = FALSE;
	u8 channel = 0;
	UINT32 eLNA_gain_from_e2p = 0;

	SYNC_CHANNEL_WITH_QA(pATEInfo, &channel);


	/* determine channel flags */
	if (channel > 14)
		band = _A_BAND;
	else
		band = _G_BAND;
	
	if (!ad->MCUCtrl.power_on) {
		band_change = TRUE;
	} else {
		if (ad->LatchRfRegs.Channel > 14)
			latch_band = _A_BAND;
		else
			latch_band = _G_BAND;

		if (band != latch_band)
			band_change = TRUE;
		else
			band_change = FALSE;
	}
	
	if (ad->ate.TxWI.TXWI_N.BW == BW_80)
		bw = 2;
	else if (ad->ate.TxWI.TXWI_N.BW == BW_40)
		bw = 1;
	else
		bw = 0;

/*
	if ((ad->CommonCfg.TxStream == 1) && (ad->CommonCfg.RxStream == 1))
		tx_rx_setting = 0x101;
	else if ((ad->CommonCfg.TxStream == 2) && (ad->CommonCfg.RxStream == 1))
		tx_rx_setting = 0x201;
	else if ((ad->CommonCfg.TxStream == 1) && (ad->CommonCfg.RxStream == 2))
		tx_rx_setting = 0x102;
	else if ((ad->CommonCfg.TxStream == 2) && (ad->CommonCfg.RxStream == 2))
		tx_rx_setting = 0x202;
	else 
*/
		tx_rx_setting = 0x202;
		

#ifdef RTMP_PCI_SUPPORT
	/* mac setting per band */
	if (IS_PCI_INF(ad)) {
		if (band_change) {
			if (band == _G_BAND) {
				for(i = 0; i < mt76x2_mac_g_band_cr_nums; i++) {
					RTMP_IO_WRITE32(ad, mt76x2_mac_g_band_cr_table[i].Register,
									mt76x2_mac_g_band_cr_table[i].Value);
				}
				if ( ad->chipCap.PAType & INT_PA_2G ) {
					for(i = 0; i < mt76x2_mac_g_band_internal_pa_cr_nums; i++) {
						RTMP_IO_WRITE32(ad, mt76x2_mac_g_band_internal_pa_cr_table[i].Register,
										mt76x2_mac_g_band_internal_pa_cr_table[i].Value);
					}
				} else {
					for(i = 0; i < mt76x2_mac_g_band_external_pa_cr_nums; i++) {
						RTMP_IO_WRITE32(ad, mt76x2_mac_g_band_external_pa_cr_table[i].Register,
										mt76x2_mac_g_band_external_pa_cr_table[i].Value);
					}
				}
			} else {
				for(i = 0; i < mt76x2_mac_a_band_cr_nums; i++) {
					RTMP_IO_WRITE32(ad, mt76x2_mac_a_band_cr_table[i].Register,
										mt76x2_mac_a_band_cr_table[i].Value);
				}
				if ( ad->chipCap.PAType & INT_PA_5G ) {
					for(i = 0; i < mt76x2_mac_a_band_internal_pa_cr_nums; i++) {
						RTMP_IO_WRITE32(ad, mt76x2_mac_a_band_internal_pa_cr_table[i].Register,
										mt76x2_mac_a_band_internal_pa_cr_table[i].Value);
					}
				} else {
					for(i = 0; i < mt76x2_mac_a_band_external_pa_cr_nums; i++) {
						RTMP_IO_WRITE32(ad, mt76x2_mac_a_band_external_pa_cr_table[i].Register,
										mt76x2_mac_a_band_external_pa_cr_table[i].Value);
					}
				}
			}
		}
	}
#endif


        /* Fine tune tx power ramp on time based on BBP Tx delay */
        if (isExternalPAMode(ad, channel))
        {
                if (bw == 0)
                        RTMP_IO_WRITE32(ad, TX_SW_CFG0, 0x00101101);
                else
                        RTMP_IO_WRITE32(ad, TX_SW_CFG0, 0x000B0C01);

                RTMP_IO_WRITE32(ad, TX_SW_CFG1, 0x00010200);
        }
        else
        {
                if (bw == 0)
                        RTMP_IO_WRITE32(ad, TX_SW_CFG0, 0x00101001);
                else
                        RTMP_IO_WRITE32(ad, TX_SW_CFG0, 0x000B0B01);

                RTMP_IO_WRITE32(ad, TX_SW_CFG1, 0x00020000);
        }


	/* tx pwr gain setting */
	//mt76x2_tx_pwr_gain(ad, channel, bw);

	/* per-rate power delta */
	mt76x2_adjust_per_rate_pwr_delta(ad, channel, 0);

	andes_switch_channel(ad, channel, FALSE, bw, tx_rx_setting, bbp_ch_idx);

	eLNA_gain_from_e2p = ((ad->ALNAGain2 & 0xFF) << 24) | ((ad->ALNAGain1 & 0xFF) << 16) | ((ad->ALNAGain0 & 0xFF) << 8) | (ad->BLNAGain & 0xFF);
	andes_init_gain(ad, channel, TRUE, eLNA_gain_from_e2p);

	RTMP_BBP_IO_READ32(ad, AGC1_R8, &value);
	DBGPRINT(RT_DEBUG_OFF, ("%s::BBP 0x2320=0x%08x\n", __FUNCTION__, value));
	RTMP_BBP_IO_READ32(ad, AGC1_R9, &value);
	DBGPRINT(RT_DEBUG_OFF, ("%s::BBP 0x2324=0x%08x\n", __FUNCTION__, value));
	RTMP_BBP_IO_READ32(ad, AGC1_R4, &value);
	DBGPRINT(RT_DEBUG_OFF, ("%s::BBP 0x2310=0x%08x\n", __FUNCTION__, value));
	RTMP_BBP_IO_READ32(ad, AGC1_R5, &value);
	DBGPRINT(RT_DEBUG_OFF, ("%s::BBP 0x2314=0x%08x\n", __FUNCTION__, value));
	
	/* Fix dynamic agc gain in ATE Rx test not been initialized problem, and will cause AGC1_R8 register having very small value */
	mt76x2_get_agc_gain(ad, TRUE);

	if (MT_REV_GTE(ad, MT76x2, REV_MT76x2E3)) {
		/* LDPC RX */
		RTMP_BBP_IO_READ32(ad, 0x2934, &value);
		value |= (1 << 10);
		RTMP_BBP_IO_WRITE32(ad, 0x2934, value);
	}

	mt76x2_ate_set_tx_rx(ad, 0, 0);

	/* backup mac 1004 value */
	RTMP_IO_READ32(ad, 0x1004, &restore_value);
	
	/* Backup the original RTS retry count and then set to 0 */
	RTMP_IO_READ32(ad, 0x1344, &ad->rts_tx_retry_num);

	/* disable mac tx/rx */
	value = restore_value;
	value &= ~0xC;
	RTMP_IO_WRITE32(ad, 0x1004, value);

	/* set RTS retry count = 0 */	
	RTMP_IO_WRITE32(ad, TX_RTS_CFG, 0x00092B00);

	/* wait mac 0x1200, bbp 0x2130 idle */
	do {
		RTMP_IO_READ32(ad, 0x1200, &value);
		value &= 0x1;
		RTMP_BBP_IO_READ32(ad, 0x2130, &value1);
		DBGPRINT(RT_DEBUG_INFO,("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0\n", __FUNCTION__));
		RtmpusecDelay(1);
		loop++;
	} while (((value != 0) || (value1 != 0)) && (loop < 300));

	if (loop >= 300) {
		DBGPRINT(RT_DEBUG_OFF, ("%s:: Wait until MAC 0x1200 bit0 and BBP 0x2130 become 0 > 300 times\n", __FUNCTION__));
	}

	/* RXDCOC calibration */	
	CHIP_CALIBRATION(ad, RXDCOC_CALIBRATION_7662, channel);

	if (MT_REV_LT(ad, MT76x2, REV_MT76x2E3)) {
		RTMP_IO_WRITE32(ad, 0x2308, 0xFFFF);
		RTMP_IO_WRITE32(ad, 0x2cb8, 0xc);
	}

	mt76x2_calibration(ad, channel);

	if ( !(ad->chipCap.tssi_enable) || !(pATEInfo->bAutoTxAlc) ) {
		/* DPD Calibration */
		if ( (ad->chipCap.PAType== INT_PA_2G_5G) 
			|| ((ad->chipCap.PAType == INT_PA_5G) && ( ad->ate.Channel  > 14 ) )
			|| ((ad->chipCap.PAType == INT_PA_2G) && ( ad->ate.Channel  <= 14 ) )
		)
		{
			CHIP_CALIBRATION(ad, DPD_CALIBRATION_7662, channel);
		}

	}

	/* enable TX/RX */
	RTMP_IO_WRITE32(ad, 0x1004, restore_value);

	/* Restore RTS retry count */
	RTMP_IO_WRITE32(ad, TX_RTS_CFG, ad->rts_tx_retry_num);		


#ifdef RTMP_PCI_SUPPORT
	if(IS_PCI_INF(ad)) {
		NdisAcquireSpinLock(&ad->tssi_lock);
	}
#endif

	/* TSSI Clibration */
	if ( (ad->chipCap.tssi_enable) && (pATEInfo->bAutoTxAlc) ) {
		ad->chipCap.tssi_stage = TSSI_CAL_STAGE;
		if (channel > 14) {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0101);
			else if (ad->chipCap.PAType == EXT_PA_5G_ONLY)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0101);
			else
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0001); 
		} else {
			if (ad->chipCap.PAType == EXT_PA_2G_5G)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0100);
			else if ((ad->chipCap.PAType == EXT_PA_5G_ONLY) ||
					(ad->chipCap.PAType == INT_PA_2G_5G))
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0000);
			else if (ad->chipCap.PAType == EXT_PA_2G_ONLY)
				CHIP_CALIBRATION(ad, TSSI_CALIBRATION_7662, 0x0100);
			 else
				DBGPRINT(RT_DEBUG_ERROR, ("illegal PA Type(%d)\n", ad->chipCap.PAType));
		}
		ad->chipCap.tssi_stage = TSSI_TRIGGER_STAGE;
	}
	

#ifdef RTMP_PCI_SUPPORT
	if (IS_PCI_INF(ad)) {
		NdisReleaseSpinLock(&ad->tssi_lock);
	}
#endif


	/* Channel latch */
	ad->LatchRfRegs.Channel = channel;
	
	if (!ad->MCUCtrl.power_on)
		ad->MCUCtrl.power_on = TRUE;


	ATEAsicSetTxRxPath(ad);

	ATE_CHIP_RX_VGA_GAIN_INIT(ad);

#ifdef SINGLE_SKU_V2
	if(pATEInfo->bDoSingleSKU)
	{
		mt76x2_single_sku(ad,channel);
	}
#endif /* SINGLE_SKU_V2 */

	DBGPRINT(RT_DEBUG_OFF,
			("%s(): Switch to Ch#%d(%dT%dR), BBP_BW=%d\n",
			__FUNCTION__,
			channel,
			2,
			2,
			bw));
}


INT mt76x2_ate_tx_pwr_handler(
	IN RTMP_ADAPTER *ad,
	IN char index)
{
	PATE_INFO pATEInfo = &(ad->ate);
	char TxPower = 0;
	u32 value;

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

		if ( (ad->chipCap.tssi_enable) && (pATEInfo->bAutoTxAlc) ) {
			if ( pATEInfo->TxPower0 > 20 ) {
				mt76x2_tx0_tssi_small_pwr_adjust = FALSE;
			} else {
				mt76x2_tx0_tssi_small_pwr_adjust = TRUE;
			}
		}


		RTMP_IO_READ32(ad, TX_ALC_CFG_1, &value);
		if ( (ad->chipCap.tssi_enable) && (pATEInfo->bAutoTxAlc) ) {
			if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
				value &= ~(0x3F);
				if ( pATEInfo->Channel > 14 ) {
					value |= 0x38;
				} else {
					value |= 0x30;
				}
			} else {
				if ( pATEInfo->Channel > 14 ) {
					if ( mt76x2_5G_tx0_pwr_offset_save != 0 ) {
						value &= ~(0x3F);
						value |= mt76x2_5G_tx0_pwr_offset_save;
						//mt76x2_tx0_tssi_small_pwr_adjust = FALSE;	//keep true to let periodic modify target power back to user set
					} else {
						value &= ~(0x3F);
						value |= 0x38;
					}
				} else {
					if ( mt76x2_2G_tx0_pwr_offset_save != 0 ) {
						value &= ~(0x3F);
						value |= mt76x2_2G_tx0_pwr_offset_save;
						//mt76x2_tx0_tssi_small_pwr_adjust = FALSE;  //keep true to let periodic modify target power back to user set
					} else {
						value &= ~(0x3F);
						value |= 0x30;
					}
				}
			}
		} else {
			value &= ~(0x3F);
		}
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_1, value);

		if ( mt76x2_tx0_tssi_small_pwr_adjust == TRUE )
			TxPower = 30;

		/* TX0 channel initial transmission gain setting */
		RTMP_IO_READ32(ad, TX_ALC_CFG_0, &value);
		value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
		value |= TX_ALC_CFG_0_CH_INT_0(TxPower);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_0, value);
	}
	else if (index == 1)
	{
		TxPower = pATEInfo->TxPower1;
	
		if ( (ad->chipCap.tssi_enable) && (pATEInfo->bAutoTxAlc) ) {
			if ( pATEInfo->TxPower1 > 20 ) {
				mt76x2_tx1_tssi_small_pwr_adjust = FALSE;
			} else {
				TxPower = 30;
				mt76x2_tx1_tssi_small_pwr_adjust = TRUE;
			}
		}


		RTMP_IO_READ32(ad, TX_ALC_CFG_2, &value);
		if ( (ad->chipCap.tssi_enable) && (pATEInfo->bAutoTxAlc) ) {
			if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
				value &= ~(0x3F);
				if ( pATEInfo->Channel > 14 ) {
					value |= 0x38;
				} else {
					value |= 0x30;
				}
			} else {
				if ( pATEInfo->Channel > 14 ) {
					if ( mt76x2_5G_tx1_pwr_offset_save != 0 ) {
						value &= ~(0x3F);
						value |= mt76x2_5G_tx1_pwr_offset_save;
						//mt76x2_tx1_tssi_small_pwr_adjust = FALSE; //keep true to let periodic modify target power back to user set
					} else {						
						value &= ~(0x3F);
						value |= 0x38;
					}

				} else {
					if ( mt76x2_5G_tx1_pwr_offset_save != 0 ) {
						value &= ~(0x3F);
						value |= mt76x2_5G_tx1_pwr_offset_save;
						//mt76x2_tx1_tssi_small_pwr_adjust = FALSE; //keep true to let periodic modify target power back to user set
					} else {
						value &= ~(0x3F);
						value |= 0x30;
					}
				}
			}
		} else {
			value &= ~(0x3F);
		}
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_2, value);

		if ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE )
			TxPower = 30;

		/* TX1 channel initial transmission gain setting */
		RTMP_IO_READ32(ad, TX_ALC_CFG_0, &value);
		value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
		value |= TX_ALC_CFG_0_CH_INT_1(TxPower);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_0, value);
	}
	else
	{
		DBGPRINT_ERR(("%s : Only TxPower0 and TxPower1 are adjustable !\n", __FUNCTION__));
		DBGPRINT_ERR(("%s : TxPower%d is out of range !\n", __FUNCTION__, index));
		return -1;
	}

	DBGPRINT(RT_DEBUG_TRACE, ("%s : (TxPower%d=%d)\n", __FUNCTION__, index, TxPower));
	
	return 0;
}	


INT mt76x2_ate_tx_pwr_Evaluation(
	IN RTMP_ADAPTER *ad)
{
	PATE_INFO pATEInfo = &(ad->ate);
	CHAR tx_pwr_bw_delta = 0;
	CHAR tx0_pwr_ch_delta = 0, tx1_pwr_ch_delta = 0;
	CHAR tx0_target_pwr = 0, tx1_target_pwr = 0;
	CHAR tx0_pwr = 0, tx1_pwr = 0;
	UCHAR channel = pATEInfo->Channel;
	UCHAR bw = pATEInfo->TxWI.TXWI_N.BW;
	UINT16 value, value1;
	INT ret = TRUE;

	if ( channel > 14 )
	{
		/* Get BW PWR DELTA */
		if ( bw == BW_40 )
		{
			RT28xx_EEPROM_READ16(ad, G_BAND_20_40_BW_PWR_DELTA, value);
			if (((value & 0xff00) == 0x0000) || ((value & 0xff00) == 0xff00)) {
				tx_pwr_bw_delta = 0;
			} else {
				if (value & A_BAND_20_40_BW_PWR_DELTA_EN) {
					if (value & A_BAND_20_40_BW_PWR_DELTA_SIGN)
						tx_pwr_bw_delta = ((value & A_BAND_20_40_BW_PWR_DELTA_MASK) >> 8);
					else
						tx_pwr_bw_delta = -((value & A_BAND_20_40_BW_PWR_DELTA_MASK) >> 8);
				} else {
					tx_pwr_bw_delta = 0;
				}
			}
		} 
		else if ( bw == BW_80 ) 
		{
			RT28xx_EEPROM_READ16(ad, A_BAND_20_80_BW_PWR_DELTA, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
				tx_pwr_bw_delta = 0;
			} else {
				if (value & A_BAND_20_80_BW_PWR_DELTA_EN) {
					if (value & A_BAND_20_80_BW_PWR_DELTA_SIGN)
						tx_pwr_bw_delta = (value & A_BAND_20_80_BW_PWR_DELTA_MASK);
					else
						tx_pwr_bw_delta = -(value & A_BAND_20_80_BW_PWR_DELTA_MASK);
				} else {
					tx_pwr_bw_delta = 0;
				}
			}
		}

		/* Get Target PWR and PWR Offset */
		if (channel >= 184 && channel <= 196)
		{
			RT28xx_EEPROM_READ16(ad, GRP0_TX0_A_BAND_TARGET_PWR, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
				tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;	
			else
				tx0_target_pwr = (value & GRP0_TX0_A_BAND_TARGET_PWR_MASK);

			RT28xx_EEPROM_READ16(ad, GRP0_TX1_A_BAND_TARGET_PWR, value1);
			if (((value1 & 0xff) == 0x00) || ((value1 & 0xff) == 0xff))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = (value1 & GRP0_TX1_A_BAND_TARGET_PWR_MASK);

			if (channel >= 184 && channel <= 188)
			{
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx0_pwr_ch_delta = ((value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx0_pwr_ch_delta = -((value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				if (((value1 & 0xff00) == 0x00) || ((value1 & 0xff00) == 0xff00)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value1 & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value1 & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx1_pwr_ch_delta = ((value1 & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx1_pwr_ch_delta = -((value1 & GRP0_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 192 && channel <= 196)
			{
				RT28xx_EEPROM_READ16(ad, GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = (value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else
							tx0_pwr_ch_delta = -(value & GRP0_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = (value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else
							tx1_pwr_ch_delta = -(value & GRP0_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else if (channel >= 36 && channel <= 48)
		{
			RT28xx_EEPROM_READ16(ad, GRP1_TX0_A_BAND_TSSI_OFFSET, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx0_target_pwr = ((value & GRP1_TX0_A_BAND_TARGET_PWR_MASK) >> 8);

			RT28xx_EEPROM_READ16(ad, GRP1_TX1_A_BAND_TSSI_OFFSET, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = ((value & GRP1_TX1_A_BAND_TARGET_PWR_MASK) >> 8);

			if (channel >= 36 && channel <= 42)
			{
				RT28xx_EEPROM_READ16(ad, GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN) {
							tx0_pwr_ch_delta = (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						} else {
							tx0_pwr_ch_delta = -(value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						}
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN) {
							tx1_pwr_ch_delta = (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						} else {
							tx1_pwr_ch_delta = -(value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						}
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 44 && channel <= 48)
			{
				RT28xx_EEPROM_READ16(ad, GRP1_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = ((value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else
							tx0_pwr_ch_delta = -((value & GRP1_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP1_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = ((value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else
							tx1_pwr_ch_delta = -((value & GRP1_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else if (channel >= 52 && channel <= 64)
		{
			RT28xx_EEPROM_READ16(ad, GRP2_TX0_A_BAND_TARGET_PWR, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
				tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx0_target_pwr = (value & GRP2_TX0_A_BAND_TARGET_PWR_MASK);

			RT28xx_EEPROM_READ16(ad, GRP2_TX1_A_BAND_TARGET_PWR, value1);
			if (((value1 & 0xff) == 0x00) || ((value1 & 0xff) == 0xff))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = (value1 & GRP2_TX1_A_BAND_TARGET_PWR_MASK);

			if (channel >= 52 && channel <= 56)
			{
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx0_pwr_ch_delta = ((value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx0_pwr_ch_delta = -((value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				if (((value1 & 0xff00) == 0x00) || ((value1 & 0xff00) == 0xff00)) {
						tx1_pwr_ch_delta = 0;
				} else {
					if (value1 & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value1 & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx1_pwr_ch_delta = ((value1 & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx1_pwr_ch_delta = -((value1 & GRP2_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 58 && channel <= 64)
			{
				RT28xx_EEPROM_READ16(ad, GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = (value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else
							tx0_pwr_ch_delta = -(value & GRP2_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = (value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else
							tx1_pwr_ch_delta = -(value & GRP2_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else if (channel >= 98 && channel <= 114)
		{
			RT28xx_EEPROM_READ16(ad, GRP3_TX0_A_BAND_TSSI_OFFSET, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx0_target_pwr = ((value & GRP3_TX0_A_BAND_TARGET_PWR_MASK) >> 8);

			RT28xx_EEPROM_READ16(ad, GRP3_TX1_A_BAND_TSSI_OFFSET, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = ((value & GRP3_TX1_A_BAND_TARGET_PWR_MASK) >> 8);

			if (channel >= 98 && channel <= 104)
			{
				RT28xx_EEPROM_READ16(ad, GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx0_pwr_ch_delta = (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						else
							tx0_pwr_ch_delta = -(value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				} 	

				RT28xx_EEPROM_READ16(ad, GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx1_pwr_ch_delta = (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						else
							tx1_pwr_ch_delta = -(value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 106 && channel <= 114)
			{
				RT28xx_EEPROM_READ16(ad, GRP3_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = ((value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else
							tx0_pwr_ch_delta = -((value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP3_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP3_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = ((value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else
							tx1_pwr_ch_delta = -((value & GRP3_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else if (channel >= 116 && channel <= 144)
		{
			RT28xx_EEPROM_READ16(ad, GRP4_TX0_A_BAND_TARGET_PWR, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff))
				tx0_target_pwr= ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx0_target_pwr = (value & GRP4_TX0_A_BAND_TARGET_PWR_MASK);

			RT28xx_EEPROM_READ16(ad, GRP4_TX1_A_BAND_TARGET_PWR, value1);
			if (((value1 & 0xff) == 0x00) || ((value1 & 0xff) == 0xff))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = (value1 & GRP4_TX1_A_BAND_TARGET_PWR_MASK);

			if (channel >= 116 && channel <= 128)
			{
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx0_pwr_ch_delta = ((value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx0_pwr_ch_delta = -((value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				if (((value1 & 0xff00) == 0x00) || ((value1 & 0xff00) == 0xff00)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value1 & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value1 & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx1_pwr_ch_delta = ((value1 & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
						else
							tx1_pwr_ch_delta = -((value1 & GRP4_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 130 && channel <= 144)
			{
				RT28xx_EEPROM_READ16(ad, GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = (value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else	
							tx0_pwr_ch_delta = -(value & GRP4_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				} 

				RT28xx_EEPROM_READ16(ad, GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = (value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
						else	
							tx1_pwr_ch_delta = -(value & GRP4_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else if (channel >= 149 && channel <= 165)
		{
			RT28xx_EEPROM_READ16(ad, GRP5_TX0_A_BAND_TSSI_OFFSET, value);	
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx0_target_pwr = ((value & GRP5_TX0_A_BAND_TARGET_PWR_MASK) >> 8);

			RT28xx_EEPROM_READ16(ad, GRP5_TX1_A_BAND_TSSI_OFFSET, value);	
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00))
				tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE;
			else
				tx1_target_pwr = ((value & GRP5_TX1_A_BAND_TARGET_PWR_MASK) >> 8);

			if (channel >= 149 && channel <= 156)
			{
				RT28xx_EEPROM_READ16(ad, GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx0_pwr_ch_delta = (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						else
							tx0_pwr_ch_delta = -(value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW_MASK);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_EN) {
						if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_SIGN)
							tx1_pwr_ch_delta = (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
						else
							tx1_pwr_ch_delta = -(value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW_MASK);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
			else if (channel >= 157 && channel <= 165)
			{
				RT28xx_EEPROM_READ16(ad, GRP5_TX0_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx0_pwr_ch_delta = 0;
				} else {
					if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx0_pwr_ch_delta = ((value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else	
							tx0_pwr_ch_delta = -((value & GRP5_TX0_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx0_pwr_ch_delta = 0;
					}
				}

				RT28xx_EEPROM_READ16(ad, GRP5_TX1_A_BAND_CHL_PWR_DELTA_LOW, value);
				if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
					tx1_pwr_ch_delta = 0;
				} else {
					if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_EN) {
						if (value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_SIGN)
							tx1_pwr_ch_delta = ((value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
						else	
							tx1_pwr_ch_delta = -((value & GRP5_TX1_A_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					} else {
						tx1_pwr_ch_delta = 0;
					}
				}
			}
		}
		else
		{
			return FALSE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel (%d)\n", __FUNCTION__, channel));
		}
	} else {
		/* Get BW PWR DELTA */
		if ( bw == BW_40 )
		{
			RT28xx_EEPROM_READ16(ad, G_BAND_20_40_BW_PWR_DELTA, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
				tx_pwr_bw_delta = 0;
			} else {
				if (value & G_BAND_20_40_BW_PWR_DELTA_EN) {
					if (value & G_BAND_20_40_BW_PWR_DELTA_SIGN)
						tx_pwr_bw_delta = (value & G_BAND_20_40_BW_PWR_DELTA_MASK);
					else
						tx_pwr_bw_delta = -(value & G_BAND_20_40_BW_PWR_DELTA_MASK);
				} else {
					tx_pwr_bw_delta = 0;
				}
			}
		}

		/* Get Target PWR */
		RT28xx_EEPROM_READ16(ad, TX0_G_BAND_TARGET_PWR, value);
		if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
			tx0_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE; 
		} else {
			tx0_target_pwr = (value & TX0_G_BAND_TARGET_PWR_MASK);
		}

		RT28xx_EEPROM_READ16(ad, TX1_G_BAND_TARGET_PWR, value1);
		if (((value1 & 0xff) == 0x00) || ((value1 & 0xff) == 0xff))
			tx1_target_pwr = ATE_TX_TARGET_PWR_DEFAULT_VALUE; 
		else
			tx1_target_pwr = (value1 & TX1_G_BAND_TARGET_PWR_MASK);

		/* Get PWR Offset */
		if (channel >= 1 && channel <= 5)
		{
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
				tx0_pwr_ch_delta = 0;
			} else {
				if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_EN) {
					if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN)
						tx0_pwr_ch_delta = ((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					else
						tx0_pwr_ch_delta = -((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
				} else {
					tx0_pwr_ch_delta = 0;
				}
			}

			if (((value1 & 0xff00) == 0x00) || ((value1 & 0xff00) == 0xff00)) {
				tx1_pwr_ch_delta =  0;
			} else {
				if (value1 & TX1_G_BAND_CHL_PWR_DELTA_LOW_EN) {
					if (value1 & TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN)
						tx1_pwr_ch_delta = ((value1 & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
					else	
						tx1_pwr_ch_delta = -((value1 & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
				} else {
					tx1_pwr_ch_delta = 0;
				}
			}
		}
		else if (channel >= 6 && channel <= 10)
		{
			RT28xx_EEPROM_READ16(ad, TX0_G_BAND_CHL_PWR_DELTA_MID, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
				tx0_pwr_ch_delta = 0;
			} else {
				if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_EN) {
					if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN)
						tx0_pwr_ch_delta = (value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
					else
						tx0_pwr_ch_delta = -(value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
				} else {
					tx0_pwr_ch_delta = 0;
				}
			}
	
			RT28xx_EEPROM_READ16(ad, TX1_G_BAND_CHL_PWR_DELTA_MID, value);
			if (((value & 0xff) == 0x00) || ((value & 0xff) == 0xff)) {
				tx1_pwr_ch_delta = 0;
			} else {
				if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_EN) {
					if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN)
						tx1_pwr_ch_delta = (value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
					else
						tx1_pwr_ch_delta = -(value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
				} else {
					tx1_pwr_ch_delta = 0;
				}
			}
		}
		else if (channel >= 11 && channel <= 14)
		{
			RT28xx_EEPROM_READ16(ad, TX0_G_BAND_CHL_PWR_DELTA_MID, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
				tx0_pwr_ch_delta = 0;
			} else {
				if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_EN) {
					if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN)
						tx0_pwr_ch_delta = ((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					else
						tx0_pwr_ch_delta = -((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
				} else {
					tx0_pwr_ch_delta = 0;
				}
			}

			RT28xx_EEPROM_READ16(ad, TX1_G_BAND_CHL_PWR_DELTA_MID, value);
			if (((value & 0xff00) == 0x00) || ((value & 0xff00) == 0xff00)) {
				tx1_pwr_ch_delta = 0;
			} else {
				if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_EN) {
					if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN)
						tx1_pwr_ch_delta = ((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
					else
						tx1_pwr_ch_delta = -((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
				} else {
					tx1_pwr_ch_delta = 0;
				}
			}
		}
		else
		{
			return FALSE;
			DBGPRINT(RT_DEBUG_ERROR, ("%s:illegal channel(%d)\n", __FUNCTION__, channel));
		}
	}

	tx0_pwr = tx0_target_pwr + tx0_pwr_ch_delta + tx_pwr_bw_delta;
	tx1_pwr = tx1_target_pwr + tx1_pwr_ch_delta + tx_pwr_bw_delta;

	/* range 0~23.5 db */
	if (tx0_pwr >= 0x2f)
		tx0_pwr = 0x2f;

	if (tx0_pwr < 0)
		tx0_pwr = 0;

	if (tx1_pwr >= 0x2f)
		tx1_pwr = 0x2f;

	if (tx1_pwr < 0)
		tx1_pwr = 0;

	pATEInfo->TxPower0 = tx0_pwr;
	pATEInfo->TxPower1 = tx1_pwr;

	DBGPRINT(RT_DEBUG_TRACE, ("%s : tx0_target_pwr=%d, tx0_pwr_ch_delta=%d, tx_pwr_bw_delta=%d, tx0_pwr=%d\n"
		, __FUNCTION__, tx0_target_pwr, tx0_pwr_ch_delta, tx_pwr_bw_delta, tx0_pwr));

	DBGPRINT(RT_DEBUG_TRACE, ("%s : tx1_target_pwr=%d, tx1_pwr_ch_delta=%d, tx_pwr_bw_delta=%d, tx1_pwr=%d\n"
		, __FUNCTION__, tx1_target_pwr, tx1_pwr_ch_delta, tx_pwr_bw_delta, tx1_pwr));

	mt76x2_ate_tx_pwr_handler(ad, 0);
	mt76x2_ate_tx_pwr_handler(ad, 1);

	return TRUE;
}


VOID mt76x2_ate_rx_vga_init(
	IN PRTMP_ADAPTER		ad)
{
#ifdef DYNAMIC_VGA_SUPPORT
	UINT32 bbp_val;

	RTMP_BBP_IO_READ32(ad, AGC1_R8, &bbp_val);
	bbp_val = (bbp_val & 0xffff80ff) | (ad->CommonCfg.lna_vga_ctl.agc_vga_init_0 << 8);
	RTMP_BBP_IO_WRITE32(ad, AGC1_R8, bbp_val);

	//if (ad->CommonCfg.RxStream >= 2) {
	RTMP_BBP_IO_READ32(ad, AGC1_R9, &bbp_val);
	bbp_val = (bbp_val & 0xffff80ff) | (ad->CommonCfg.lna_vga_ctl.agc_vga_init_1 << 8);
	RTMP_BBP_IO_WRITE32(ad, AGC1_R9, bbp_val);
	//}
#endif /* DYNAMIC_VGA_SUPPORT */
}


VOID mt76x2_ate_set_tx_rx(
    IN PRTMP_ADAPTER ad,
    IN CHAR tx,
    IN CHAR rx)
{
	PATE_INFO   pATEInfo = &(ad->ate);
	u32 BbpValue = 0;
	u32 Value = 0 , TxPinCfg = 0;

	DBGPRINT(RT_DEBUG_TRACE, ("%s : tx=%d, rx=%d\n", __FUNCTION__, tx, rx));

	/* store the original value of TX_PIN_CFG */
	RTMP_IO_READ32(ad, TX_PIN_CFG, &Value);

	pATEInfo->Default_TX_PIN_CFG = Value;
	Value &= ~0xF;

	switch (pATEInfo->TxAntennaSel)
	{
		case 0: /* both TX0/TX1 */
			TxPinCfg = Value | 0x0F;
			break;
		case 1: /* TX0 */
			TxPinCfg = Value | 0x03;
			break;
		case 2: /* TX1 */
			TxPinCfg = Value | 0x0C;
			break;
	}
	RTMP_IO_WRITE32(ad, TX_PIN_CFG, TxPinCfg);

	/* set TX path, pAd->TxAntennaSel : 0 -> All, 1 -> TX0, 2 -> TX1 */
	switch (ad->Antenna.field.TxPath)
	{
		case 2:
			switch (tx)
			{
				case 1: /* DAC 0 */
					/* bypass MAC control DAC selection */
					RTMP_BBP_IO_READ32(ad, IBI_R9, &BbpValue);
					BbpValue &= 0xFFFFF7FF; /* 0x2124[11]=0 */
					RTMP_BBP_IO_WRITE32(ad, IBI_R9, BbpValue);

					if ( ((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTMIX) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTGREENFIELD) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_VHT) && (pATEInfo->TxWI.TXWI_N.MCS >= 16 )) )
					{
						/* 0x2714[7:0]=0 (default) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					} else {
						/* 0x2714[7:0]=0 (default) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					}

					/* 0x2080[21:20]=2'b10 */
					RTMP_BBP_IO_READ32(ad, CORE_R32, &BbpValue);
					BbpValue &= 0xFFCFFFFF;
					BbpValue |= 0x00200000;
					RTMP_BBP_IO_WRITE32(ad, CORE_R32, BbpValue);

					/* use manul mode for dac1 clock control: 
					    0x2084[11]=1,set dac1 clock enable =0 0x2084[11]=0 (default already) */
					RTMP_BBP_IO_READ32(ad, CORE_R33, &BbpValue);
					BbpValue &= 0xFFFFE1FF;
					BbpValue |= 0x800;
					RTMP_BBP_IO_WRITE32(ad, CORE_R33, BbpValue);

					break;
				case 2: /* DAC 1 */
					/* bypass MAC control DAC selection */
					RTMP_BBP_IO_READ32(ad, IBI_R9, &BbpValue);
					BbpValue &= 0xFFFFF7FF; /* 0x2124[11]=0 */
					RTMP_BBP_IO_WRITE32(ad, IBI_R9, BbpValue);

					if ( ((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTMIX) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTGREENFIELD) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_VHT) && (pATEInfo->TxWI.TXWI_N.MCS >= 16 )) )
					{
						/* 0x2714[7:0]=0 (default) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					} else {
						/* 0x2714[7:0]=81 (force direct DAC0 to DAC1) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						BbpValue |= 0x00000001;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					}

					/* 0x2080[21:20]=2'b01 */
					RTMP_BBP_IO_READ32(ad, CORE_R32, &BbpValue);
					BbpValue &= 0xFFCFFFFF;
					BbpValue |= 0x00100000;
					RTMP_BBP_IO_WRITE32(ad, CORE_R32, BbpValue);

					/* use manul mode for dac0 clock control:
					    0x2084[9]=1,set dac0 clock enable =0 0x2084[10]=0 (default already) */
					RTMP_BBP_IO_READ32(ad, CORE_R33, &BbpValue);
					BbpValue &= 0xFFFFE1FF;
					BbpValue |= 0x00000200;
					RTMP_BBP_IO_WRITE32(ad, CORE_R33, BbpValue);

					break;
				default: /* all DACs */
					/* bypass MAC control DAC selection */
					RTMP_BBP_IO_READ32(ad, IBI_R9, &BbpValue);
					BbpValue &= 0xFFFFF7FF; /* 0x2124[11]=0 */
					RTMP_BBP_IO_WRITE32(ad, IBI_R9, BbpValue);

					if ( ((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTMIX) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_HTGREENFIELD) && (pATEInfo->TxWI.TXWI_N.MCS >= 8 )) ||
						((pATEInfo->TxWI.TXWI_N.PHYMODE == MODE_VHT) && (pATEInfo->TxWI.TXWI_N.MCS >= 16 )) )
					{
						/* 0x2714[7:0]=0 (default) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					} else {
						/* 0x2714[7:0]=0 (default) */
						RTMP_BBP_IO_READ32(ad, TXBE_R5, &BbpValue);
						BbpValue &= 0xFFFFFF00;
						BbpValue |= 0x00000083;
						RTMP_BBP_IO_WRITE32(ad, TXBE_R5, BbpValue);
					}

					/* 0x2080[21:20]=0 */
					RTMP_BBP_IO_READ32(ad, CORE_R32, &BbpValue);
					BbpValue &= 0xFFCFFFFF;
					RTMP_BBP_IO_WRITE32(ad, CORE_R32, BbpValue);

					/* 0x2084[12:9]=0 */
					RTMP_BBP_IO_READ32(ad, CORE_R33, &BbpValue);
					BbpValue &= 0xFFFFE1FF;
					RTMP_BBP_IO_WRITE32(ad, CORE_R33, BbpValue);

					break;                                              
			}
			break;

		default:
			break;
	}

	switch (ad->Antenna.field.RxPath)
	{
		case 1: /* 1R */
		case 2: /* 2R */
			switch (rx)
			{
				case 1:
					// Check One Rx path, so set AGC1_OFFSET+R0 bit[4:3] = 0:0 => use 1R
					RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
					BbpValue &= 0xFFFFFFE7;				//clear bit 4:3, set bit 4:3 = 0:0
					RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);

					// Set the AGC1_OFFSET+R0 bit[1:0] = 0 :: ADC0
					RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
					BbpValue &= 0xFFFFFFFC;				//clear bit 1:0, set bit[1:0] = 0:0
					RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);
					break;
					
				case 2:
                			// Check One Rx path, so set AGC1_OFFSET+R0 bit[4:3] = 0:0 => use 1R
                			RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
            				BbpValue &= 0xFFFFFFE7;				//clear bit 4:3, set bit 4:3 = 0:0
            				RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);

                			// Set the AGC1_OFFSET+R0 bit[1:0] according to RXANT_NUM : 2R
                			RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
					BbpValue &= 0xFFFFFFFC;				//clear bit 1:0
					BbpValue |= 0x00000001;				//set bit 1:0 = 0:1
					RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);
					break;
				default:
					// Check One Rx path, so set AGC1_OFFSET+R0 bit[4:3] = 0:1 => use 2R
					RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
					BbpValue &= 0xFFFFFFE7; //clear bit 4:3
					BbpValue |= 0x00000008; //set bit 4:3 = 0:1
					RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);

					// Set the AGC1_OFFSET+R0 bit[1:0] = 3 :: All ADCs
					RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
					BbpValue &= 0xFFFFFFFC; //clear bit 1:0, set bit[1:0] = 0:0
					BbpValue |= 0x00000003; //set bit[1:0] = 3
					RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);

					if ( ad->Antenna.field.RxPath == 1 ) // 1R
					{
						// 1R AGC1_OFFSET+R0 bit[4:3:1:0]=0:0:0:0
						RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
						BbpValue &= 0xFFFFFFE4; //clear bit 4:3:1:0, set bit 4:3:1:0 = 0:0:0:0
						RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);
					}         
					else if ( ad->Antenna.field.RxPath == 2 ) // 2R
					{
						// 2R AGC1_OFFSET+R0 bit[4:3:1:0]=0:1:0:0
						RTMP_BBP_IO_READ32(ad, AGC1_R0, &BbpValue);
						BbpValue &= 0xFFFFFFE4; //clear bit 4:3:1:0
						BbpValue |= 0x00000008; //set bit 4:3:1:0 = 0:1:0:0
						RTMP_BBP_IO_WRITE32(ad, AGC1_R0, BbpValue);
					}
	            			break;
			}
			break;

		default:
		    break;              
	}

}

VOID mt76x2_ate_set_tx_rx_path(
    IN PRTMP_ADAPTER ad)
{
	PATE_INFO   pATEInfo = &(ad->ate);

	DBGPRINT(RT_DEBUG_TRACE, ("%s : Mode = %d\n", __FUNCTION__, pATEInfo->Mode));
	mt76x2_ate_set_tx_rx(ad, pATEInfo->TxAntennaSel, pATEInfo->RxAntennaSel);
}


INT	mt76x2_set_ate_tx_bw_proc(
	IN	PRTMP_ADAPTER	ad, 
	IN	PSTRING			arg)
{
	u32 core, core_r1 = 0, core_r4 = 0;
	u32 agc, agc_r0 = 0;
	u32 ret;
	u8 BBPCurrentBW;
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;

	
	BBPCurrentBW = simple_strtol(arg, 0, 10);

	if ((BBPCurrentBW == 0))
	{
		ad->ate.TxWI.TXWI_N.BW= BW_20;
	}
	else if ((BBPCurrentBW == 1))
	{
		ad->ate.TxWI.TXWI_N.BW = BW_40;
 	}
	else if ((BBPCurrentBW == 2))
	{
		ad->ate.TxWI.TXWI_N.BW = BW_80;
 	}
	else if ((BBPCurrentBW == 4))
	{
		ad->ate.TxWI.TXWI_N.BW = BW_10;
 	}
	else
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Error!! Unknow bw.\n"));
		return TRUE;
	}


	if ((ad->ate.TxWI.TXWI_N.PHYMODE == MODE_CCK) && (ad->ate.TxWI.TXWI_N.BW != BW_20))
	{
		DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! CCK only supports 20MHZ!!\nBandwidth switch to 20\n"));
		ad->ate.TxWI.TXWI_N.BW = BW_20;
	}

	if ( (ad->ate.TxWI.TXWI_N.PHYMODE == MODE_VHT) && (ad->ate.TxWI.TXWI_N.BW == BW_20) )
	{
		if ((ad->ate.TxWI.TXWI_N.MCS == 9) ||(ad->ate.TxWI.TXWI_N.MCS == 25))
		{
			DBGPRINT(RT_DEBUG_ERROR, ("Set_ATE_TX_BW_Proc!! Warning!! VHT MCS9 not support Bandwidth switch to 20\n"));
			ad->ate.TxWI.TXWI_N.BW = BW_40;
		}
	}


	RTMP_BBP_IO_READ32(ad, CORE_R1, &core_r1);
		core = (core_r1 & (~0x18));

	RTMP_BBP_IO_READ32(ad, AGC1_R0, &agc_r0);
	agc = agc_r0 & (~0x7000);

	switch (ad->ate.TxWI.TXWI_N.BW )
	{
		case BW_80:
			core |= 0x18;
			agc |= 0x7000;
			break;
		case BW_40:
			core |= 0x10;
			agc |= 0x3000;
			break;
		case BW_20:
			core &= (~0x18);
			agc |= 0x1000;
			break;
		case BW_10:
			core |= 0x08;
			agc |= 0x1000;
			break;
	}

	if (core != core_r1) 
	{
		if (IS_MT76x0(ad))
		{
			if (ad->ate.TxWI.TXWI_N.BW == BW_80)
				core |= 0x40;
			/*
				Hold BBP in reset by setting CORE_R4[0]=1
			*/
			RTMP_BBP_IO_READ32(ad, CORE_R4, &core_r4);
			core_r4 |= 0x00000001;
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, core_r4);

			/*
				Wait 0.5 us to ensure BBP is in the idle state.
			*/
			RtmpusecDelay(10);
		}
	
		RTMP_BBP_IO_WRITE32(ad, CORE_R1, core);

		if (IS_MT76x0(ad))
		{
			/*
				Wait 0.5 us for BBP clocks to settle.
			*/
			RtmpusecDelay(10);

			/*
				Release BBP from reset by clearing CORE_R4[0].
			*/
			RTMP_BBP_IO_READ32(ad, CORE_R4, &core_r4);
			core_r4 &= ~(0x00000001);
			RTMP_BBP_IO_WRITE32(ad, CORE_R4, core_r4);
		}
	}

	if (agc != agc_r0) {
		RTMP_BBP_IO_WRITE32(ad, AGC1_R0, agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, Set AGC1_R0=0x%x, agc_r0=0x%x\n", __FUNCTION__, bw, agc, agc_r0));
//		RTMP_BBP_IO_READ32(pAd, AGC1_R0, &agc);
//DBGPRINT(RT_DEBUG_OFF, ("%s(): bw=%d, After write, Get AGC1_R0=0x%x,\n", __FUNCTION__, bw, agc));
	}


	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_BW_Proc (BBPCurrentBW = %d)\n", ad->ate.TxWI.TXWI_N.BW));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_BW_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}	


#ifdef RTMP_TEMPERATURE_TX_ALC
void mt76x2_ate_temp_tx_alc(RTMP_ADAPTER *ad)
{
	RTMP_CHIP_CAP *pChipCap = &ad->chipCap;
	INT32 temp_diff = 0, dB_diff = 0, tx0_temp_comp = 0, tx1_temp_comp = 0;

	if (pChipCap->temp_tx_alc_enable) {
		mt76x2_get_current_temp(ad);
		temp_diff = pChipCap->current_temp - 25;
		
		if (ad->ate.Channel > 14) {
			if (temp_diff > 0)
				dB_diff = (temp_diff / pChipCap->high_temp_slope_a_band);
			else if (temp_diff < 0)
		 		dB_diff = 0 - ((0 - temp_diff) / pChipCap->low_temp_slope_a_band);
			else
				dB_diff = 0;
			
			/* temperature compensation boundary check and limit */
			dB_diff = (dB_diff > pChipCap->tc_upper_bound_a_band) ? pChipCap->tc_upper_bound_a_band : dB_diff;
			dB_diff = (dB_diff < pChipCap->tc_lower_bound_a_band) ? pChipCap->tc_lower_bound_a_band : dB_diff;
		} else {
			if (temp_diff > 0)
				dB_diff = (temp_diff / pChipCap->high_temp_slope_g_band);
			else if (temp_diff < 0)
		 		dB_diff = 0 - ((0 - temp_diff) / pChipCap->low_temp_slope_g_band);
			else
				dB_diff = 0;

			/* temperature compensation boundary check and limit */
			dB_diff = (dB_diff > pChipCap->tc_upper_bound_g_band) ? pChipCap->tc_upper_bound_g_band : dB_diff;
			dB_diff = (dB_diff < pChipCap->tc_lower_bound_g_band) ? pChipCap->tc_lower_bound_g_band : dB_diff;
		}

		DBGPRINT(RT_DEBUG_INFO, ("%s::temp_diff=%d (0x%x), dB_diff=%d (0x%x)\n", 
			__FUNCTION__, temp_diff, temp_diff, dB_diff, dB_diff)); 
		
		RTMP_IO_READ32(ad, TX_ALC_CFG_1, &tx0_temp_comp);
		tx0_temp_comp &= ~TX_ALC_CFG_1_TX0_TEMP_COMP_MASK;
		tx0_temp_comp |= (dB_diff*2 & TX_ALC_CFG_1_TX0_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_1, tx0_temp_comp);
		DBGPRINT(RT_DEBUG_INFO, ("%s::Tx0 power compensation = 0x%x\n", 
			__FUNCTION__, tx0_temp_comp & 0x3f)); 
		
		RTMP_IO_READ32(ad, TX_ALC_CFG_2, &tx1_temp_comp);
		tx1_temp_comp &= ~TX_ALC_CFG_2_TX1_TEMP_COMP_MASK;
		tx1_temp_comp |= (dB_diff*2 & TX_ALC_CFG_2_TX1_TEMP_COMP_MASK);
		RTMP_IO_WRITE32(ad, TX_ALC_CFG_2, tx1_temp_comp);
		DBGPRINT(RT_DEBUG_INFO, ("%s::Tx1 power compensation = 0x%x\n", 
			__FUNCTION__, tx1_temp_comp & 0x3f)); 
	}	
}
#endif /* RTMP_TEMPERATURE_TX_ALC */

VOID mt76x2_adjust_tssi_offset(
		IN PRTMP_ADAPTER pAd,
		IN UINT32 *slope_offset)
{
	CHAR OrgTSSIOffset0, OrgTSSIOffset1;
	CHAR TSSIOffsetDelta0, TSSIOffsetDelta1;
	CHAR NewTSSIOffset0, NewTSSIOffset1;
	INT32 CurrentTemperature;

	OrgTSSIOffset0 = (*slope_offset >> 16) & 0xFF;
	OrgTSSIOffset1 = (*slope_offset >> 24) & 0xFF;

	//read temperature and get TSSI offset delta in correspont temperature
	mt76x2_get_current_temp(pAd);
	CurrentTemperature = pAd->chipCap.current_temp;

	if(CurrentTemperature < -30)
	{
		TSSIOffsetDelta0 = -10;
		TSSIOffsetDelta1 = -12;
	}
	else if(CurrentTemperature <= -21)
	{
		TSSIOffsetDelta0 = -8;
		TSSIOffsetDelta1 = -10;
	}
	else if(CurrentTemperature <= -11)
	{
		TSSIOffsetDelta0 = -6;
		TSSIOffsetDelta1 = -8;
	}
	else if(CurrentTemperature <= -1)
	{
		TSSIOffsetDelta0 = -5;
		TSSIOffsetDelta1 = -6;
	}
	else if(CurrentTemperature <= 9)
	{
		TSSIOffsetDelta0 = -3;
		TSSIOffsetDelta1 = -4;
	}
	else if(CurrentTemperature <= 19)
	{
		TSSIOffsetDelta0 = -2;
		TSSIOffsetDelta1 = -2;
	}
	else if(CurrentTemperature <= 29)
	{
		TSSIOffsetDelta0 = 0;
		TSSIOffsetDelta1 = 0;
		DBGPRINT(RT_DEBUG_INFO, ("TSSIOffsetDelta = 0, not need adjust\n"));
		return;
	}
	else if(CurrentTemperature <= 39)
	{
		TSSIOffsetDelta0 = 2;
		TSSIOffsetDelta1 = 2;
	}
	else if(CurrentTemperature <= 49)
	{
		TSSIOffsetDelta0 = 3;
		TSSIOffsetDelta1 = 4;
	}
	else if(CurrentTemperature <= 59)
	{
		TSSIOffsetDelta0 = 5;
		TSSIOffsetDelta1 = 6;
	}
	else if(CurrentTemperature <= 69)
	{
		TSSIOffsetDelta0 = 6;
		TSSIOffsetDelta1 = 8;
	}
	else if(CurrentTemperature <= 79)
	{
		TSSIOffsetDelta0 = 8;
		TSSIOffsetDelta1 = 10;
	}
	else if(CurrentTemperature <= 89)
	{
		TSSIOffsetDelta0 = 10;
		TSSIOffsetDelta1 = 12;
	}
	else if(CurrentTemperature <= 99)
	{
		TSSIOffsetDelta0 = 11;
		TSSIOffsetDelta1 = 14;
	}
	else
	{
		TSSIOffsetDelta0 = 13;
		TSSIOffsetDelta1 = 16;
	}

	NewTSSIOffset0 = OrgTSSIOffset0 + TSSIOffsetDelta0;
	if( (OrgTSSIOffset0 > 0) && (TSSIOffsetDelta0 >0))
	{
		if(NewTSSIOffset0 < 0)
			NewTSSIOffset0 = 127;
	}
	else if( (OrgTSSIOffset0 < 0) && (TSSIOffsetDelta0 < 0))
	{
		if(NewTSSIOffset0 > 0)
			NewTSSIOffset0 = -128;
	}

	NewTSSIOffset1 = OrgTSSIOffset1 + TSSIOffsetDelta1;
	if( (OrgTSSIOffset1 > 0) && (TSSIOffsetDelta1 >0))
	{
		if(NewTSSIOffset1 < 0)
			NewTSSIOffset1 = 127;
	}
	else if( (OrgTSSIOffset1 < 0) && (TSSIOffsetDelta1 < 0))
	{
		if(NewTSSIOffset1 > 0)
			NewTSSIOffset1 = -128;
	}

	*slope_offset = (*slope_offset & 0x0000FFFF) | (NewTSSIOffset1<< 24) | (NewTSSIOffset0 << 16);
}


VOID mt76x2_ate_asic_adjust_tx_power(
	IN PRTMP_ADAPTER pAd) 
{
	RTMP_CHIP_CAP *cap = &pAd->chipCap;
	ANDES_CALIBRATION_PARAM param;
	UINT32 pa_mode = 0, tssi_slope_offset = 0;
	UINT32 ret = 0;
	PATE_INFO   pATEInfo = &(pAd->ate);
	UINT32 value;
	char TxPower = 0;


	if ((pAd->chipCap.tssi_enable) &&
			(RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_RADIO_OFF | 
				fRTMP_ADAPTER_DISABLE_DEQUEUEPACKET) == FALSE)) {
		//mt76x2_tssi_compensation(pAd, pAd->ate.Channel);


#ifdef RTMP_PCI_SUPPORT
		if(IS_PCI_INF(pAd)) {
			NdisAcquireSpinLock(&pAd->tssi_lock);
		}
#endif

		if (pAd->ate.Channel > 14) {
			if (pAd->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if (pAd->chipCap.PAType == EXT_PA_5G_ONLY)
				pa_mode = 1;
			else
				pa_mode = 0;
		} else {
			if (pAd->chipCap.PAType == EXT_PA_2G_5G)
				pa_mode = 1;
			else if ((pAd->chipCap.PAType == EXT_PA_5G_ONLY) ||
					(pAd->chipCap.PAType == INT_PA_2G_5G))
				pa_mode = 0;
			else if (pAd->chipCap.PAType == EXT_PA_2G_ONLY)
				pa_mode = 1;
		} 
	
		if (pAd->ate.Channel < 14) {
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_g_band);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_g_band);
		} else {
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE0(cap->tssi_0_slope_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_SLOPE1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_SLOPE1(cap->tssi_1_slope_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET0_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET0(cap->tssi_0_offset_a_band[get_chl_grp(pAd->ate.Channel )]);
			tssi_slope_offset &= ~TSSI_PARAM2_OFFSET1_MASK;
			tssi_slope_offset |= TSSI_PARAM2_OFFSET1(cap->tssi_1_offset_a_band[get_chl_grp(pAd->ate.Channel )]);

			//mt76x2_adjust_tssi_offset(pAd, &tssi_slope_offset);
		}

		param.mt76x2_tssi_comp_param.pa_mode = pa_mode;
		param.mt76x2_tssi_comp_param.tssi_slope_offset = tssi_slope_offset;

		/* TSSI Compensation */
		if(pAd->chipOps.Calibration != NULL)
			pAd->chipOps.Calibration(pAd, TSSI_COMPENSATION_7662, &param);

#ifdef RTMP_PCI_SUPPORT
		if (IS_PCI_INF(pAd)) {
			NdisReleaseSpinLock(&pAd->tssi_lock);
		}
#endif


		mt76x2_ate_calibration_delay ++;

		if ( (mt76x2_ate_calibration_delay % 10) == 0 ) {
			if ( pATEInfo->TxAntennaSel == 0 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx0_pwr_offset_save = value;
					else
						mt76x2_2G_tx0_pwr_offset_save = value;
				}

				if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx1_pwr_offset_save = value;
					else
						mt76x2_2G_tx1_pwr_offset_save = value;
				}
			} else if ( pATEInfo->TxAntennaSel == 1 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx0_pwr_offset_save = value;
					else
						mt76x2_2G_tx0_pwr_offset_save = value;
				}
			} else if ( pATEInfo->TxAntennaSel == 2 ) {
				if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
					RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
					value &= 0x3F;
					if ( pATEInfo->Channel > 14 )
						mt76x2_5G_tx1_pwr_offset_save = value;
					else
						mt76x2_2G_tx1_pwr_offset_save = value;
				}
			}
		}

		//DBGPRINT(RT_DEBUG_INFO, ("mt76x2_ate_calibration_delay %d   mt76x2_ate_tssi_stable_count %d\n", mt76x2_ate_calibration_delay,mt76x2_ate_tssi_stable_count));
		//if ( mt76x2_ate_calibration_delay == mt76x2_ate_tssi_stable_count ) {
		if ( mt76x2_ate_calibration_delay % 3 == 0 ) {
			DBGPRINT(RT_DEBUG_INFO,("mt76x2_ate_calibration_delay mod 3 == 0 mt76x2_tx0_tssi_small_pwr_adjust %d mt76x2_tx1_tssi_small_pwr_adjust %d\n",mt76x2_tx0_tssi_small_pwr_adjust,mt76x2_tx1_tssi_small_pwr_adjust));
			if ( pATEInfo->TxAntennaSel == 0 ) {

				if ( mt76x2_tx0_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower0;

					/* TX0 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
					value |= TX_ALC_CFG_0_CH_INT_0(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);
				}

				if ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower1;

					/* TX1 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
					value |= TX_ALC_CFG_0_CH_INT_1(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);
				}

				if ( (mt76x2_tx0_tssi_small_pwr_adjust == TRUE) || ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ))
				{
					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}

			} else if ( pATEInfo->TxAntennaSel == 1 ) {
				if ( mt76x2_tx0_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower0;

					/* TX0 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_0_MASK);
					value |= TX_ALC_CFG_0_CH_INT_0(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);

					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}

			} else if ( pATEInfo->TxAntennaSel == 2 ) {
				if ( mt76x2_tx1_tssi_small_pwr_adjust == TRUE ) {
					TxPower = pATEInfo->TxPower1;

					/* TX1 channel initial transmission gain setting */
					RTMP_IO_READ32(pAd, TX_ALC_CFG_0, &value);
					value = value & (~TX_ALC_CFG_0_CH_INT_1_MASK);
					value |= TX_ALC_CFG_0_CH_INT_1(TxPower);
					RTMP_IO_WRITE32(pAd, TX_ALC_CFG_0, value);

					mt76x2_ate_tssi_stable_count += MT76x2_TSSI_STABLE_COUNT;
				}
			}
		}

		if ( mt76x2_ate_calibration_delay == mt76x2_ate_tssi_stable_count ) {
			/* DPD Calibration */
			if ( (pAd->chipCap.PAType== INT_PA_2G_5G) 
				|| ((pAd->chipCap.PAType == INT_PA_5G) && ( pAd->ate.Channel  > 14 ) )
				|| ((pAd->chipCap.PAType == INT_PA_2G) && ( pAd->ate.Channel  <= 14 ) )
			)
			{

				if ( pATEInfo->TxAntennaSel == 0 ) {
					if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx0_pwr_offset_save = value;
						else
							mt76x2_2G_tx0_pwr_offset_save = value;
					}

					if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx1_pwr_offset_save = value;
						else
							mt76x2_2G_tx1_pwr_offset_save = value;
					}
				} else if ( pATEInfo->TxAntennaSel == 1 ) {
					if ( mt76x2_tx0_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_1, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx0_pwr_offset_save = value;
						else
							mt76x2_2G_tx0_pwr_offset_save = value;
					}
				} else if ( pATEInfo->TxAntennaSel == 2 ) {
					if ( mt76x2_tx1_tssi_small_pwr_adjust == FALSE ) {
						RTMP_IO_READ32(pAd, TX_ALC_CFG_2, &value);
						value &= 0x3F;
						if ( pATEInfo->Channel > 14 )
							mt76x2_5G_tx1_pwr_offset_save = value;
						else
							mt76x2_2G_tx1_pwr_offset_save = value;
					}
				}
			
				CHIP_CALIBRATION(pAd, DPD_CALIBRATION_7662, pAd->ate.Channel );
			}
		} 
	}
#ifdef RTMP_TEMPERATURE_TX_ALC
	else 
	{
		mt76x2_ate_temp_tx_alc(pAd);
	}
#endif /* RTMP_TEMPERATURE_TX_ALC */

}


INT	mt76x2_set_ate_tx_freq_offset_proc(
	IN	PRTMP_ADAPTER	ad, 
	IN	PSTRING			arg)
{
	u32 freq_offset = 0;
	u32 value = 0;
//	u32 misc_ctrl = 0;
//	u8 count = 0;

	freq_offset = simple_strtol(arg, 0, 10);
	freq_offset &= 0xFF;
	ad->ate.RFFreqOffset = freq_offset;

	/* Set crystal trim1 */
	read_reg(ad, 0x40, XO_CTRL5, &value);
	value &= 0xffff80ff;
	value |= ((freq_offset & XTAL_TRIM1_MASK) << 8);
	write_reg(ad, 0x40, XO_CTRL5, value);

	/* Enable */
	read_reg(ad, 0x40, XO_CTRL6, &value);
	value &= 0xffff80ff;
	value |= (0x7f << 8);
	write_reg(ad, 0x40, XO_CTRL6, value);

	DBGPRINT(RT_DEBUG_TRACE, ("Set_ATE_TX_FREQOFFSET_Proc (RFFreqOffset = %d)\n", ad->ate.RFFreqOffset));
	DBGPRINT(RT_DEBUG_TRACE, ("Ralink: Set_ATE_TX_FREQOFFSET_Proc Success\n"));

#ifdef CONFIG_AP_SUPPORT
#endif /* CONFIG_AP_SUPPORT */
	
	return TRUE;
}


VOID mt76x2_ate_asic_calibration(
	IN PRTMP_ADAPTER pAd, UCHAR ate_mode)
{
	
	//UCHAR channel = pAd->ate.Channel;
	UCHAR i;
	UINT32 bbpValue;
	DBGPRINT(RT_DEBUG_TRACE, ("%s: channel=%d ate_mode=0x%x\n", __FUNCTION__, pAd->ate.Channel, ate_mode));
	
	mt76x2_ate_calibration_delay = 0;
	mt76x2_ate_tssi_stable_count = MT76x2_TSSI_STABLE_COUNT;

	switch ( ate_mode )
	{
		case ATE_STOP:
			if (MT_REV_LT(pAd, MT76x2, REV_MT76x2E3)) {
				RTMP_IO_WRITE32(pAd, 0x200C , 0x0700030A);
				RTMP_IO_WRITE32(pAd, 0x2394 , 0x2121262C);
			}
			break;
		case ATE_START:
			break;
		case ATE_RXFRAME:
			if (MT_REV_LT(pAd, MT76x2, REV_MT76x2E3)) {
				RTMP_IO_WRITE32(pAd, 0x200C , 0x0600030A);
				RTMP_IO_WRITE32(pAd, 0x2394 , 0x1010161C);
			}

			if ( pAd->ate.RxAntennaSel == 2 )
			{
				for ( i = 0; i < 4 ; i++ ) {
					RTMP_BBP_IO_WRITE32(pAd, 0x2C50, i);

					RTMP_BBP_IO_READ32(pAd, 0x2c64, &bbpValue);
					//printk("0x2c64 = 0x%x\n", bbpValue);
					RTMP_BBP_IO_WRITE32(pAd, 0x2c60, bbpValue);

					RTMP_BBP_IO_READ32(pAd, 0x2c74, &bbpValue);
					//printk("0x2c74 = 0x%x\n", bbpValue);
					RTMP_BBP_IO_WRITE32(pAd, 0x2c70, bbpValue);
				}

				RTMP_BBP_IO_READ32(pAd, 0x2814, &bbpValue);
				//printk("0x2814 = 0x%x\n", bbpValue);
				RTMP_BBP_IO_WRITE32(pAd, 0x2814, 0);
			}
			break;
		case ATE_TXFRAME:
			break;
		default:
			break;
	}

}


VOID mt76x2_ate_do_calibration(
	IN PRTMP_ADAPTER pAd, UINT32 cal_id, UINT32 param)
{

	switch (cal_id )
	{
		case SX_LOGEN_CALIBRATION_7662:
			/* SX Calibration */
			CHIP_CALIBRATION(pAd, SX_LOGEN_CALIBRATION_7662, param);
		break;

		case LC_CALIBRATION_7662:
			/* LC Calibration */
			CHIP_CALIBRATION(pAd, LC_CALIBRATION_7662, param);
		break;

		case TX_LOFT_CALIBRATION_7662:
			/* TX LOFT */
			CHIP_CALIBRATION(pAd, TX_LOFT_CALIBRATION_7662, param);
			break;
		case TXIQ_CALIBRATION_7662:
			/* TXIQ Clibration */
			CHIP_CALIBRATION(pAd, TXIQ_CALIBRATION_7662, param);
			break;
		case DPD_CALIBRATION_7662:
			if (IS_PCI_INF(pAd) 
			) {
				/* DPD Calibration */
				CHIP_CALIBRATION(pAd, DPD_CALIBRATION_7662, param);
			}
			break;
		case RXIQC_FI_CALIBRATION_7662:
			/* RXIQC-FI */
			CHIP_CALIBRATION(pAd, RXIQC_FI_CALIBRATION_7662, param);
			break;

		case RXIQC_FD_CALIBRATION_7662:
			/* RXIQC-FI */
			CHIP_CALIBRATION(pAd, RXIQC_FD_CALIBRATION_7662, param);
			break;

		case RXDCOC_CALIBRATION_7662:
			/* RXDCOC calibration */	
			CHIP_CALIBRATION(pAd, RXDCOC_CALIBRATION_7662, param);
			break;
		case RC_CALIBRATION_7662:
			/* RX LPF calibration */
			CHIP_CALIBRATION(pAd, RC_CALIBRATION_7662, param);
			break;
		default:
		break;
	}

}
#ifdef SINGLE_SKU_V2
static void mt76x2_ate_single_sku(IN PRTMP_ADAPTER	pAd, IN BOOLEAN value)
{	
	PATE_INFO pATEInfo = &(pAd->ate);
	if (value > 0)
		{
			pATEInfo->bDoSingleSKU = TRUE;
			DBGPRINT(RT_DEBUG_TRACE, ("ATESINGLESKU = TRUE , enabled single sku in ATE!\n"));
		}
		else
		{
			u32 value1 = 0;
			pATEInfo->bDoSingleSKU = FALSE;
			DBGPRINT(RT_DEBUG_TRACE, ("ATESINGLESKU = FALSE , disabled single sku in ATE!\n"));
			//show_pwr_info(pAd,NULL);
			DBGPRINT(RT_DEBUG_TRACE, ("\n\n=========================== restore per rate!\n"));
			mt76x2_adjust_per_rate_pwr_delta(pAd, pAd->ate.Channel,0); //restore per_rate_delta pwr in 0x1314  

			//show_pwr_info(pAd,NULL);
			
			DBGPRINT(RT_DEBUG_TRACE, ("per_rate_delta restored!\n"));
		}	
	
}
#endif
struct _ATE_CHIP_STRUCT mt76x2ate =
{
	/* functions */
	.ChannelSwitch = mt76x2_ate_switch_channel,
	.TxPwrHandler = mt76x2_ate_tx_pwr_handler,
	.TxPwrEvaluation = mt76x2_ate_tx_pwr_Evaluation,
	.TssiCalibration = NULL,
	.ExtendedTssiCalibration = NULL /* RT5572_ATETssiCalibrationExtend */,
	.RxVGAInit = mt76x2_ate_rx_vga_init,
	.AsicSetTxRxPath = mt76x2_ate_set_tx_rx_path,
	.AdjustTxPower = mt76x2_ate_asic_adjust_tx_power,
	//.AsicExtraPowerOverMAC = DefaultATEAsicExtraPowerOverMAC,
	.Set_BW_Proc = mt76x2_set_ate_tx_bw_proc,
	.Set_FREQ_OFFSET_Proc = mt76x2_set_ate_tx_freq_offset_proc,
	.AsicCalibration = mt76x2_ate_asic_calibration,
#ifdef SINGLE_SKU_V2
	.do_ATE_single_sku = mt76x2_ate_single_sku,
#endif
	/* variables */
	.maxTxPwrCnt = 5,
	.bBBPStoreTXCARR = FALSE,
	.bBBPStoreTXCARRSUPP = FALSE,	
	.bBBPStoreTXCONT = FALSE,
	.bBBPLoadATESTOP = FALSE,/* ralink debug */
};

#endif /* MT76x2 */





