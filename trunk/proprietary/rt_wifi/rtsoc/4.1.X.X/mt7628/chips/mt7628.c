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
	mt7628.c
*/

#include "rt_config.h"
#ifdef MT7628_E1
#include "mcu/mt7628_firmware.h"
#endif
#include "mcu/mt7628_e2_firmware.h"
#include "eeprom/mt7628_e2p.h"
#ifdef LINUX
#include "phy/wf_phy_back.h"
#endif

static VOID mt7628_bbp_adjust(RTMP_ADAPTER *pAd)
{
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
	UCHAR rf_bw, ext_ch;

#ifdef DOT11_N_SUPPORT
	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch) == FALSE)
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = pAd->CommonCfg.Channel;
	}

	bbp_set_bw(pAd, rf_bw);

#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d(%d) \n",
					__FUNCTION__, ext_str[ext_ch],
					pAd->CommonCfg.HtCapability.HtCapInfo.ChannelWidth,
					pAd->CommonCfg.Channel,
					pAd->CommonCfg.RegTransmitSetting.field.EXTCHA,
					pAd->CommonCfg.AddHTInfo.AddHtInfo.ExtChanOffset));
#endif /* DOT11_N_SUPPORT */
}


static void mt7628_switch_channel(RTMP_ADAPTER *pAd, UCHAR channel, BOOLEAN scan)
{
#ifdef RTMP_FLASH_SUPPORT
	USHORT doCal1 = 0, doReload = 0;
#endif /*RTMP_FLASH_SUPPORT*/

	if (pAd->CommonCfg.BBPCurrentBW == BW_20)
		CmdChannelSwitch(pAd, channel, channel, BW_20,
								pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream, scan);								
	else
		CmdChannelSwitch(pAd, pAd->CommonCfg.Channel, channel, pAd->CommonCfg.BBPCurrentBW,
							pAd->CommonCfg.TxStream, pAd->CommonCfg.RxStream, scan);

	CmdSetTxPowerCtrl(pAd, channel);
	
	/* Channel latch */
	pAd->LatchRfRegs.Channel = channel;

#ifdef RTMP_FLASH_SUPPORT
	rtmp_ee_flash_read(pAd, 0x9F, &doCal1);	
	doReload = (doCal1 & (0x1 << 7)) >> 7;
	if (scan == FALSE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("reload DPD from flash , 0x9F = [%04x] doReload bit7[%x]\n", doCal1, doReload));
		/* reload DPD cal data from flash  , follow primary channel -by CSD */
		CmdLoadDPDDataFromFlash(pAd, pAd->CommonCfg.Channel, doReload);  
	}
#endif /* RTMP_FLASH_SUPPORT */


	MTWF_LOG(DBG_CAT_ALL, DBG_CAT_HW, DBG_LVL_TRACE,
			("%s(): Switch to Ch#%d(%dT%dR), BBP_BW=%d\n",
			__FUNCTION__,
			channel,
			pAd->CommonCfg.TxStream,
			pAd->CommonCfg.RxStream,
			pAd->CommonCfg.BBPCurrentBW));
}


static VOID mt7628_init_mac_cr(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	// enable MAC2MAC mode
#ifdef MT7628_FPGA
	UINT32 mac_val;
	RTMP_IO_READ32(pAd, RMAC_MISC, &mac_val);
	mac_val |= BIT18;
	RTMP_IO_WRITE32(pAd, RMAC_MISC, mac_val);

	mac_val = 0x00d700d7;
	RTMP_IO_WRITE32(pAd, TMAC_CDTR, mac_val);
	mac_val = 0x00d700d7;
	RTMP_IO_WRITE32(pAd, TMAC_ODTR, mac_val);
#endif /* MT7628_FPGA */

	/* TxS Setting */
	InitTxSTypeTable(pAd);
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00);
	MtAsicSetTxSClassifyFilter(pAd, TXS2MCU, TXS2M_QID0, TXS2MCU_AGGNUMS, 0x00);
}


static UINT8 mt7628_txpwr_chlist[] = {
	1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
};


int mt7628_read_chl_pwr(RTMP_ADAPTER *pAd)
{
	UINT32 i, choffset;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

	mt7628_get_tx_pwr_info(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s()--->\n", __FUNCTION__));

	for (i = 0; i < sizeof(mt7628_txpwr_chlist); i++) {
		pAd->TxPower[i].Channel = mt7628_txpwr_chlist[i];
		pAd->TxPower[i].Power = TX_TARGET_PWR_DEFAULT_VALUE;
		pAd->TxPower[i].Power2 = TX_TARGET_PWR_DEFAULT_VALUE;
	}

	for (i = 0; i < 14; i++) {
		pAd->TxPower[i].Power = cap->tx_0_target_pwr_g_band;
		pAd->TxPower[i].Power2 = cap->tx_1_target_pwr_g_band;
	}

	choffset = 14;

	/* 4. Print and Debug*/
	for (i = 0; i < choffset; i++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("E2PROM: TxPower[%03d], Channel=%d, Power[Tx0:%d, Tx1:%d]\n",
					i, pAd->TxPower[i].Channel, pAd->TxPower[i].Power, pAd->TxPower[i].Power2 ));
	}

	return TRUE;
}


/* Read power per rate */
void mt7628_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
    BOOLEAN is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* G Band tx power for CCK 1M/2M, 5.5M/11M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_CCK_1_2M, value);
    if (is_empty) {
        cap->tx_pwr_cck_1_2 = 0;
        cap->tx_pwr_cck_5_11 = 0;
    } else {
        /* CCK 1M/2M */
        if (value & TX_PWR_CCK_1_2M_EN) {
            if (value & TX_PWR_CCK_1_2M_SIGN) {
                cap->tx_pwr_cck_1_2 = (value & TX_PWR_CCK_1_2M_MASK);
            } else {
                cap->tx_pwr_cck_1_2 = -(value & TX_PWR_CCK_1_2M_MASK);
            }
        } else {
            cap->tx_pwr_cck_1_2 = 0;
        }

        /* CCK 5.5M/11M */
        if (value & TX_PWR_CCK_5_11M_EN) {
            if (value & TX_PWR_CCK_5_11M_SIGN) {
                cap->tx_pwr_cck_5_11 = ((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            } else {
                cap->tx_pwr_cck_5_11 = -((value & TX_PWR_CCK_5_11M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_cck_5_11 = 0;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));

    /* G Band tx power for OFDM 6M/9M, 12M/18M, 24M/36M, 48M/54M */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_6_9M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_6_9 = 0;
        cap->tx_pwr_g_band_ofdm_12_18 = 0;
    } else {
        /* OFDM 6M/9M */
        if (value & TX_PWR_G_BAND_OFDM_6_9M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_6_9M_SIGN) {
                cap->tx_pwr_g_band_ofdm_6_9 = (value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_6_9 = -(value & TX_PWR_G_BAND_OFDM_6_9M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_6_9 = 0;
        }

        /* OFDM 12M/18M */
        if (value & TX_PWR_G_BAND_OFDM_12_18M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_12_18M_SIGN) {
                cap->tx_pwr_g_band_ofdm_12_18 = ((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_12_18 = -((value & TX_PWR_G_BAND_OFDM_12_18M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_12_18 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_24_36M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_24_36 = 0;
        cap->tx_pwr_g_band_ofdm_48= 0;
    } else {
        /* OFDM 24M/36M */
        if (value & TX_PWR_G_BAND_OFDM_24_36M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_24_36M_SIGN) {
                cap->tx_pwr_g_band_ofdm_24_36 = (value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_24_36 = -(value & TX_PWR_G_BAND_OFDM_24_36M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_24_36 = 0;
        }

        /* OFDM 48M */
        if (value & TX_PWR_G_BAND_OFDM_48M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_48M_SIGN) {
                cap->tx_pwr_g_band_ofdm_48 = ((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            } else {
                cap->tx_pwr_g_band_ofdm_48 = -((value & TX_PWR_G_BAND_OFDM_48M_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_48 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_G_BAND_OFDM_54M, value);
    if (is_empty) {
        cap->tx_pwr_g_band_ofdm_54 = 0;
        cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
    } else {
        /* OFDM 54M */
        if (value & TX_PWR_G_BAND_OFDM_54M_EN) {
            if (value & TX_PWR_G_BAND_OFDM_54M_SIGN) {
                cap->tx_pwr_g_band_ofdm_54 = (value & TX_PWR_G_BAND_OFDM_54M_MASK);
            } else {
                cap->tx_pwr_g_band_ofdm_54 = -(value & TX_PWR_G_BAND_OFDM_54M_MASK);
            }
        } else {
            cap->tx_pwr_g_band_ofdm_54 = 0;
        }

        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_0_8_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_0_8_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = ((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_0_8 = -((value & TX_PWR_HT_BPSK_MCS_0_8_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_0_8 = 0;
        }    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_BPSK_MCS_32, value);
    if (is_empty) {
        cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
    } else {
        /* HT MCS_0, MCS_8 */
        if (value & TX_PWR_HT_BPSK_MCS_32_EN) {
            if (value & TX_PWR_HT_BPSK_MCS_32_SIGN) {
                cap->tx_pwr_ht_bpsk_mcs_32 = (value & TX_PWR_HT_BPSK_MCS_32_MASK);
            } else {
                cap->tx_pwr_ht_bpsk_mcs_32 = -(value & TX_PWR_HT_BPSK_MCS_32_MASK);
            }
        } else {
            cap->tx_pwr_ht_bpsk_mcs_32 = 0;
        }

        /* HT MCS_1, MCS_2, MCS_9, MCS_10 */
        if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_EN) {
            if (value & TX_PWR_HT_QPSK_MCS_1_2_9_10_SIGN) {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = ((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = -((value & TX_PWR_HT_QPSK_MCS_1_2_9_10_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_qpsk_mcs_1_2_9_10 = 0;
        }
    }

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_16QAM_MCS_3_4_11_12, value);
    if (is_empty) {
        cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
    } else {
        /* HT MCS_3, MCS_4, MCS_11, MCS_12 */
        if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_EN) {
            if (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_SIGN) {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = (value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            } else {
                cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = -(value & TX_PWR_HT_16QAM_MCS_3_4_11_12_MASK);
            }
        } else {
            cap->tx_pwr_ht_16qam_mcs_3_4_11_12 = 0;
        }

        /* HT MCS_5, MCS_13 */
        if (value & TX_PWR_HT_64QAM_MCS_5_13_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_5_13_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_5_13 = ((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_5_13 = -((value & TX_PWR_HT_64QAM_MCS_5_13_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_5_13 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX_PWR_HT_64QAM_MCS_6_14, value);
    if (is_empty) {
        cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
    } else {
        /* HT MCS_6, MCS_14 */
        if (value & TX_PWR_HT_64QAM_MCS_6_14_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_6_14_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_6_14 = (value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            } else {
                cap->tx_pwr_ht_64qam_mcs_6_14 = -(value & TX_PWR_HT_64QAM_MCS_6_14_MASK);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_6_14 = 0;
        }

        /* HT MCS_7, MCS_15 */
        if (value & TX_PWR_HT_64QAM_MCS_7_15_EN) {
            if (value & TX_PWR_HT_64QAM_MCS_7_15_SIGN) {
                cap->tx_pwr_ht_64qam_mcs_7_15 = ((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            } else {
                cap->tx_pwr_ht_64qam_mcs_7_15 = -((value & TX_PWR_HT_64QAM_MCS_7_15_MASK) >> 8);
            }
        } else {
            cap->tx_pwr_ht_64qam_mcs_7_15 = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	return;
}


void mt7628_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
    bool is_empty = 0;
    UINT16 value = 0;
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;

    /* Read 20/40 BW delta */
    is_empty = RT28xx_EEPROM_READ16(pAd, G_BAND_20_40_BW_PWR_DELTA, value);

	if (is_empty) {
        cap->delta_tx_pwr_bw40_g_band = 0x0;
    } else {
        /* G Band */
        if (value & G_BAND_20_40_BW_PWR_DELTA_EN) {
            if (value & G_BAND_20_40_BW_PWR_DELTA_SIGN) {
                /* bit[0..5] tx power delta value */
                cap->delta_tx_pwr_bw40_g_band = (value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            } else {
                cap->delta_tx_pwr_bw40_g_band = -(value & G_BAND_20_40_BW_PWR_DELTA_MASK);
            }
        } else {
            cap->delta_tx_pwr_bw40_g_band = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

    /////////////////// Tx0 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TSSI_SLOPE, value);

    cap->tssi_0_slope_g_band =
        (is_empty) ? TSSI_0_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX0_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_0_offset_g_band =
        (is_empty) ? TSSI_0_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX0_G_BAND_TSSI_OFFSET_MASK) >> 8);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_slope_g_band = %d\n", cap->tssi_0_slope_g_band));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_offset_g_band = %d\n", cap->tssi_0_offset_g_band));
    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_TARGET_PWR, value);

    cap->tx_0_target_pwr_g_band =
        (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX0_G_BAND_TARGET_PWR_MASK);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_0_target_pwr_g_band = %d\n", cap->tx_0_target_pwr_g_band));

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
    } else {
        /* tx power offset LOW */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX0_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX0_G_BAND_CHL_PWR_DELTA_MID, value);

    if (is_empty) {
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
    } else {
        /* tx power offset MID */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_MID_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX0_G_BAND_CHL_PWR_DELTA_MID_MASK);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x0;
        }
        /* tx power offset HIGH */
        if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX0_G_BAND_CHL_PWR_DELTA_HI_SIGN)
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            else
                cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX0_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
        } else {
            cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));

    /////////////////// Tx1 //////////////////////////
    /* Read TSSI slope/offset for TSSI compensation */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TSSI_SLOPE, value);

    cap->tssi_1_slope_g_band = (is_empty) ? TSSI_1_SLOPE_G_BAND_DEFAULT_VALUE : (value & TX1_G_BAND_TSSI_SLOPE_MASK);

    cap->tssi_1_offset_g_band = (is_empty) ? TSSI_1_OFFSET_G_BAND_DEFAULT_VALUE : ((value & TX1_G_BAND_TSSI_OFFSET_MASK) >> 8);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_1_slope_g_band = %d\n", cap->tssi_1_slope_g_band));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_1_offset_g_band = %d\n", cap->tssi_1_offset_g_band));

    /* Read 54M target power */
    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_TARGET_PWR, value);

    cap->tx_1_target_pwr_g_band = (is_empty) ? TX_TARGET_PWR_DEFAULT_VALUE : (value & TX1_G_BAND_TARGET_PWR_MASK);

    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tssi_1_target_pwr_g_band = %d\n", cap->tx_1_target_pwr_g_band));

    /* Read power offset (channel delta) */
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] =  0;
    } else {
        /* tx power offset LOW */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_LOW_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = ((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = -((value & TX1_G_BAND_CHL_PWR_DELTA_LOW_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));

    is_empty = RT28xx_EEPROM_READ16(pAd, TX1_G_BAND_CHL_PWR_DELTA_MID, value);
    if (is_empty) {
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
    } else {
        /* tx power offset MID */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_MID_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = (value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = -(value & TX1_G_BAND_CHL_PWR_DELTA_MID_MASK);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0;
        }
        /* tx power offset HIGH */
        if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_EN) {
            if (value & TX1_G_BAND_CHL_PWR_DELTA_HI_SIGN) {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = ((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            } else {
                cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = -((value & TX1_G_BAND_CHL_PWR_DELTA_HI_MASK) >> 8);
            }
        } else {
            cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0;
        }
    }
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
    MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = %d\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

    return ;
}


static VOID mt7628_show_pwr_info(RTMP_ADAPTER *pAd)
{
	struct MT_TX_PWR_CAP *cap = &pAd->chipCap.MTTxPwrCap;
	UINT32 value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel info related to power\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));

	if (pAd->LatchRfRegs.Channel < 14) {
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("central channel = %d, low_mid_hi = %d\n", pAd->LatchRfRegs.Channel,
							get_low_mid_hi_index(pAd->LatchRfRegs.Channel)));
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel power(unit: 0.5dbm)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_target_pwr_g_band = 0x%x\n", cap->tx_0_target_pwr_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_target_pwr_g_band = 0x%x\n", cap->tx_1_target_pwr_g_band));

	/* channel power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("channel power delta(unit: 0.5db)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_LOW]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_MID]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_0_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_0_chl_pwr_delta_g_band[G_BAND_HI]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_LOW] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_LOW]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_MID] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_MID]));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_1_chl_pwr_delta_g_band[G_BAND_HI] = 0x%x\n", cap->tx_1_chl_pwr_delta_g_band[G_BAND_HI]));

	/* bw power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("bw power delta(unit: 0.5db)\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("delta_tx_pwr_bw40_g_band = %d\n", cap->delta_tx_pwr_bw40_g_band));

	/* per-rate power delta */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("per-rate power delta\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_cck_1_2 = %d\n", cap->tx_pwr_cck_1_2));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_cck_5_11 = %d\n", cap->tx_pwr_cck_5_11));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_6_9 = %d\n", cap->tx_pwr_g_band_ofdm_6_9));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_12_18 = %d\n", cap->tx_pwr_g_band_ofdm_12_18));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_24_36 = %d\n", cap->tx_pwr_g_band_ofdm_24_36));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_48 = %d\n", cap->tx_pwr_g_band_ofdm_48));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_g_band_ofdm_54 = %d\n", cap->tx_pwr_g_band_ofdm_54));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_bpsk_mcs_32 = %d\n", cap->tx_pwr_ht_bpsk_mcs_32));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_bpsk_mcs_0_8 = %d\n", cap->tx_pwr_ht_bpsk_mcs_0_8));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_qpsk_mcs_1_2_9_10 = %d\n", cap->tx_pwr_ht_qpsk_mcs_1_2_9_10));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_16qam_mcs_3_4_11_12 = %d\n", cap->tx_pwr_ht_16qam_mcs_3_4_11_12));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_5_13 = %d\n", cap->tx_pwr_ht_64qam_mcs_5_13));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_6_14 = %d\n", cap->tx_pwr_ht_64qam_mcs_6_14));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tx_pwr_ht_64qam_mcs_7_15 = %d\n", cap->tx_pwr_ht_64qam_mcs_7_15));

	/* TMAC POWER INFO */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("per-rate power delta in MAC 0x60130020 ~ 0x60130030\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	RTMP_IO_READ32(pAd, TMAC_FP0R0, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R0 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R1, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R1 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R2, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R2 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R3, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R3 = 0x%x\n", value));
	RTMP_IO_READ32(pAd, TMAC_FP0R4, &value);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TMAC_FP0R4 = 0x%x\n", value));

	/* TSSI info */
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\n===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TSSI info\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("===================================\n"));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("TSSI enable = %d\n", pAd->chipCap.tssi_enable));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_0_slope_g_band = 0x%x\n", cap->tssi_0_slope_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_1_slope_g_band = 0x%x\n", cap->tssi_1_slope_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_0_offset_g_band = 0x%x\n", cap->tssi_0_offset_g_band));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("tssi_1_offset_g_band = 0x%x\n", cap->tssi_1_offset_g_band));
}


#ifdef CAL_FREE_IC_SUPPORT
static BOOLEAN mt7628_is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x54, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x56, &Value);

	if (NotValid)
		return FALSE;

	if (Value == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0x5c, &Value);

	if (NotValid)
		return FALSE;

	if (Value == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf0, &Value);

	if (NotValid)
		return FALSE;

	if ((Value & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf4, &Value);

	if (NotValid)
		return FALSE;

	if ((Value & 0xff) == 0x00)
		return FALSE;

	NotValid = rtmp_ee_efuse_read16(pAd, 0xf6, &Value);

	if (NotValid)
		return FALSE;

	if (((Value >> 8) & 0xff) == 0x00)
		return FALSE;

	return TRUE;
}



static VOID mt7628_cal_free_data_get(RTMP_ADAPTER *ad)

{

	UINT16 value;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));



	/* 0x55 0x56 0x57 0x5C 0x5D */
	eFuseReadRegisters(ad, A_BAND_EXT_PA_SETTING, 2, &value);
	/*0x55*/
	ad->EEPROMImage[A_BAND_EXT_PA_SETTING + 1] = (value >> 8) & 0xFF;

	//eFuseReadRegisters(ad, TX0_G_BAND_TSSI_SLOPE, 2, &value);
	/*0x56,0x57*/

	//*(UINT16 *)(&ad->EEPROMImage[TX0_G_BAND_TSSI_SLOPE]) = le2cpu16(value);



//	eFuseReadRegisters(ad, TX1_G_BAND_TSSI_SLOPE, 2, &value);

	/*0x5c,0x5d*/
//	*(UINT16 *)(&ad->EEPROMImage[TX1_G_BAND_TSSI_SLOPE]) = le2cpu16(value);

	eFuseReadRegisters(ad, TX1_G_BAND_TARGET_PWR, 2, &value);


	/* 0xF0 0xF4 0xF7  */

	eFuseReadRegisters(ad, CP_FT_VERSION, 2, &value);

	ad->EEPROMImage[CP_FT_VERSION] = value & 0xFF;

	eFuseReadRegisters(ad, XTAL_CALIB_FREQ_OFFSET, 2, &value);

	ad->EEPROMImage[XTAL_CALIB_FREQ_OFFSET]  = value & 0xFF;

	eFuseReadRegisters(ad, XTAL_TRIM_3_COMP, 2, &value);

	ad->EEPROMImage[XTAL_TRIM_3_COMP+1] = (value >> 8) & 0xFF;


}


#endif /* CAL_FREE_IC_SUPPORT */





static void mt7628_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7628;
	pAntenna->field.TxPath = 2;
	pAntenna->field.RxPath = 2;
}

#ifdef CONFIG_STA_SUPPORT
static VOID mt7628_init_dev_nick_name(RTMP_ADAPTER *ad)
{
#ifdef RTMP_MAC_PCI
	if (IS_MT7628(ad))
		snprintf((RTMP_STRING *) ad->nickname, sizeof(ad->nickname), "mt7628_sta");
#endif
}
#endif /* CONFIG_STA_SUPPORT */

static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd,UINT16 addr)
{
	UINT32 i = pCmd->ucCount;
	pCmd->aBinContent[i].u2Addr = cpu2le16(addr);
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[addr] ;
	pCmd->ucCount++;
}


static VOID mt7628_bufferModeEfuseFill(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	int i=0;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_0_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_0_TOP] ;
	i++;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_1];
	i++;
	pCmd->aBinContent[i].u2Addr = NIC_CONFIGURE_1_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[NIC_CONFIGURE_1_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = WIFI_RF_SETTING;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[WIFI_RF_SETTING];
	i++;
	pCmd->aBinContent[i].u2Addr = G_BAND_20_40_BW_PWR_DELTA;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[G_BAND_20_40_BW_PWR_DELTA];
	i++;
	pCmd->aBinContent[i].u2Addr = A_BAND_20_80_BW_PWR_DELTA_ANALOG;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[A_BAND_20_80_BW_PWR_DELTA_ANALOG];
	i++;
	pCmd->aBinContent[i].u2Addr = A_BAND_EXT_PA_SETTING;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[A_BAND_EXT_PA_SETTING];
	i++;
	pCmd->aBinContent[i].u2Addr = TEMP_SENSOR_CAL;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TEMP_SENSOR_CAL];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TSSI_SLOPE;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TSSI_SLOPE];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TSSI_SLOPE_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TSSI_SLOPE_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_TARGET_PWR;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_TARGET_PWR];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_OFFSET_LOW;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_OFFSET_LOW];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_CHL_PWR_DELTA_MID;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_CHL_PWR_DELTA_MID];
	i++;
	pCmd->aBinContent[i].u2Addr = TX0_G_BAND_OFFSET_HIGH;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX0_G_BAND_OFFSET_HIGH];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TSSI_SLOPE;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TSSI_SLOPE];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TSSI_SLOPE_TOP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TSSI_SLOPE_TOP];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_TARGET_PWR;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_TARGET_PWR];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELATE_LOW;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELATE_LOW];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELTA_MID;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_MID];
	i++;
	pCmd->aBinContent[i].u2Addr = TX1_G_BAND_CHL_PWR_DELTA_HIGH;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX1_G_BAND_CHL_PWR_DELTA_HIGH];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_CCK_1_2M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_CCK_1_2M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_CCK_5_11M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_CCK_5_11M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_6_9M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_6_9M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_OFDM_12_18M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_OFDM_12_18M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_24_36M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_24_36M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BNAD_OFDM_48;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BNAD_OFDM_48];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_G_BAND_OFDM_54M;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_G_BAND_OFDM_54M];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_BPSK_MCS_0_8;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_BPSK_MCS_0_8];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_BPSK_MCS_32;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_BPSK_MCS_32];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_QPSK_MCS_1_2_9_10;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_QPSK_MCS_1_2_9_10];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_16QAM_MCS_3_4_11_12;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_16QAM_MCS_3_4_11_12];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_5_13;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_5_13];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_6_14;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_6_14];
	i++;
	pCmd->aBinContent[i].u2Addr = TX_PWR_HT_64QAM_MCS_7_15;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[TX_PWR_HT_64QAM_MCS_7_15];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_GAIN;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_GAIN];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_NF;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_NF];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_RX_MODE_P1DB;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_RX_MODE_P1DB];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_GAIN;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_GAIN];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_NF;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_NF];
	i++;
	pCmd->aBinContent[i].u2Addr = ELAN_BYPASS_MODE_P1DB;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[ELAN_BYPASS_MODE_P1DB];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_7;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_7];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_6;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_6];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_5;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_5];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_4;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_4];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_3;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_3];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_2;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_2];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_1];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_NEG_0;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_NEG_0];
	i++;
	pCmd->aBinContent[i].u2Addr = REF_STEP_24G;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[REF_STEP_24G];
	i++;
	pCmd->aBinContent[i].u2Addr = REF_TEMP_24G;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[REF_TEMP_24G];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_1;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_1];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_2;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_2];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_3;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_3];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_4;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_4];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_5;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_5];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_6;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_6];
	i++;
	pCmd->aBinContent[i].u2Addr = STEP_NUM_PLUS_7;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[STEP_NUM_PLUS_7];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_CALIB_FREQ_OFFSET;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_CALIB_FREQ_OFFSET];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_TRIM_2_COMP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_TRIM_2_COMP];
	i++;
	pCmd->aBinContent[i].u2Addr = XTAL_TRIM_3_COMP;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[XTAL_TRIM_3_COMP];
	i++;
	pCmd->aBinContent[i].u2Addr = WF_RCAL;
	pCmd->aBinContent[i].ucValue = pAd->EEPROMImage[WF_RCAL];
	i++;

	pCmd->ucCount = i;

	for(i=0;i<pCmd->ucCount;i++)
	{
		pCmd->aBinContent[i].u2Addr = cpu2le16(pCmd->aBinContent[i].u2Addr);
	}


	/*extend for function requset, need backward compatible*/
	bufferModeFieldSet(pAd,pCmd,0x24);
	bufferModeFieldSet(pAd,pCmd,0x25);
	bufferModeFieldSet(pAd,pCmd,0x34);
	bufferModeFieldSet(pAd,pCmd,0x39);
	bufferModeFieldSet(pAd,pCmd,0x3b);
	bufferModeFieldSet(pAd,pCmd,0x42);
	bufferModeFieldSet(pAd,pCmd,0x43);
	bufferModeFieldSet(pAd,pCmd,0x9e);
	bufferModeFieldSet(pAd,pCmd,0x9f);
	bufferModeFieldSet(pAd,pCmd,0xf2);
	bufferModeFieldSet(pAd,pCmd,0xf8);
	bufferModeFieldSet(pAd,pCmd,0xf9);
	bufferModeFieldSet(pAd,pCmd,0xfa);
	bufferModeFieldSet(pAd,pCmd,0x12e);
	for(i=0;i<=0xf;i++)
	{
		bufferModeFieldSet(pAd,pCmd,0x130+i);
	}
	/*need minus 1 for add one more time*/
	pCmd->ucCount--;

}


static const RTMP_CHIP_CAP MT7628_ChipCap = {
	.max_nss = 2,
	.TXWISize = sizeof(TMAC_TXD_L), /* 32 */
	.RXWISize = 28,
	.WtblHwNum = MT7628_MT_WTBL_SIZE,
	.WtblPseAddr = MT7628_PSE_WTBL_2_ADDR,
#ifdef RTMP_MAC_PCI
	.WPDMABurstSIZE = 3,
#endif
	.SnrFormula = SNR_FORMULA4,
	.FlgIsHwWapiSup = TRUE,
	.FlgIsHwAntennaDiversitySup = FALSE,
#ifdef STREAM_MODE_SUPPORT
	.FlgHwStreamMode = FALSE,
#endif
#ifdef FIFO_EXT_SUPPORT
	.FlgHwFifoExtCap = FALSE,
#endif
	.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT),
	.phy_caps = (fPHY_CAP_24G | fPHY_CAP_HT),
	.MaxNumOfRfId = MAX_RF_ID,
	.pRFRegTable = NULL,
	.MaxNumOfBbpId = 200,
	.pBBPRegTable = NULL,
	.bbpRegTbSize = 0,
	/* Force MT7628 use MBSSID_MODE2~ MBSSID_MODE6 of ENHANCE_NEW_MBSSID_MODE*/
	.MBSSIDMode = MBSSID_MODE4,
#ifdef RTMP_EFUSE_SUPPORT
	.EFUSE_USAGE_MAP_START = 0x1e0,
	.EFUSE_USAGE_MAP_END = 0x1fc,
	.EFUSE_USAGE_MAP_SIZE = 29,
	.EFUSE_RESERVED_SIZE = 22,	// Cal-Free is 22 free block
#endif
	.EEPROM_DEFAULT_BIN = MT7628_E2PImage,
	.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7628_E2PImage),
#ifdef CONFIG_ANDES_SUPPORT
	.CmdRspRxRing = RX_RING1,
	.need_load_fw = TRUE,
	.DownLoadType = DownLoadTypeA,
	/*.load_code_method = BIN_FILE_METHOD,*/
	.load_code_method = HEADER_METHOD,
#endif
	.MCUType = ANDES,
	.cmd_header_len = 12,
#ifdef RTMP_PCI_SUPPORT
	.cmd_padding_len = 0,
#endif
#ifdef MT7628_E1
	.fw_header_image = MT7628_FirmwareImage,
	.fw_len = sizeof(MT7628_FirmwareImage),
#endif
	.fw_bin_file_name = "mtk/MT7628.bin",
#ifdef CARRIER_DETECTION_SUPPORT
	.carrier_func = TONE_RADAR_V2,
#endif
#ifdef CONFIG_WIFI_TEST
	.MemMapStart = 0x0000,
	.MemMapEnd = 0xffff,
	.MacMemMapOffset = 0x1000,
	.MacStart = 0x0000,
	.MacEnd = 0x0600,
	.BBPMemMapOffset = 0x2000,
	.BBPStart = 0x0000,
	.BBPEnd = 0x0f00,
	.RFIndexNum = 0,
	.RFIndexOffset = 0,
	.E2PStart = 0x0000,
	.E2PEnd = 0x00fe,
#endif /* CONFIG_WIFI_TEST */
	.hif_type = HIF_MT,
	.rf_type = RF_MT,
	.RxBAWinSize = 64,
	.AMPDUFactor = 3,
	.TxOPScenario = 1 // Scenario_1
};


static const RTMP_CHIP_OP MT7628_ChipOp = {
	.ChipBBPAdjust = mt7628_bbp_adjust,
	.ChipSwitchChannel = mt7628_switch_channel,
	.AsicMacInit = mt7628_init_mac_cr,
	.AsicAntennaDefaultReset = mt7628_antenna_default_reset,
	.ChipAGCInit = NULL,
#ifdef CONFIG_STA_SUPPORT
	.ChipAGCAdjust = NULL,
#endif
	.AsicRfTurnOn = NULL,
	.AsicHaltAction = NULL,
	.AsicRfTurnOff = NULL,
	.AsicReverseRfFromSleepMode = NULL,
#ifdef CONFIG_STA_SUPPORT
	.NetDevNickNameInit = mt7628_init_dev_nick_name,
#endif
#ifdef CARRIER_DETECTION_SUPPORT
	.ToneRadarProgram = ToneRadarProgram_v2,
#endif
	.RxSensitivityTuning = NULL,
	.DisableTxRx = NULL,

#ifdef RTMP_PCI_SUPPORT
	//.AsicRadioOn = RT28xxPciAsicRadioOn,
	//.AsicRadioOff = RT28xxPciAsicRadioOff,
#endif
#ifdef CAL_FREE_IC_SUPPORT
	.is_cal_free_ic = mt7628_is_cal_free_ic,
	.cal_free_data_get = mt7628_cal_free_data_get,
#endif /* CAL_FREE_IC_SUPPORT */
	.show_pwr_info = mt7628_show_pwr_info,
	.bufferModeEfuseFill =mt7628_bufferModeEfuseFill,
	.ChipSetEDCCA = mt7628_set_ed_cca,
#ifdef GREENAP_SUPPORT
	.EnableAPMIMOPS = EnableAPMIMOPSv2,
	.DisableAPMIMOPS = DisableAPMIMOPSv2,
#endif /* GREENAP_SUPPORT */
};


VOID mt7628_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	memcpy(&pAd->chipCap, &MT7628_ChipCap, sizeof(RTMP_CHIP_CAP));
	memcpy(&pAd->chipOps, &MT7628_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
	pAd->chipCap.mac_type = MAC_MT;

	mt_phy_probe(pAd);

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(FW(%x), HW(%x), CHIPID(%x))\n",
							__FUNCTION__,
							pAd->FWVersion,
							pAd->HWVersion,
							pAd->ChipID));

#ifdef MT7628_E1
	if (((pAd->HWVersion & 0x0000ffff) == 0x8A00) && IS_MT7628(pAd))
	{
		pChipCap->fw_header_image = MT7628_FirmwareImage;
		pChipCap->fw_bin_file_name = "mtk/WIFI_RAM_CODE_MT7628_e1.bin";
		pChipCap->fw_len = sizeof(MT7628_FirmwareImage);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("e1.bin %s(%d)::(1), pChipCap->fw_len(%d)\n", __FUNCTION__, __LINE__, pChipCap->fw_len));
	}
	else
#endif
	if (((pAd->HWVersion & 0x0000ffff) == 0x8A01) && IS_MT7628(pAd))
	{
		pChipCap->fw_header_image = MT7628_e2_FirmwareImage;
		pChipCap->fw_bin_file_name = "mtk/WIFI_RAM_CODE_MT7628_e2.bin";
		pChipCap->fw_len = sizeof(MT7628_e2_FirmwareImage);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("e2.bin %s(%d)::(2), pChipCap->fw_len(%d)\n", __FUNCTION__, __LINE__, pChipCap->fw_len));
	}
	else
	{
		pChipCap->fw_header_image = MT7628_e2_FirmwareImage;
		pChipCap->fw_bin_file_name = "mtk/WIFI_RAM_CODE_MT7628_e2.bin";
		pChipCap->fw_len = sizeof(MT7628_e2_FirmwareImage);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("e2.bin %s(%d)::(3), pChipCap->fw_len(%d)\n", __FUNCTION__, __LINE__, pChipCap->fw_len));
	}

	pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;
	pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;




	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_GRP);

	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum /
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
	mt_bcn_buf_init(pAd);

#ifdef DOT11W_PMF_SUPPORT
	pChipCap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __FUNCTION__));
}


void mt7628_set_ed_cca(RTMP_ADAPTER *pAd, BOOLEAN enable)
{
	
	UINT32 macVal = 0, macVal2 = 0;
	UINT32 NBIDmacVal = 0;

	RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0634, &macVal2);

	if (enable)
	{
		macVal = 0xD7C87D0F;  //EDCCA ON , TH - L, USER case  //D7C87D0F
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

		macVal2 |= 0x1;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);

#ifdef SMART_CARRIER_SENSE_SUPPORT	
		pAd->SCSCtrl.EDCCA_Status = 1;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TURN ON EDCCA mac 0x10618 = 0x%x, EDCCA_Status=%d\n", __FUNCTION__, macVal, pAd->SCSCtrl.EDCCA_Status));
#else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TURN ON EDCCA mac 0x10618 = 0x%x\n", __FUNCTION__, macVal));
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		
	}
	else
	{
		macVal = 0xD7083F0F;  //EDCCA OFF //d7083f0f		
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0618, macVal);

		macVal2 &= 0xFFFFFFFE;
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0634, macVal2);
		
#ifdef SMART_CARRIER_SENSE_SUPPORT
		pAd->SCSCtrl.EDCCA_Status = 0;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TURN OFF EDCCA  mac 0x10618 = 0x%x, EDCCA_Status=%d\n", __FUNCTION__, macVal, pAd->SCSCtrl.EDCCA_Status));
#else
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TURN OFF EDCCA  mac 0x10618 = 0x%x\n", __FUNCTION__, macVal));
#endif /* SMART_CARRIER_SENSE_SUPPORT */
		
		
	}

	if (strncmp(pAd->CommonCfg.CountryCode, "JP", 2) == 0)
	{
		/* disable NBID for JAPAN carrier sense test mac, 0610[24]=0 [31]=0 */		
		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0610, &NBIDmacVal);
		
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: pAd->CommonCfg.CountryCode = %s \n", __FUNCTION__, pAd->CommonCfg.CountryCode));
		NBIDmacVal &= ~(1<<24); 
		NBIDmacVal &= ~(1<<31); 
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0610, NBIDmacVal);
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: TURN OFF NBID mac 0x10610 = 0x%x\n", __FUNCTION__, NBIDmacVal));
	}
	else
	{
		RTMP_IO_READ32(pAd, WF_PHY_BASE + 0x0610, &NBIDmacVal);		
		NBIDmacVal |= (1<<24); 
		NBIDmacVal |= (1<<31); 
		RTMP_IO_WRITE32(pAd, WF_PHY_BASE + 0x0610, NBIDmacVal);		
	}

}



