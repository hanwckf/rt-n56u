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
	mt7615.c
*/

#include "rt_config.h"
#include "mcu/mt7615_firmware.h"

#include "mcu/mt7615_cr4_firmware.h"
#ifdef NEED_ROM_PATCH
#include "mcu/mt7615_rom_patch.h"
#endif /* NEED_ROM_PATCH */

#include "hdev/hdev_basic.h"


/* ePAeLNA shall always be included as default */
#include "eeprom/mt7615_e2p_ePAeLNA.h"

#if defined (CONFIG_FIRST_IF_IPAILNA) || defined (CONFIG_SECOND_IF_IPAILNA) || defined (CONFIG_THIRD_IF_IPAILNA)
#include "eeprom/mt7615_e2p_iPAiLNA.h"
#endif
#if defined (CONFIG_FIRST_IF_IPAELNA) || defined (CONFIG_SECOND_IF_IPAELNA) || defined (CONFIG_THIRD_IF_IPAELNA)
#include "eeprom/mt7615_e2p_iPAeLNA.h"
#endif
#if defined (CONFIG_FIRST_IF_EPAILNA) || defined (CONFIG_SECOND_IF_EPAILNA) || defined (CONFIG_THIRD_IF_EPAILNA)
#include "eeprom/mt7615_e2p_ePAiLNA.h"
#endif

#ifdef CONFIG_AP_SUPPORT
#define DEFAULT_BIN_FILE "/etc_ro/wlan/MT7615E_EEPROM1.bin"
#else
#define DEFAULT_BIN_FILE "/etc/MT7615E_EEPROM1.bin"
#endif /* CONFIG_AP_SUPPORT */

#ifdef MT_SECOND_CARD
#define SECOND_BIN_FILE "/etc_ro/wlan/MT7615E_EEPROM2.bin"
#endif /* MT_SECOND_CARD */
#ifdef MT_THIRD_CARD
#define THIRD_BIN_FILE "/etc_ro/wlan/MT7615E_EEPROM3.bin"
#endif /* MT_THIRD_CARD */

extern UCHAR g_BFBackOffMode; // BF Backoff Mode: 2/3/4: apply 2T/3T/4T value in BF backoff table

#ifdef MT7615_FPGA
REG_CHK_PAIR hif_dft_cr[]=
{
	{HIF_BASE + 0x00, 0xffffffff, 0x76030001},
	{HIF_BASE + 0x04, 0xffffffff, 0x1b},
	{HIF_BASE + 0x10, 0xffffffff, 0x3f01},
	{HIF_BASE + 0x20, 0xffffffff, 0xe01001e0},
	{HIF_BASE + 0x24, 0xffffffff, 0x1e00000f},

	{HIF_BASE + 0x200, 0xffffffff, 0x0},
	{HIF_BASE + 0x204, 0xffffffff, 0x0},
	{HIF_BASE + 0x208, 0xffffffff, 0x10001870},
	{HIF_BASE + 0x20c, 0xffffffff, 0x0},
	{HIF_BASE + 0x210, 0xffffffff, 0x0},
	{HIF_BASE + 0x214, 0xffffffff, 0x0},
	{HIF_BASE + 0x218, 0xffffffff, 0x0},
	{HIF_BASE + 0x21c, 0xffffffff, 0x0},
	{HIF_BASE + 0x220, 0xffffffff, 0x0},
	{HIF_BASE + 0x224, 0xffffffff, 0x0},
	{HIF_BASE + 0x234, 0xffffffff, 0x0},
	{HIF_BASE + 0x244, 0xffffffff, 0x0},
	{HIF_BASE + 0x300, 0xffffffff, 0x0},
	{HIF_BASE + 0x304, 0xffffffff, 0x0},
	{HIF_BASE + 0x308, 0xffffffff, 0x0},
	{HIF_BASE + 0x30c, 0xffffffff, 0x0},
	{HIF_BASE + 0x310, 0xffffffff, 0x0},
	{HIF_BASE + 0x314, 0xffffffff, 0x0},
	{HIF_BASE + 0x318, 0xffffffff, 0x0},
	{HIF_BASE + 0x31c, 0xffffffff, 0x0},
	{HIF_BASE + 0x320, 0xffffffff, 0x0},
	{HIF_BASE + 0x324, 0xffffffff, 0x0},
	{HIF_BASE + 0x328, 0xffffffff, 0x0},
	{HIF_BASE + 0x32c, 0xffffffff, 0x0},
	{HIF_BASE + 0x330, 0xffffffff, 0x0},
	{HIF_BASE + 0x334, 0xffffffff, 0x0},
	{HIF_BASE + 0x338, 0xffffffff, 0x0},
	{HIF_BASE + 0x33c, 0xffffffff, 0x0},

	{HIF_BASE + 0x400, 0xffffffff, 0x0},
	{HIF_BASE + 0x404, 0xffffffff, 0x0},
	{HIF_BASE + 0x408, 0xffffffff, 0x0},
	{HIF_BASE + 0x40c, 0xffffffff, 0x0},
	{HIF_BASE + 0x410, 0xffffffff, 0x0},
	{HIF_BASE + 0x414, 0xffffffff, 0x0},
	{HIF_BASE + 0x418, 0xffffffff, 0x0},
	{HIF_BASE + 0x41c, 0xffffffff, 0x0},
};


INT mt7615_chk_hif_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for HIF!\n", __FUNCTION__));
	for (i = 0; i < sizeof(hif_dft_cr) / sizeof(REG_CHK_PAIR); i++)
	{
		RTMP_IO_READ32(pAd, hif_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
						hif_dft_cr[i].Register, val, (val & hif_dft_cr[i].Mask),
						hif_dft_cr[i].Value, hif_dft_cr[i].Mask,
						((val & hif_dft_cr[i].Mask)!= hif_dft_cr[i].Value) ? "No" : "Yes"));

		if ((val & hif_dft_cr[i].Mask)!= hif_dft_cr[i].Value)
		{
			match = FALSE;
		}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
				__FUNCTION__, match == TRUE ? "All" : "No"));

	return match;
}


REG_CHK_PAIR top_dft_cr[]=
{
	{TOP_CFG_BASE+ 0x1000, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1004, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1008, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1010, 0xffffffff, 0x0},

	{TOP_CFG_BASE+ 0x1100, 0xffffffff, 0x26110310},
	{TOP_CFG_BASE+ 0x1108, 0x0000ff00, 0x1400},
	{TOP_CFG_BASE+ 0x110c, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1110, 0x0f0f00ff, 0x02090040},
	{TOP_CFG_BASE+ 0x1124, 0xf000f00f, 0x00000008},
	{TOP_CFG_BASE+ 0x1130, 0x000f0000, 0x0},
	{TOP_CFG_BASE+ 0x1134, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1140, 0x00ff00ff, 0x0},

	{TOP_CFG_BASE+ 0x1200, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1204, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1208, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x120c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1210, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1214, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1218, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x121c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1220, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1224, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1228, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x122c, 0x000fffff, 0x0},
	{TOP_CFG_BASE+ 0x1234, 0x00ffffff, 0x0},
	{TOP_CFG_BASE+ 0x1238, 0x00ffffff, 0x0},
	{TOP_CFG_BASE+ 0x123c, 0xffffffff, 0x5c1fee80},
	{TOP_CFG_BASE+ 0x1240, 0xffffffff, 0x6874ae05},
	{TOP_CFG_BASE+ 0x1244, 0xffffffff, 0x00fb89f1},

	{TOP_CFG_BASE+ 0x1300, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1304, 0xffffffff, 0x8f020006},
	{TOP_CFG_BASE+ 0x1308, 0xffffffff, 0x18010000},
	{TOP_CFG_BASE+ 0x130c, 0xffffffff, 0x0130484f},
	{TOP_CFG_BASE+ 0x1310, 0xffffffff, 0xff000004},
	{TOP_CFG_BASE+ 0x1314, 0xffffffff, 0xf0000084},
	{TOP_CFG_BASE+ 0x1318, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x131c, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1320, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1324, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1328, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x132c, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1330, 0xffffffff, 0x00007800},
	{TOP_CFG_BASE+ 0x1334, 0x00000000, 0x0},
	{TOP_CFG_BASE+ 0x1338, 0xffffffff, 0x0000000a},
	{TOP_CFG_BASE+ 0x1400, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1404, 0xffffffff, 0x00005180},
	{TOP_CFG_BASE+ 0x1408, 0xffffffff, 0x00001f00},
	{TOP_CFG_BASE+ 0x140c, 0xffffffff, 0x00000020},
	{TOP_CFG_BASE+ 0x1410, 0xffffffff, 0x0000003a},
	{TOP_CFG_BASE+ 0x141c, 0xffffffff, 0x0},

	{TOP_CFG_BASE+ 0x1500, 0xffffffff, 0x0},
	{TOP_CFG_BASE+ 0x1504, 0xffffffff, 0x0},
};

INT mt7615_chk_top_default_cr_setting(RTMP_ADAPTER *pAd)
{
	UINT32 val;
	INT i;
	BOOLEAN match = TRUE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Default CR Setting Checking for TOP!\n", __FUNCTION__));
	for (i = 0; i < sizeof(top_dft_cr) / sizeof(REG_CHK_PAIR); i++)
	{
		RTMP_IO_READ32(pAd, top_dft_cr[i].Register, &val);
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("\t Reg(%x): Current=0x%x(0x%x), Default=0x%x, Mask=0x%x, Match=%s\n",
					top_dft_cr[i].Register, val, (val & top_dft_cr[i].Mask),
					top_dft_cr[i].Value, top_dft_cr[i].Mask,
					((val & top_dft_cr[i].Mask)!= top_dft_cr[i].Value) ? "No" : "Yes"));
		if ((val & top_dft_cr[i].Mask) != top_dft_cr[i].Value)
		{
			match = FALSE;
		}
	}
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s(): Checking Done, Result=> %s match!\n",
				__FUNCTION__, match == TRUE ? "All" : "No"));

	return match;

}
#endif /* MT7615_FPGA */


static VOID mt7615_bbp_adjust(RTMP_ADAPTER *pAd, UCHAR Channel)
{
	static char *ext_str[]={"extNone", "extAbove", "", "extBelow"};
	UCHAR rf_bw, ext_ch;
	UCHAR RfIC;
	UCHAR PhyMode;
	BOOLEAN IsSupportRf;

	if(Channel > 14)
		RfIC = RFIC_5GHZ;
	else
		RfIC = RFIC_24GHZ;

	IsSupportRf = HcIsRfSupport(pAd,RfIC);

	if(!IsSupportRf)
		return;

	PhyMode = HcGetPhyModeByRf(pAd,RfIC);

#ifdef DOT11_N_SUPPORT
	if (get_ht_cent_ch(pAd, &rf_bw, &ext_ch,Channel) == FALSE)
#endif /* DOT11_N_SUPPORT */
	{
		rf_bw = BW_20;
		ext_ch = EXTCHA_NONE;
		pAd->CommonCfg.CentralChannel = Channel;
	}

#ifdef DOT11_VHT_AC

	if (WMODE_CAP(PhyMode, WMODE_AC) &&
		(Channel > 14) &&
		(rf_bw == BW_40) &&
		(pAd->CommonCfg.vht_bw >= VHT_BW_80) &&
		(pAd->CommonCfg.vht_cent_ch != pAd->CommonCfg.CentralChannel))
	{
		if (pAd->CommonCfg.vht_bw == VHT_BW_80)
		{
			rf_bw = BW_80;
		}
		else if (pAd->CommonCfg.vht_bw == VHT_BW_160)
		{
			rf_bw = BW_160;
		}
		else if (pAd->CommonCfg.vht_bw == VHT_BW_8080)
		{
			rf_bw = BW_8080;
		}
		pAd->CommonCfg.vht_cent_ch = vht_cent_ch_freq(Channel, pAd->CommonCfg.vht_bw);
	}

//+++Add by shiang for debug
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s():rf_bw=%d, ext_ch=%d, PrimCh=%d, HT-CentCh=%d, VHT-CentCh=%d\n",
				__FUNCTION__, rf_bw, ext_ch, Channel,
				pAd->CommonCfg.CentralChannel, pAd->CommonCfg.vht_cent_ch));
//---Add by shiang for debug
#endif /* DOT11_VHT_AC */

	HcBbpSetBwByChannel(pAd,rf_bw,Channel);

#ifdef DOT11_N_SUPPORT
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s() : %s, ChannelWidth=%d, Channel=%d, ExtChanOffset=%d \n",
					__FUNCTION__, ext_str[ext_ch],
					rf_bw,
					Channel,
					HcGetExtCha(pAd,Channel)));
#endif /* DOT11_N_SUPPORT */
}

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
/* RXDCOC */
UINT16 K_A20Freq[]={4980,5805,5905};
UINT16 K_A40Freq[]={5190,5230,5270,5310,5350
	,5390,5430,5470,5510,5550,5590,5630,5670,5710,5755,5795,5835,5875}; /* delta should <=10 */
UINT16 K_A80Freq[]={5210,5290,5370,5450,5530,5610,5690,5775,5855};
UINT16 K_G20Freq[]={2417,2432,2447,2467}; /* delta should <=5 */
UINT16 K_AllFreq[]={4980,5805,5905,5190,5230,5270,5310,5350,5390,5430
	,5470,5510,5550,5590,5630,5670,5710,5755,5795,5835,5875,5210,5290
	,5370,5450,5530,5610,5690,5775,5855,2417,2432,2447,2467};

UINT16 K_A20_SIZE = (sizeof(K_A20Freq)/sizeof(UINT16));
UINT16 K_A40_SIZE = (sizeof(K_A40Freq)/sizeof(UINT16));
UINT16 K_A80_SIZE = (sizeof(K_A80Freq)/sizeof(UINT16));
UINT16 K_G20_SIZE = (sizeof(K_G20Freq)/sizeof(UINT16));
UINT16 K_ALL_SIZE = (sizeof(K_AllFreq)/sizeof(UINT16));

/* TXDPD */
UINT16 DPD_A20Freq[]={4920,4940,4960,4980,5040,5060,5080,5180,5200,
   5220,5240,5260,5280,5300,5320,5340,5360,5380,5400,5420,5440,5460,5480,
   5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5720,5745,5765,
   5785,5805,5825,5845,5865,5885,5905};
UINT16 DPD_G20Freq[]={2422,2442,2462};
UINT16 DPD_AllFreq[]={4920,4940,4960,4980,5040,5060,5080,5180,5200,
   5220,5240,5260,5280,5300,5320,5340,5360,5380,5400,5420,5440,5460,5480,
   5500,5520,5540,5560,5580,5600,5620,5640,5660,5680,5700,5720,5745,5765,
   5785,5805,5825,5845,5865,5885,5905,2422,2442,2462};
   
UINT16 DPD_A20_SIZE = (sizeof(DPD_A20Freq)/sizeof(UINT16));
UINT16 DPD_G20_SIZE = (sizeof(DPD_G20Freq)/sizeof(UINT16));
UINT16 DPD_ALL_SIZE = (sizeof(DPD_AllFreq)/sizeof(UINT16));
#endif

#ifdef PRE_CAL_TRX_SET1_SUPPORT
void ShowDPDData(RTMP_ADAPTER *pAd, TXDPD_RESULT_T TxDPDResult)
{
	//UINT	i=0;	
	UINT	j=0;
	if(pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s : Currently not in FLASH or BIN MODE,return. \n", __FUNCTION__));
		return;
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF0]: u4DPDG0_WF0_Prim = 0x%x \n", TxDPDResult.u4DPDG0_WF0_Prim));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF1]: u4DPDG0_WF1_Prim = 0x%x \n", TxDPDResult.u4DPDG0_WF1_Prim));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF2]: u4DPDG0_WF2_Prim = 0x%x \n", TxDPDResult.u4DPDG0_WF2_Prim));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF2]: u4DPDG0_WF2_Sec = 0x%x \n", TxDPDResult.u4DPDG0_WF2_Sec));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF3]: u4DPDG0_WF3_Prim = 0x%x \n", TxDPDResult.u4DPDG0_WF3_Prim));
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("[WF3]: u4DPDG0_WF3_Sec = 0x%x \n", TxDPDResult.u4DPDG0_WF3_Sec));

	for(j=0;j<16;j++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF0]: ucDPDLUTEntry_WF0_B0_6[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF0_B0_6[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF0]: ucDPDLUTEntry_WF0_B16_23[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF0_B16_23[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF1]: ucDPDLUTEntry_WF1_B0_6[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF1_B0_6[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF1]: ucDPDLUTEntry_WF1_B16_23[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF1_B16_23[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2]: ucDPDLUTEntry_WF2_B0_6[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF2_B0_6[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2]: ucDPDLUTEntry_WF2_B16_23[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF2_B16_23[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2]: ucDPDLUTEntry_WF2_B8_14[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF2_B8_14[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2]: ucDPDLUTEntry_WF2_B24_31[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF2_B24_31[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3]: ucDPDLUTEntry_WF3_B0_6[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF3_B0_6[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3]: ucDPDLUTEntry_WF3_B16_23[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF3_B16_23[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3]: ucDPDLUTEntry_WF3_B8_14[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF3_B8_14[j]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3]: ucDPDLUTEntry_WF3_B24_31[%d] = 0x%x \n",j,TxDPDResult.ucDPDLUTEntry_WF3_B24_31[j]));
	}	
	
}


void ShowDCOCData(RTMP_ADAPTER *pAd, RXDCOC_RESULT_T RxDcocResult)
{
	UINT	i=0;
	
	if(pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
		("%s : Currently not in FLASH or BIN MODE,return. \n", __FUNCTION__));
		return;
	}
	
	for(i=0;i<4;i++)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF0 SX0]: ucDCOCTBL_I_WF0_SX0_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF0_SX0_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF0_SX0_LNA[i]));	
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF0 SX2]: ucDCOCTBL_I_WF0_SX2_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF0_SX2_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF0_SX2_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF0_SX2_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF1 SX0]: ucDCOCTBL_I_WF1_SX0_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF1_SX0_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF1_SX0_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF1_SX0_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF1 SX2]: ucDCOCTBL_I_WF1_SX2_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF1_SX2_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF1_SX2_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF1_SX2_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2 SX0]: ucDCOCTBL_I_WF2_SX0_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF2_SX0_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF2_SX0_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF2_SX0_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF2 SX2]: ucDCOCTBL_I_WF2_SX2_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF2_SX2_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF2_SX2_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF2_SX2_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3 SX0]: ucDCOCTBL_I_WF3_SX0_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF3_SX0_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF3_SX0_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF3_SX0_LNA[i]));
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("[WF3 SX2]: ucDCOCTBL_I_WF3_SX2_LNA[%d] = 0x%x \tucDCOCTBL_Q_WF3_SX2_LNA[%d] = 0x%x \n"
			, i, RxDcocResult.ucDCOCTBL_I_WF3_SX2_LNA[i]
			,i, RxDcocResult.ucDCOCTBL_Q_WF3_SX2_LNA[i]));
	}
}

BOOLEAN mt7615_dpd_check_illegal(RTMP_ADAPTER *pAd,MT_SWITCH_CHANNEL_CFG SwChCfg, UINT16 BW160Central)
{
	UINT16 CentralFreq = 0;
	UINT8  i = 0;
	BOOLEAN ChannelIsIllegal = FALSE;

	if(SwChCfg.Bw == BW_8080 || SwChCfg.Bw == BW_160)
		CentralFreq = BW160Central * 5 + 5000;
	else
		CentralFreq = SwChCfg.CentralChannel * 5 + 5000;
		
	if(SwChCfg.Bw == BW_20)
	{		
		for(i=0;i<DPD_ALL_SIZE;i++)
		{
			if(CentralFreq == DPD_AllFreq[i])
				break;
		}
		if(i == DPD_ALL_SIZE)
			ChannelIsIllegal = TRUE;
	}
	else if(SwChCfg.Bw == BW_40)
	{
		for(i=0;i<K_A40_SIZE;i++)
		{
			if(CentralFreq == K_A40Freq[i])
				break;
		}
		if(i == K_A40_SIZE)
			ChannelIsIllegal = TRUE;
	}
	else if(SwChCfg.Bw == BW_80 || SwChCfg.Bw == BW_8080)
	{
		for(i=0;i<K_A80_SIZE;i++)
		{
			if(CentralFreq == K_A80Freq[i])
				break;
		}
		if(i == K_A80_SIZE)
			ChannelIsIllegal = TRUE;
	}
	else if(SwChCfg.Bw == BW_160)
	{
		if(BW160Central == 199)		
			ChannelIsIllegal = TRUE;
	}	

	if(ChannelIsIllegal)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s : non-IEEE CH, ONLINE CAL, FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
				, __FUNCTION__,CentralFreq));
	}
	return ChannelIsIllegal;
}

void mt7615_apply_dpd(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg, UINT16 BW160Central, BOOLEAN bSecBW80)
{
	UINT8 			i = 0;
	UINT8 			Band = 0;
	UINT16 			CentralFreq = 0;
	ULONG 			Offset = 0;	
	TXDPD_RESULT_T  TxDPDResult;
	BOOLEAN 		toCR = TRUE;	

	if(pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s : Currently not in FLASH or BIN MODE,return. \n", __FUNCTION__));
		return;
	}

	if(SwChCfg.CentralChannel == 14)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s() : CH 14 don't need DPD , return!!! \n",__FUNCTION__));
		return;
	}
	else if(SwChCfg.CentralChannel < 14)  /* 2G */
	{
		Band = GBAND;
		
		if(SwChCfg.CentralChannel >= 1 && SwChCfg.CentralChannel <= 4)
			CentralFreq = 2422;
		else if(SwChCfg.CentralChannel >= 5 && SwChCfg.CentralChannel <= 9)
			CentralFreq = 2442;
		else if(SwChCfg.CentralChannel >= 10 && SwChCfg.CentralChannel <= 13)
			CentralFreq = 2462;		
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s() : can't find cent freq for CH %d , should not happen!!! \n",
					__FUNCTION__, SwChCfg.CentralChannel));		
	}
	else							 /* 5G */
	{
		Band = ABAND;

		/*
		* by the rule DE suggests,
		* 1.  BW20 directly apply , illegal channel online cal.
		* 2.  BW40/80/160 add center frequency by 10 MHz to find a nearest calibrated BW20 CH
		* 3.  if center freq + 10MHz = different group , then use center freq -10 MHz to apply
		*/	

		if(mt7615_dpd_check_illegal(pAd,SwChCfg,BW160Central)==TRUE)
		{			
			MtCmdGetTXDPDCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&TxDPDResult);
			return;
		}
		
		if(SwChCfg.Bw == BW_20)
		{
			CentralFreq = SwChCfg.CentralChannel * 5 + 5000;
		}
		else if(SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080)
		{
			UINT32 Central = BW160Central * 5 + 5000;
			UINT32 CentralAdd10M = (BW160Central+2) * 5 + 5000;
			if(ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralAdd10M))
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("==== Different Group Central %d @ group %d Central+10 @ group %d !! \n"
				, Central,ChannelFreqToGroup(Central),ChannelFreqToGroup(CentralAdd10M)));

				CentralFreq = (BW160Central-2) * 5 + 5000;
			}
			else
			{
				CentralFreq = (BW160Central+2) * 5 + 5000;
			}
		}
		else
		{
			UINT32 Central = SwChCfg.CentralChannel * 5 + 5000;
			UINT32 CentralAdd10M = (SwChCfg.CentralChannel+2) * 5 + 5000;
			if(ChannelFreqToGroup(Central) != ChannelFreqToGroup(CentralAdd10M))
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
				("==== Different Group Central %d @ group %d Central+10 @ group %d !! \n"
				,Central,ChannelFreqToGroup(Central),ChannelFreqToGroup(CentralAdd10M)));

				CentralFreq = (SwChCfg.CentralChannel-2) * 5 + 5000;
			}
			else
			{
				CentralFreq = (SwChCfg.CentralChannel+2) * 5 + 5000;
			}
		}
		
	}
	
	/* Find offset base on CentralFreq */
	for(i=0;i<DPD_ALL_SIZE;i++)
	{
		if(DPD_AllFreq[i] == CentralFreq)
			break;
	}
	if(i == DPD_ALL_SIZE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s : UNEXPECTED ONLINE CAL, FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
				, __FUNCTION__,CentralFreq));
		/* send command to tell FW do online K */
		MtCmdGetTXDPDCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&TxDPDResult);		
		return;
	}

	if(i < TXDPD_PART1_LIMIT)
	{
		Offset = i * TXDPD_SIZE;	
		memcpy(&TxDPDResult.u4DPDG0_WF0_Prim,pAd->CalDPDAPart1Image + Offset, TXDPD_SIZE);
	}
	else
	{
		Offset = (i-TXDPD_PART1_LIMIT) * TXDPD_SIZE;
		memcpy(&TxDPDResult.u4DPDG0_WF0_Prim,pAd->CalDPDAPart2GImage + Offset, TXDPD_SIZE);
	}

	if(SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s() : reload 160 Central CH [%d] BW [%d] from cetral freq [%d] i[%d] offset [%x] \n",
				__FUNCTION__, BW160Central,SwChCfg.Bw,CentralFreq,i, DPDPART1_OFFSET + i * TXDPD_SIZE));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s() : reload Central CH [%d] BW [%d] from cetral freq [%d] i[%d] offset [%x] \n",
				__FUNCTION__, SwChCfg.CentralChannel,SwChCfg.Bw
				,CentralFreq,i,DPDPART1_OFFSET + i * TXDPD_SIZE));
	}
	
	ShowDPDData(pAd,TxDPDResult);
	MtCmdGetTXDPDCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,FALSE,&TxDPDResult);	
}
//!Levarage form MP1.0 CL#170108
RXDCOC_RESULT_T RxDcocResult;
void mt7615_apply_dcoc(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg, UINT16 BW160Central, BOOLEAN bSecBW80)
{
	UINT8 			i = 0;
	UINT8 			Band = 0;
	UINT16 			CentralFreq = 0;
	ULONG 			Offset = 0;	
	BOOLEAN 		toCR = TRUE;

	if(pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s : Currently not in FLASH or BIN MODE,return. \n", __FUNCTION__));
		return;
	}
//! Levarage from MP1.0 CL 170210
	os_zero_mem(&RxDcocResult, sizeof(RXDCOC_RESULT_T));
	
	/*
	  * 11j TODO - 
	  * currently SwChCfg.Channel_Band is always 0 , can't judge 11j channels
	  * should add code below to convert to correct frequency if SwChCfg.Channel_Band is corrected.
	  */
	if(SwChCfg.CentralChannel <= 14)  /* 2G */
	{
		Band = GBAND;
		
		if(SwChCfg.CentralChannel >= 1 && SwChCfg.CentralChannel <= 3)
			CentralFreq = 2417;
		else if(SwChCfg.CentralChannel >= 4 && SwChCfg.CentralChannel <= 6)
			CentralFreq = 2432;
		else if(SwChCfg.CentralChannel >= 7 && SwChCfg.CentralChannel <= 9)
			CentralFreq = 2447;
		else if(SwChCfg.CentralChannel >= 10 && SwChCfg.CentralChannel <= 14)
			CentralFreq = 2467;
		else
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
			("%s() : can't find cent freq for CH %d , should not happen!!! \n",
					__FUNCTION__, SwChCfg.CentralChannel));		
	}
	else							 /* 5G */
	{
		Band = ABAND;
		
		CentralFreq = SwChCfg.CentralChannel * 5 + 5000;

		if(SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080)
		{
			CentralFreq = BW160Central * 5 + 5000;
		}
		else if(SwChCfg.Bw == BW_20 && SwChCfg.CentralChannel == 161)
		{
			CentralFreq = 5805;
		}
		else if(SwChCfg.Bw == BW_20)
		{
			/* find nearest BW40 central to apply */
			for(i=0;i<K_A40_SIZE;i++)
			{
				UINT delta = (CentralFreq >= K_A40Freq[i])?(CentralFreq-K_A40Freq[i]):(K_A40Freq[i]-CentralFreq);
				
				if(delta <= 10)
				{
					CentralFreq = K_A40Freq[i];
					break;
				}
			}
			if(i == K_A40_SIZE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s : UNEXPECTED. FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__,CentralFreq));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&RxDcocResult);				
				return;
			}
		}
		else if(SwChCfg.Bw == BW_40)
		{
			/* prevent illegal channel */
			for(i=0;i<K_A40_SIZE;i++)
			{
				if(CentralFreq == K_A40Freq[i])
					break;				
			}
			if(i == K_A40_SIZE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s : UNEXPECTED. FREQ[%d] @BW[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__,CentralFreq,BW_40));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&RxDcocResult);				
				return;
			}
		}
		else if(SwChCfg.Bw == BW_80)
		{
			/* prevent illegal channel */
			for(i=0;i<K_A80_SIZE;i++)
			{
				if(CentralFreq == K_A80Freq[i])
					break;				
			}
			if(i == K_A80_SIZE)
			{
				MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
					("%s : UNEXPECTED. FREQ[%d] @BW[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
					, __FUNCTION__,CentralFreq,BW_80));
				/* send command to tell FW do online K */
				MtCmdGetRXDCOCCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&RxDcocResult);				
				return;
			}
		}
	}

	/* Find offset base on CentralFreq */
	for(i=0;i<K_ALL_SIZE;i++)
	{
		if(K_AllFreq[i] == CentralFreq)
			break;
	}

	if(i == K_ALL_SIZE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s : UNEXPECTED. FREQ[%d] CANT FIND LEGAL CHANNEL TO APPLY, PLEASE CHECK!! \n"
				, __FUNCTION__,CentralFreq));
		/* send command to tell FW do online K */
		MtCmdGetRXDCOCCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,TRUE,&RxDcocResult);
		return;
	}

	Offset = i * RXDCOC_SIZE;	

	if(SwChCfg.Bw == BW_160 || SwChCfg.Bw == BW_8080)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s() : reload 160 Central CH [%d] BW [%d] from cetral freq [%d]  offset [%lx] \n",
				__FUNCTION__, BW160Central,SwChCfg.Bw,CentralFreq, DCOC_FLASH_OFFSET + Offset));
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, 
				("%s() : reload Central CH [%d] BW [%d] from cetral freq [%d]  offset [%lx] \n",
				__FUNCTION__, SwChCfg.CentralChannel,SwChCfg.Bw,CentralFreq, DCOC_FLASH_OFFSET + Offset));
	}

	memcpy(&RxDcocResult.ucDCOCTBL_I_WF0_SX0_LNA[0],pAd->CalDCOCImage + Offset, RXDCOC_SIZE);
//! Levarage from MP1.0 CL 170210
	//ShowDCOCData(pAd,RxDcocResult);
	MtCmdGetRXDCOCCalResult(pAd,toCR,CentralFreq,SwChCfg.Bw,Band,bSecBW80,FALSE,&RxDcocResult);	
}

static BOOLEAN find_both_central_for_bw160(MT_SWITCH_CHANNEL_CFG SwChCfg, UCHAR *CentPrim80, UCHAR *CentSec80)
{
	BOOLEAN found = FALSE;
	switch(SwChCfg.CentralChannel)
	{
	case 50:		
	case 82:
	case 114:
	case 163:
		if(SwChCfg.ControlChannel < SwChCfg.CentralChannel)
		{
			*CentPrim80 = SwChCfg.CentralChannel - 8;
			*CentSec80 = SwChCfg.CentralChannel + 8;
		}
		else
		{
			*CentPrim80 = SwChCfg.CentralChannel + 8;
			*CentSec80 = SwChCfg.CentralChannel - 8;
		}
		found = TRUE;
		break;
	default:
		*CentPrim80 = 199;
		*CentSec80 = 199;
		found = FALSE;
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s() : ERROR!!!! unknown bw160 central %d !!!! shall do online K\n"
			,__FUNCTION__,SwChCfg.CentralChannel));			
	}

	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s() : ControlChannel [%d], CentralChannel [%d]  => PrimCentral [%d] , SecCentral [%d]\n"
			,__FUNCTION__,SwChCfg.ControlChannel,SwChCfg.CentralChannel,*CentPrim80,*CentSec80));
	return found;
}
void mt7615_apply_cal_data(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
	USHORT doCal1 = 0;

	if(pAd->E2pAccessMode != E2P_FLASH_MODE && pAd->E2pAccessMode != E2P_BIN_MODE)
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
		("%s : Currently not in FLASH or BIN MODE,return. \n", __FUNCTION__));
		return;
	}

#ifdef RTMP_FLASH_SUPPORT
	if(pAd->E2pAccessMode == E2P_FLASH_MODE)
		rtmp_ee_flash_read(pAd, 0x52, &doCal1);	
#endif
	if(pAd->E2pAccessMode == E2P_BIN_MODE)
		rtmp_ee_bin_read16(pAd, 0x52, &doCal1);

	

	if(/* pAd->bDCOCReloaded && */ (doCal1 & (1 << 1)) != 0) /* 0x52 bit 1 for Reload RXDCOC on/off */
	{	
		UCHAR CentPrim80=0,CentSec80=0;
		if(SwChCfg.Bw == BW_160)
		{			
			find_both_central_for_bw160(SwChCfg, &CentPrim80, &CentSec80);
			mt7615_apply_dcoc(pAd,SwChCfg,CentPrim80,FALSE);
			mt7615_apply_dcoc(pAd,SwChCfg,CentSec80,TRUE);			
		}
		else if(SwChCfg.Bw == BW_8080)
		{
			UINT abs_cent1 = ABS(SwChCfg.ControlChannel,SwChCfg.CentralChannel);
			UINT abs_cent2 = ABS(SwChCfg.ControlChannel,SwChCfg.ControlChannel2);		
			
			if(abs_cent1 < abs_cent2)
			{	/* prim 80 is CentralChannel */
				CentPrim80 = SwChCfg.CentralChannel;
				CentSec80 = SwChCfg.ControlChannel2;
			}
			else
			{	/* prim 80 is ControlChannel2 */			
				CentPrim80 = SwChCfg.ControlChannel2;
				CentSec80 = SwChCfg.CentralChannel;			
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("###### BW_8080 ControlCH %d ControlChannel2 %d  CentralChannel %d  [ABS %d , %d] => prim80 [%d] sec80 [%d] ############\n"
			,SwChCfg.ControlChannel,SwChCfg.ControlChannel2,SwChCfg.CentralChannel,
			abs_cent1,abs_cent2,CentPrim80,CentSec80));
			
			mt7615_apply_dcoc(pAd,SwChCfg,CentPrim80,FALSE);
			mt7615_apply_dcoc(pAd,SwChCfg,CentSec80,TRUE);			
		}
		else
		{
			mt7615_apply_dcoc(pAd,SwChCfg,0,FALSE);
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s() : eeprom 0x52 bit 1 is 0, do runtime cal , skip RX reload\n",__FUNCTION__));
	}

	if(/* pAd->bDPDReloaded && */ (doCal1 & (0x1)) != 0) /* 0x52 bit 0 for Reload TXDPD on/off */
	{
		UCHAR CentPrim80=0,CentSec80=0;
		if(SwChCfg.Bw == BW_160)
		{			
			find_both_central_for_bw160(SwChCfg, &CentPrim80, &CentSec80);
			mt7615_apply_dpd(pAd,SwChCfg,CentPrim80,FALSE);
			mt7615_apply_dpd(pAd,SwChCfg,CentSec80,TRUE);
		}
		else if(SwChCfg.Bw == BW_8080)
		{
			UINT abs_cent1 = ABS(SwChCfg.ControlChannel,SwChCfg.CentralChannel);
			UINT abs_cent2 = ABS(SwChCfg.ControlChannel,SwChCfg.ControlChannel2);		
			
			if(abs_cent1 < abs_cent2)
			{	/* prim 80 is CentralChannel */
				CentPrim80 = SwChCfg.CentralChannel;
				CentSec80 = SwChCfg.ControlChannel2;
			}
			else
			{	/* prim 80 is ControlChannel2 */			
				CentPrim80 = SwChCfg.ControlChannel2;
				CentSec80 = SwChCfg.CentralChannel;			
			}
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("###### BW_8080 ControlCH %d ControlChannel2 %d  CentralChannel %d  [ABS %d , %d] => prim80 [%d] sec80 [%d] ############\n"
			,SwChCfg.ControlChannel,SwChCfg.ControlChannel2,SwChCfg.CentralChannel,
			abs_cent1,abs_cent2,CentPrim80,CentSec80));
			
			mt7615_apply_dpd(pAd,SwChCfg,CentPrim80,FALSE);
			mt7615_apply_dpd(pAd,SwChCfg,CentSec80,TRUE);			
		}
		else
		{
			mt7615_apply_dpd(pAd,SwChCfg,0,FALSE);
		}
	}
	else
	{
		MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE, 
			("%s() : eeprom 0x52 bit 0 is 0, do runtime cal , skip TX reload\n",__FUNCTION__));
	}
}
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

// TODO: Star
static void mt7615_switch_channel(RTMP_ADAPTER *pAd, MT_SWITCH_CHANNEL_CFG SwChCfg)
{
#ifdef PRE_CAL_TRX_SET1_SUPPORT
    mt7615_apply_cal_data(pAd, SwChCfg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */
   
    if(SwChCfg.Bw == BW_8080)
    {
         if((SwChCfg.ControlChannel2 - SwChCfg.CentralChannel) == 16 || (SwChCfg.CentralChannel - SwChCfg.ControlChannel2) == 16)
         {    
             SwChCfg.Bw = BW_160;
         }
    }		

#ifdef LINK_TEST_SUPPORT
    if (pAd->CommonCfg.LinkTestSupport)
    {
        /* Update Channel Band Info */
        if (SwChCfg.ControlChannel <= 14)
        {
            pAd->ucCmwChannelBand[SwChCfg.BandIdx] = CHANNEL_BAND_2G;
    
            MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(Link Test) 2G Channel Band !! \n"));
        }
        else
        {
            pAd->ucCmwChannelBand[SwChCfg.BandIdx] = CHANNEL_BAND_5G;
    
            MTWF_LOG(DBG_CAT_CMW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("(Link Test) 5G Channel Band !! \n"));
        }
    }    
#endif /* LINK_TEST_SUPPORT */

    MtCmdChannelSwitch(pAd,SwChCfg);

    if(!SwChCfg.bScan)
    {
        MtCmdSetTxRxPath(pAd,SwChCfg);
    }

    pAd->LatchRfRegs.Channel = SwChCfg.CentralChannel;

#ifdef LINK_TEST_SUPPORT
    if (pAd->CommonCfg.LinkTestSupport)
    {
        /* Update Link Up status (Disable) */
    	pAd->fgCmwLinkDone = FALSE;

		/* Update Apclient Link up Flag */
		pAd->fgApclientLinkUp = FALSE;
    
        /* clear Timeout Count */
        pAd->ucRxTestTimeoutCount = 0;
                
        /* Restore to 4R Config */
        MtCmdLinkTestRxCtrl(pAd, BITMAP_WF_ALL);
        
        /* Update 1R PD Detection Status */
        pAd->ucRxStreamState = FALSE;
    }
#endif /* LINK_TEST_SUPPORT */

#ifdef SINGLE_SKU_V2
#ifdef TXBF_SUPPORT
#ifdef MT_MAC
#ifdef CONFIG_ATE

    if(ATE_ON(pAd) == FALSE)
    {
        UINT8 i;
        UINT8 fg5Gband;
        UINT8 BfBoundTable[3];
        INT8  acTxPwrFccBfOnCase[10];
        INT8  acTxPwrFccBfOffCase[10];
            
        if (SwChCfg.Channel_Band == 0) // Not 802.11j
        {
            if (SwChCfg.ControlChannel <= 14)
            {
                fg5Gband = 0;
            }
            else
            {
                fg5Gband = 1;
            }
        }
        else
        {
            fg5Gband = 1;
        }

        /* obtain BF Backoff Table */
        mt_FillBFBackoff(pAd, SwChCfg.ControlChannel, fg5Gband, BfBoundTable);

        /*==============================================================================================*/
        /* Update BF ON Table */
        /*===============================================================================================*/

        for (i = 0; i < 3; i++)
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,("%s: BfBoundTable[%d]: 0x%x \n", __FUNCTION__, i, BfBoundTable[i]));

    	if (pAd->CommonCfg.dbdc_mode)
    	{
    		if (fg5Gband)
    			g_BFBackOffMode = pAd->dbdc_5G_tx_stream;
    		else
    			g_BFBackOffMode = pAd->dbdc_2G_tx_stream;
    	} else {
    		g_BFBackOffMode = pAd->Antenna.field.TxPath;
    	}

        /* Update BF Backoff value for different Tx Stream case */
        if (g_BFBackOffMode == 4)
        {
            acTxPwrFccBfOnCase[0] = BfBoundTable[0] + 12;   // Entry_1
            acTxPwrFccBfOnCase[1] = BfBoundTable[0] +  6;   // Entry_2
            acTxPwrFccBfOnCase[2] = BfBoundTable[0] + 12;   // Entry_3
            acTxPwrFccBfOnCase[3] = BfBoundTable[0] +  2;   // Entry_4
            acTxPwrFccBfOnCase[4] = BfBoundTable[0] +  8;   // Entry_5
            acTxPwrFccBfOnCase[5] = BfBoundTable[0] + 12;   // Entry_6
            acTxPwrFccBfOnCase[6] = BfBoundTable[0] +  0;   // Entry_7 (reference point)
            acTxPwrFccBfOnCase[7] = BfBoundTable[0] +  6;   // Entry_8
            acTxPwrFccBfOnCase[8] = BfBoundTable[0] +  9;   // Entry_9
            acTxPwrFccBfOnCase[9] = BfBoundTable[0] + 12;   // Entry_10
        }
        else if (g_BFBackOffMode == 3)
        {
            acTxPwrFccBfOnCase[0] = BfBoundTable[1] +  9;   // Entry_1
            acTxPwrFccBfOnCase[1] = BfBoundTable[1] +  3;   // Entry_2
            acTxPwrFccBfOnCase[2] = BfBoundTable[1] +  9;   // Entry_3
            acTxPwrFccBfOnCase[3] = BfBoundTable[1] +  0;   // Entry_4 (reference point)
            acTxPwrFccBfOnCase[4] = BfBoundTable[1] +  6;   // Entry_5
            acTxPwrFccBfOnCase[5] = BfBoundTable[1] +  9;   // Entry_6
            acTxPwrFccBfOnCase[6] = BfBoundTable[1] -  3;   // Entry_7 
            acTxPwrFccBfOnCase[7] = BfBoundTable[1] +  3;   // Entry_8
            acTxPwrFccBfOnCase[8] = BfBoundTable[1] +  7;   // Entry_9
            acTxPwrFccBfOnCase[9] = BfBoundTable[1] +  9;   // Entry_10
        }
        else if (g_BFBackOffMode == 2)
        {
            acTxPwrFccBfOnCase[0] = BfBoundTable[2] +  6;   // Entry_1
            acTxPwrFccBfOnCase[1] = BfBoundTable[2] +  0;   // Entry_2 (reference point)
            acTxPwrFccBfOnCase[2] = BfBoundTable[2] +  6;   // Entry_3
            acTxPwrFccBfOnCase[3] = BfBoundTable[2] -  4;   // Entry_4
            acTxPwrFccBfOnCase[4] = BfBoundTable[2] +  2;   // Entry_5
            acTxPwrFccBfOnCase[5] = BfBoundTable[2] +  6;   // Entry_6
            acTxPwrFccBfOnCase[6] = BfBoundTable[2] -  6;   // Entry_7
            acTxPwrFccBfOnCase[7] = BfBoundTable[2] +  0;   // Entry_8
            acTxPwrFccBfOnCase[8] = BfBoundTable[2] +  3;   // Entry_9
            acTxPwrFccBfOnCase[9] = BfBoundTable[2] +  6;   // Entry_10
        }
        else
        {
            /* Fill BF Backoff ON Table */
            os_fill_mem(acTxPwrFccBfOnCase, BF_GAIN_FINAL_SIZE, 0x3F);
        }
        
        /*===============================================================================================*/
        /* Update BF OFF Table (Default BF OFF Table is 0x3f to bypass the backoff mechanism) */
        /*===============================================================================================*/

        /* Fill BF Backoff OFF Table */
        os_fill_mem(acTxPwrFccBfOffCase, BF_GAIN_FINAL_SIZE, 0x3F);
        
        for (i = 0; i < BF_GAIN_FINAL_SIZE; i++)
            MTWF_LOG(DBG_CAT_FW, DBG_SUBCAT_ALL, DBG_LVL_INFO,("acTxPwrFccBfOnCase[%d]: 0x%x \n", i, acTxPwrFccBfOnCase[i]));

        CmdTxBfTxPwrBackOff(pAd, SwChCfg.BandIdx, acTxPwrFccBfOnCase, acTxPwrFccBfOffCase);
    }   

#endif /* CONFIG_ATE */
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
#endif /* SINGLE_SKU_V2 */
    
return;
}

#ifdef NEW_SET_RX_STREAM
static INT mt7615_set_RxStream(RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx)
{
    UINT32  path = 0;
    UINT    i;

#ifdef DBDC_MODE
    if (pAd->CommonCfg.dbdc_mode == TRUE)
    {
        if (StreamNums > 2) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                            ("%s():illegal StreamNums(%d) for BandIdx(%d)!DBDC max can allow 2SS\n",
                            __FUNCTION__, StreamNums, BandIdx));
            StreamNums = 2;
        }

        for (i = 0; i < StreamNums; i++)
            path |= 1 << i;

        if (BandIdx == 1)
            path = path << 2;
    }
    else
#endif /* DBDC_MODE */
    {
        if (StreamNums > 4) {
            MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR,
                            ("%s():illegal StreamNums(%d) \n",
                            __FUNCTION__, StreamNums));
            StreamNums = 4;
        }

        for (i = 0; i < StreamNums; i++)
            path |= 1 << i;
    }

	return MtCmdSetRxPath(pAd, path, BandIdx);

}
#endif

static inline VOID bufferModeFieldSet(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd,UINT16 addr)
{
	UINT32 i = pCmd->ucCount;
	pCmd->ucBinContent[i] = pAd->EEPROMImage[addr] ;
	pCmd->ucCount++;
}


static VOID mt7615_bufferModeEfuseFill(RTMP_ADAPTER *pAd,EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd)
{
	UINT16 i=0;
	pCmd->ucCount = 0;

	for(i=EFUSE_CONTENT_START; i<=EFUSE_CONTENT_END;i++)
	{
		bufferModeFieldSet(pAd,pCmd,i);
	}
	/*must minus last add*/
	pCmd->ucCount --;
}

#ifdef CAL_FREE_IC_SUPPORT
static UINT32 ICAL[] = {0x53, 0x54, 0x55, 0x56, 0x57, 0x5c, 0x5d, 0x62, 0x63, 0x68, 0x69,
			0x6e, 0x6f, 0x73, 0x74, 0x78, 0x79, 0x82, 0x83, 0x87,
			0x88, 0x8c, 0x8d, 0x91, 0x92, 0x96, 0x97, 0x9b, 0x9c, 0xa0, 0xa1,
			0xaa, 0xab, 0xaf, 0xb0, 0xb4, 0xb5, 0xb9, 0xba, 0xf4,
			0xf7, 0xff, 0x140, 0x141, 0x145, 0x146, 0x14a, 0x14b,
			0x154, 0x155, 0x159, 0x15a, 0x15e, 0x15f, 0x163, 0x164, 0x168, 0x169,
			0x16d, 0x16e, 0x172, 0x173, 0x17c, 0x17d, 0x181, 0x182,
			0x186, 0x187, 0x18b, 0x18c}; //check and merge
static UINT32 ICAL_NUM = (sizeof(ICAL)/sizeof(UINT32));
static UINT32 ICAL_JUST_MERGE[] = {0x118, 0x1b5, 0x1b6, 0x1b7, 0x3ac, 0x3ad, 0x3ae, 0x3af, 0x3b0, 0x3b1, 0x3b2}; //merge but nott check
static UINT32 ICAL_JUST_MERGE_NUM = (sizeof(ICAL_JUST_MERGE)/sizeof(UINT32));


inline static BOOLEAN check_valid(RTMP_ADAPTER *pAd, UINT16 Offset)
{
	UINT16 Value = 0;
	BOOLEAN NotValid;

	if((Offset % 2) != 0) {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset - 1, &Value);
		if (NotValid)
			return FALSE;
		if (((Value >> 8) & 0xff) == 0x00)
			return FALSE;
	}
	else {
		NotValid = rtmp_ee_efuse_read16(pAd, Offset, &Value);
		if (NotValid)
			return FALSE;
		if ((Value & 0xff) == 0x00)
			return FALSE;
	}
	return TRUE;
}

static BOOLEAN mt7615_is_cal_free_ic(RTMP_ADAPTER *pAd)
{
	UINT32 i;

	for (i = 0; i < ICAL_NUM; i++)
		if (check_valid(pAd, ICAL[i]) == FALSE)
			return FALSE;

	return TRUE;
}

inline static VOID cal_free_data_get_from_addr(RTMP_ADAPTER *ad, UINT16 Offset)
{
	UINT16 value;
	if((Offset % 2) != 0) {
		rtmp_ee_efuse_read16(ad, Offset - 1, &value);
		ad->EEPROMImage[Offset] = (value >> 8) & 0xFF;

	}
	else {
		rtmp_ee_efuse_read16(ad, Offset, &value);
		ad->EEPROMImage[Offset] =  value & 0xFF;
	}
}

static VOID mt7615_cal_free_data_get(RTMP_ADAPTER *ad)
{

	UINT32 i;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("%s\n", __FUNCTION__));

	for (i = 0; i < ICAL_NUM; i++)
		cal_free_data_get_from_addr(ad, ICAL[i]);
	for (i = 0; i < ICAL_JUST_MERGE_NUM; i++)
		cal_free_data_get_from_addr(ad, ICAL_JUST_MERGE[i]);
}

static BOOLEAN mt7615_check_is_cal_free_merge(RTMP_ADAPTER *pAd)
{
	UINT32 i;
	UINT16 Offset;
	UINT16 value;
	
	for (i = 0; i < ICAL_NUM; i++) {
		Offset = ICAL[i];
		if (Offset >= MAX_EEPROM_BUFFER_SIZE)
			return FALSE;
		if((Offset % 2) != 0) {
			rtmp_ee_efuse_read16(pAd, Offset - 1, &value);
			value = (value >> 8) & 0xFF;
		}
		else {
			rtmp_ee_efuse_read16(pAd, Offset, &value);
			value =  value & 0xFF;
		}
		if (pAd->EEPROMImage[Offset] != value)
			return FALSE;
			
	}
	for (i = 0; i < ICAL_JUST_MERGE_NUM; i++) {
		Offset = ICAL_JUST_MERGE[i];
		if (Offset >= MAX_EEPROM_BUFFER_SIZE)
			return FALSE;
		if((Offset % 2) != 0) {
			rtmp_ee_efuse_read16(pAd, Offset - 1, &value);
			value = (value >> 8) & 0xFF;
		}
		else {
			rtmp_ee_efuse_read16(pAd, Offset, &value);
			value =  value & 0xFF;
		}
		if (pAd->EEPROMImage[Offset] != value)
			return FALSE;
	}
	return TRUE;
}

#endif /* CAL_FREE_IC_SUPPORT */

#ifdef RF_LOCKDOWN

static UINT32 RFLOCK[] = { 0x03F, 0x040, 0x041, 0x056, 0x057, 0x058, 0x059, 0x05A, 0x05B, 0x05C, 0x05D, 0x05E, 0x05F, 0x060, 0x061, 0x062,
                           0x063, 0x064, 0x065, 0x066, 0x067, 0x068, 0x069, 0x06A, 0x06B, 0x06C, 0x06D, 0x06E, 0x06F, 0x070, 0x071, 0x072,
                           0x073, 0x074, 0x075, 0x076, 0x077, 0x078, 0x079, 0x07A, 0x07B, 0x07C, 0x07D, 0x07E, 0x07F, 0x080, 0x081, 0x082,
                           0x083, 0x084, 0x085, 0x086, 0x087, 0x088, 0x089, 0x08A, 0x08B, 0x08C, 0x08D, 0x08E, 0x08F, 0x090, 0x091, 0x092,
                           0x093, 0x094, 0x095, 0x096, 0x097, 0x098, 0x099, 0x09A, 0x09B, 0x09C, 0x09D, 0x09E, 0x09F, 0x0A0, 0x0A1, 0x0A2,
                           0x0A3, 0x0A4, 0x0A5, 0x0A6, 0x0A7, 0x0A8, 0x0A9, 0x0AA, 0x0AB, 0x0AC, 0x0AD, 0x0AE, 0x0AF, 0x0B0, 0x0B1, 0x0B2,
                           0x0B3, 0x0B4, 0x0B5, 0x0B6, 0x0B7, 0x0B8, 0x0B9, 0x0BA, 0x0BB, 0x0BC, 0x0BD, 0x0BE, 0x0BF, 0x0C0, 0x0C1, 0x0C2,
                           0x0C3, 0x0C4, 0x0C5, 0x0C6, 0x0C7, 0x0C8, 0x0C9, 0x0CA, 0x0CB, 0x0CC, 0x0CD, 0x0CE, 0x0CF, 0x0D0, 0x0D1, 0x0D2,
                           0x0D3, 0x0D4, 0x0D5, 0x0D6, 0x0D7, 0x0D8, 0x0D9, 0x0DA, 0x0DB, 0x0DC, 0x0DD, 0x0DE, 0x0DF, 0x0E0, 0x0E1, 0x0E2,
                           0x0E3, 0x0E4, 0x0E5, 0x0E6, 0x0E7, 0x0E8, 0x0E9, 0x0EA, 0x0EB, 0x0EC, 0x0ED, 0x0EE, 0x0EF, 0x0F0, 0x0F2, 0x0F3,
                           0x118, 0x11C, 0x11D, 0x11E, 0x11F, 0x12C, 0x12D, 0x140, 0x141, 0x142, 0x143, 0x144, 0x145, 0x146, 0x147, 0x148,
                           0x149, 0x14A, 0x14B, 0x14C, 0x14D, 0x14E, 0x14F, 0x150, 0x151, 0x152, 0x153, 0x154, 0x155, 0x156, 0x157, 0x158,
                           0x159, 0x15A, 0x15B, 0x15C, 0x15D, 0x15E, 0x15F, 0x160, 0x161, 0x162, 0x163, 0x164, 0x165, 0x166, 0x167, 0x168,
                           0x169, 0x16A, 0x16B, 0x16C, 0x16D, 0x16E, 0x16F, 0x170, 0x171, 0x172, 0x173, 0x174, 0x175, 0x176, 0x177, 0x178,
                           0x179, 0x17A, 0x17B, 0x17C, 0x17D, 0x17E, 0x17F, 0x180, 0x181, 0x182, 0x183, 0x184, 0x185, 0x186, 0x187, 0x188,
                           0x189, 0x18A, 0x18B, 0x18C, 0x18D, 0x18E, 0x18F, 0x190, 0x191, 0x192, 0x193, 0x194, 0x195, 0x196, 0x197, 0x198,
                           0x199, 0x19A, 0x19B, 0x19C, 0x19D, 0x19E, 0x19F, 0x1A0, 0x1A2, 0x1A3, 0x1A4, 0x1A5, 0x1A6, 0x1A7, 0x1A8, 0x1A9,
                           0x1AA, 0x1AB, 0x1AC, 0x1AD, 0x1AE, 0x1AF, 0x1B0, 0x1B1, 0x1B2, 0x3AC, 0x3AD, 0x3AE, 0x3AF};

static UINT32 RFLOCK_NUM = (sizeof(RFLOCK)/sizeof(UINT32));

static BOOLEAN mt7615_check_RF_lock_down(RTMP_ADAPTER *pAd)
{
    UCHAR block[EFUSE_BLOCK_SIZE]="";
    USHORT offset = 0;
    UINT isVaild = 0;
    BOOL RFlockDown;

    /* RF lock down column (0x12C) */
    offset = RF_LOCKDOWN_EEPROME_BLOCK_OFFSET;
    MtCmdEfuseAccessRead(pAd,offset,&block[0],&isVaild);

    if (((block[RF_LOCKDOWN_EEPROME_COLUMN_OFFSET] & RF_LOCKDOWN_EEPROME_BIT) >> RF_LOCKDOWN_EEPROME_BIT_OFFSET) == TRUE)
        RFlockDown = TRUE;
    else
        RFlockDown = FALSE;

    return RFlockDown;
}

static BOOLEAN mt7615_write_RF_lock_parameter(RTMP_ADAPTER *pAd, USHORT offset)
{
    BOOLEAN RFParaWrite;
    BOOLEAN fgRFlock = FALSE;
    UINT16  RFlock_index = 0;
#ifdef CAL_FREE_IC_SUPPORT
    BOOLEAN fgCalFree = FALSE;
    UINT16  CalFree_index = 0;
#endif /* CAL_FREE_IC_SUPPORT */

    /* Priority rule 1: RF lock paramter or not?    */
    /* Priority rule 2: Apply cal free or not?      */
    /* Priority rule 3: Cal free parameter or not?  */

    /* Check whether the offset exist in RF Lock Table or not */
    for (RFlock_index = pAd->RFlockTempIdx; RFlock_index < RFLOCK_NUM; RFlock_index++)
    {
        if (RFLOCK[RFlock_index] == offset)
        {
            fgRFlock = TRUE;
            pAd->RFlockTempIdx = RFlock_index;
            break;
        }
    }

#ifdef CAL_FREE_IC_SUPPORT
    /* Check whether the offset exist in Cal Free Table or not */
    for (CalFree_index = pAd->CalFreeTempIdx; CalFree_index < ICAL_NUM; CalFree_index++)
    {
        if (ICAL[CalFree_index] == offset)
        {
            fgCalFree = TRUE;
            pAd->CalFreeTempIdx = CalFree_index;
            break;
        }
    }

    /* Check whether the offset exist in Cal Free (Merge but not check) Table or not */
    for (CalFree_index = 0; CalFree_index < ICAL_JUST_MERGE_NUM; CalFree_index++)
    {
        if (ICAL_JUST_MERGE[CalFree_index] == offset)
        {
            fgCalFree = TRUE;
            break;
        }
    }
#endif /* CAL_FREE_IC_SUPPORT */

    /* Determine whether this offset needs to be written or not when RF lockdown */
    if (fgRFlock == TRUE)
    {
#ifdef CAL_FREE_IC_SUPPORT
        if (pAd->fgCalFreeApply == TRUE)
        {
            if (fgCalFree == TRUE)
            {
                RFParaWrite = FALSE;
            }
            else
            {
                RFParaWrite = TRUE;
            }
        }
        else
        {
            RFParaWrite = TRUE;
        }
#else
        RFParaWrite = TRUE;
#endif
    }
    else
    {
        RFParaWrite = FALSE;
    }

    return RFParaWrite;
}

static BOOLEAN mt7615_merge_RF_lock_parameter(RTMP_ADAPTER *pAd)
{
    UCHAR   block[EFUSE_BLOCK_SIZE]="";
    USHORT  length = pAd->chipCap.EEPROM_DEFAULT_BIN_SIZE;
    UCHAR   *ptr = pAd->EEPROMImage;
    UCHAR   index;
    USHORT  offset = 0;
    UINT    isVaild = 0;
    BOOL    WriteStatus;

    /* Merge RF parameters in Effuse to E2p buffer */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE)) 
    {
        /* Check Effuse Content block by block */
        for (offset = 0; offset < length; offset += EFUSE_BLOCK_SIZE)
        {
            MtCmdEfuseAccessRead(pAd,offset,&block[0],&isVaild);

            /* Check the Needed contents are different and update the E2p content by Effuse */
            for (index = 0; index < EFUSE_BLOCK_SIZE; index++)
            {
                /* Obtain the status of this E2p column need to write or not */
                WriteStatus = pAd->chipOps.write_RF_lock_parameter(pAd, offset + index);
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_INFO, ("Effuse[0x%04x]: Write(%d) \n", offset + index, WriteStatus));

                if ((block[index] != ptr[index]) && (WriteStatus == TRUE))
                    ptr[index] = block[index];
                else
                    continue;
                
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("index 0x%04x: ", offset + index));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("orignal E2p value=0x%04x, write value=0x%04x\n", ptr[index], block[index]));
            }

            ptr += EFUSE_BLOCK_SIZE;
        }
    }
    
    return TRUE;
}

static UCHAR mt7615_Read_Effuse_parameter(RTMP_ADAPTER *pAd, USHORT offset)
{
    UCHAR   block[EFUSE_BLOCK_SIZE]="";
    UINT    isVaild = 0;
    UINT16  BlockOffset, IndexOffset;
    UCHAR   RFUnlock = 0xFF;

    /* Obtain corresponding BlockOffset and IndexOffset for Effuse contents access */
    IndexOffset = offset % EFUSE_BLOCK_SIZE;
    BlockOffset = offset - IndexOffset;

    /* Merge RF parameters in Effuse to E2p buffer */
    if ((pAd->chipOps.check_RF_lock_down != NULL) && (pAd->chipOps.check_RF_lock_down(pAd) == TRUE))
    {
        /* Check Effuse Content block by block */
        MtCmdEfuseAccessRead(pAd, BlockOffset, &block[0], &isVaild);

        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Read Effuse[0x%x]: 0x%x ", offset, block[IndexOffset]));

        return block[IndexOffset];
    }
    
    return RFUnlock;
}

static BOOLEAN mt7615_Config_Effuse_Country(RTMP_ADAPTER *pAd)
{
    UCHAR   Buffer0, Buffer1;
    UCHAR   CountryCode[2];

    /* Read Effuse Content */
    if (pAd->chipOps.Read_Effuse_parameter != NULL)
    {
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        /* Country Region 2G */
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        Buffer0 = pAd->chipOps.Read_Effuse_parameter(pAd, COUNTRY_REGION_2G_EEPROME_OFFSET);

        /* Check the RF lock status */
        if (Buffer0 != 0xFF)
        {
            /* Check Validation bit for content */
            if (((Buffer0) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET) == TRUE)
                pAd->CommonCfg.CountryRegion = ((Buffer0) & (COUNTRY_REGION_CONTENT_MASK));
        }

        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        /* Country Region 5G */
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        
        Buffer1 = pAd->chipOps.Read_Effuse_parameter(pAd, COUNTRY_REGION_5G_EEPROME_OFFSET);

        /* Check the RF lock status */
        if (Buffer1 != 0xFF)
        {
            /* Check Validation bit for content */
            if (((Buffer1) & (COUNTRY_REGION_VALIDATION_MASK)) >> (COUNTRY_REGION_VALIDATION_OFFSET) == TRUE)
                pAd->CommonCfg.CountryRegionForABand = ((Buffer1) & (COUNTRY_REGION_CONTENT_MASK));
        }
        
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/
        /* Country Code */
        /* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

        CountryCode[0] = pAd->chipOps.Read_Effuse_parameter(pAd, COUNTRY_CODE_BYTE0_EEPROME_OFFSET);
        CountryCode[1] = pAd->chipOps.Read_Effuse_parameter(pAd, COUNTRY_CODE_BYTE1_EEPROME_OFFSET);

        /* Check the RF lock status */
        if ((CountryCode[0] != 0xFF) && (CountryCode[1] != 0xFF))
        {
            /* Check Validation for content */
            if ((CountryCode[0] != 0x00) && (CountryCode[1] != 0x00))
            {
                pAd->CommonCfg.CountryCode[0] = CountryCode[0];
                pAd->CommonCfg.CountryCode[1] = CountryCode[1];

                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[0]: 0x%x, %c ", pAd->CommonCfg.CountryCode[0], pAd->CommonCfg.CountryCode[0]));
                MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("pAd->CommonCfg.CountryCode[1]: 0x%x, %c ", pAd->CommonCfg.CountryCode[1], pAd->CommonCfg.CountryCode[1]));
            }
        }
    }
    
    return TRUE;
}
#endif /* RF_LOCKDOWN */

#ifdef CFG_SUPPORT_MU_MIMO
#ifdef MANUAL_MU
INT mu_update_profile_tb(RTMP_ADAPTER *pAd, INT profile_id, UCHAR wlan_id)
{

}

INT mu_update_grp_table(RTMP_ADAPTER *pAd, INT grp_id)
{

    return TRUE;
}


INT mu_update_cluster_tb(RTMP_ADAPTER *pAd, UCHAR c_id, UINT32 *m_ship, UINT32 *u_pos)
{
    UINT32 entry_base, mac_val, offset;

    ASSERT(c_id <=31);

    MAC_IO_READ32(pAd, MU_MUCR1, &mac_val);

    if (c_id < 16) {
        mac_val &= (~MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK);
    } else {
        mac_val |= MUCR1_CLUSTER_TAB_REMAP_CTRL_MASK;
    }
    MAC_IO_WRITE32(pAd, MU_MUCR1, mac_val);

    entry_base = MU_CLUSTER_TABLE_BASE  + (c_id & (~0x10)) * 24;

    /* update membership */
    MAC_IO_WRITE32(pAd, entry_base + 0x0, m_ship[0]);
    MAC_IO_WRITE32(pAd, entry_base + 0x4, m_ship[1]);

    /* Update user position */
    MAC_IO_WRITE32(pAd, entry_base + 0x8, u_pos[0]);
    MAC_IO_WRITE32(pAd, entry_base + 0xc, u_pos[1]);
    MAC_IO_WRITE32(pAd, entry_base + 0x10, u_pos[2]);
    MAC_IO_WRITE32(pAd, entry_base + 0x14, u_pos[3]);

    return TRUE;
}


INT mu_get_wlanId_ac_len(RTMP_ADAPTER *pAd, UINT32 wlan_id, UINT ac)
{
    return TRUE;
}


INT mu_get_mu_tx_retry_cnt(RTMP_ADAPTER *pAd)
{
    return TRUE;
}


INT mu_get_pfid_tx_stat(RTMP_ADAPTER *pAd)
{

}

INT mu_get_gpid_rate_per_stat(RTMP_ADAPTER *pAd)
{
    return TRUE;
}


INT mt7615_mu_init(RTMP_ADAPTER *pAd)
{
    UINT32 mac_val;

    /****************************************************************************
            MU Part
    ****************************************************************************/
    /* After power on initial setting,  AC legnth clear */
    MAC_IO_READ32(pAd, MU_MUCR4, &mac_val);
    mac_val = 0x1;
    MAC_IO_WRITE32(pAd, MU_MUCR4, mac_val); //820fe010= 0x0000_0001

    /* PFID table */
   MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x0, 0x1e000);  //820fe780= 0x0001_e000
   MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x4, 0x1e103);  //820fe784= 0x0001_e103
   MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0x8, 0x1e205);  //820fe788= 0x0001_e205
   MAC_IO_WRITE32(pAd, MU_PROFILE_TABLE_BASE + 0xc, 0x1e306);  //820fe78c= 0x0001_e306

   /* Cluster table */
   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x0, 0x0);  //820fe400= 0x0000_0000
   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x8, 0x0);  //820fe408= 0x0000_0000

   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x20, 0x2);  //820fe420= 0x0000_0002
   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x28, 0x0);  //820fe428= 0x0000_0000

   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x40, 0x2);  //820fe440= 0x0000_0002
   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x48, 0x4);  //820fe448= 0x0000_0004

   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x60, 0x0);  //820fe460= 0x0000_0000
   MAC_IO_WRITE32(pAd, MU_CLUSTER_TABLE_BASE + 0x68, 0x0);  //820fe468= 0x0000_0000

    /* Group rate table */
   MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x0, 0x4109);  //820ff000= 0x0000_4109
   MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x4, 0x99);  //820ff004= 0x0000_0099
   MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0x8, 0x800000f0);  //820ff008= 0x8000_00f0
   MAC_IO_WRITE32(pAd, MU_GRP_TABLE_RATE_MAP + 0xc, 0x99);  //820ff00c= 0x0000_0099

    /* SU Tx minimum setting */
   MAC_IO_WRITE32(pAd, MU_MUCR2, 0x10000001);  //820fe008= 0x1000_0001

   /* MU max group search entry = 1 group entry */
   MAC_IO_WRITE32(pAd, MU_MUCR1, 0x0);  //820fe004= 0x0000_0000

   /* MU enable */
   MAC_IO_READ32(pAd, MU_MUCR0, &mac_val);
   mac_val |= 1;
   MAC_IO_WRITE32(pAd, MU_MUCR0, 0x1);  //820fe000= 0x1000_0001

    /****************************************************************************
            M2M Part
    ****************************************************************************/
    /* Enable M2M MU temp mode */
    MAC_IO_READ32(pAd, RMAC_M2M_BAND_CTRL, &mac_val);
    mac_val |= (1<<16);
    MAC_IO_WRITE32(pAd, RMAC_M2M_BAND_CTRL, mac_val);


    /****************************************************************************
            AGG Part
    ****************************************************************************/
    /* 820f20e0[15] = 1 or 0 all need to be verified, because
        a). if primary is the fake peer, and peer will not ACK to us, cannot setup the TxOP
        b). Or can use CTS2Self to setup the TxOP
*/
    MAC_IO_READ32(pAd, AGG_MUCR, &mac_val);
    mac_val &= (~MUCR_PRIM_BAR_MASK);
    //mac_val |= (1 << MUCR_PRIM_BAR_BIT);
    MAC_IO_WRITE32(pAd, AGG_MUCR, mac_val);  //820fe000= 0x1000_0001

    return TRUE;
}
#endif /* MANUAL_MU */
#endif /* CFG_SUPPORT_MU_MIMO */

#ifndef  MAC_INIT_OFFLOAD
#ifdef CONFIG_FPGA_MODE
static VOID mt7615_init_mac_cr_for_fpga(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;


//++Check if anyone touch those CRs
	MAC_IO_READ32(pAd, LPON_WLANCKCR, &mac_val);
	printk("%s(): LPON_WLANCKCR=0x%x\n", __FUNCTION__, mac_val);
	MAC_IO_READ32(pAd, TMAC_ICR_BAND_0, &mac_val);
    printk("%s(): TMAC_ICR_BAND_0=0x%x\n", __FUNCTION__, mac_val);
	MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
	printk("%s(): TMAC_TRCR0=0x%x\n", __FUNCTION__, mac_val);
	MAC_IO_READ32(pAd, TMAC_RRCR, &mac_val);
	printk("%s(): TMAC_RRCR=0x%x\n", __FUNCTION__, mac_val);
//---Check if anyone touch those CRs

#ifndef PALLADIUM
	// enable MAC2MAC mode
	MAC_IO_READ32(pAd, RMAC_MISC, &mac_val);
	mac_val |= BIT18;
	MAC_IO_WRITE32(pAd, RMAC_MISC, mac_val);
#endif /* PALLADIUM */

#ifdef PALLADIUM
	/* MAC D0 2x / MAC D0 1x clock enable */
	MAC_IO_READ32(pAd, CFG_CCR, &mac_val);
	mac_val |= (BIT31 | BIT25);
	MAC_IO_WRITE32(pAd, CFG_CCR, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
    	("%s: MAC D0 2x 1x initial(val=%x)\n", __FUNCTION__, mac_val));
#endif /* PALLADIUM */

//+++Workaround for BCN transmission
	MAC_IO_READ32(pAd, CFG_DBDC_CTRL0, &mac_val);
	mac_val |= BIT21;
	MAC_IO_WRITE32(pAd, CFG_DBDC_CTRL0, mac_val);
//---Workaround for BCN transmission


	// TMAC DMA related setting for LMAC Tx DMA burst!
if (0){
	// TODO: shiang-MT7615, need verify this!
	MAC_IO_READ32(pAd, DMA_DCR1, &mac_val);
	mac_val |= (1<<14);
	MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val);
}

	// Band 0
	// TODO: shiang-Mt7615, remove these after FW is ready!
	
	MAC_IO_READ32(pAd, DMA_BN0RCFR1, &mac_val); // Rx Classify filter 1
	mac_val |= 0xc0000000;
	MAC_IO_WRITE32(pAd, DMA_BN0RCFR1, mac_val);

	MAC_IO_READ32(pAd, DMA_BN0VCFR0, &mac_val); // Vector Classify filter 0
	mac_val = 0x2000;
	MAC_IO_WRITE32(pAd, DMA_BN0VCFR0, mac_val);

	MAC_IO_READ32(pAd, DMA_BN0TCFR0, &mac_val); // TxS Classify filter 0
	mac_val = 0x8000;
	MAC_IO_WRITE32(pAd, DMA_BN0TCFR0, mac_val);

	MAC_IO_READ32(pAd, DMA_BN0TMCFR0, &mac_val); // TMR Classify filter 0, to HIF and RxRing 1
	mac_val = 0x2000;
	MAC_IO_WRITE32(pAd, DMA_BN0TMCFR0, mac_val);

	// Band 1
	MAC_IO_READ32(pAd, DMA_BN1RCFR0, &mac_val); // Rx Classify filter 0
	mac_val |= 0xc0000000;
	MAC_IO_WRITE32(pAd, DMA_BN1RCFR0, mac_val);

	MAC_IO_READ32(pAd, DMA_BN1RCFR1, &mac_val); // Rx Classify filter 1
	mac_val |= 0xc0000000;
	MAC_IO_WRITE32(pAd, DMA_BN1RCFR1, mac_val);

	MAC_IO_READ32(pAd, DMA_BN1VCFR0, &mac_val); // Vector Classify filter 0
	mac_val = 0x2000;
	MAC_IO_WRITE32(pAd, DMA_BN1VCFR0, mac_val);

	MAC_IO_READ32(pAd, DMA_BN1TCFR0, &mac_val); // TxS Classify filter 0
	mac_val = 0x8000;
	MAC_IO_WRITE32(pAd, DMA_BN1TCFR0, mac_val);

	MAC_IO_READ32(pAd, DMA_BN1TMCFR0, &mac_val); // TMR Classify filter 0, to HIF and RxRing 1
	mac_val = 0x2000;
	MAC_IO_WRITE32(pAd, DMA_BN1TMCFR0, mac_val);

//+++work-around for change T2R time for cannot receive Ack issue in FPGA mode
	MAC_IO_READ32(pAd, TMAC_RRCR, &mac_val);
	mac_val = 0;
	MAC_IO_WRITE32(pAd, TMAC_RRCR, mac_val);
//---work-around for change T2R time for cannot receive Ack issue in FPGA mode

//+++work-around for RTS/CTS bandwidth signaling failed issue in FPGA mode
/*
    M2M mode the CTS will not respond the RTS with bandwidth signaling request, and make
    sender keep sending the RTS, and reduce the throughput
*/
    MAC_IO_READ32(pAd, TMAC_TCR, &mac_val);
    mac_val &= ~((1<<12) | (1<<13) | (1<<14));
    MAC_IO_WRITE32(pAd, TMAC_TCR, mac_val);
    MAC_IO_READ32(pAd, TMAC_TCR1, &mac_val);
    mac_val &= ~((1<<12) | (1<<13) | (1<<14));
    MAC_IO_WRITE32(pAd, TMAC_TCR1, mac_val);
//---work-around for RTS/CTS bandwidth signaling failed issue in FPGA mode

//+++work-around for in-correct TxOP behavior of TMAC when run in M2M mode
/*
    M2M not support real PHY rate control, make Rx response too fast and create
    un-expected idle behavior, make when TxOP!=0, mostly LMAC is stay in idle state
    and make throughput redude
*/
    MAC_IO_READ32(pAd, TMAC_ATCR, &mac_val);
    mac_val &= ~(ATCR_TXV_TOUT_MASK);
    mac_val |= ((0x48 & ATCR_TXV_TOUT_MASK) << ATCR_TXV_TOUT_BIT);
    MAC_IO_WRITE32(pAd, TMAC_ATCR, mac_val);

    MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
    mac_val &= ~(TMAC_TR2T_CHK_MASK << TMAC_TR2T_CHK_BIT);
    mac_val |= ((0x6c & TMAC_TR2T_CHK_MASK) << TMAC_TR2T_CHK_BIT);
    MAC_IO_WRITE32(pAd, TMAC_TRCR0, mac_val);
    MAC_IO_READ32(pAd, TMAC_TRCR1, &mac_val);
    mac_val &= ~(TMAC_TR2T_CHK_MASK << TMAC_TR2T_CHK_BIT);
    mac_val |= ((0x6c & TMAC_TR2T_CHK_MASK) << TMAC_TR2T_CHK_BIT);
    MAC_IO_WRITE32(pAd, TMAC_TRCR1, mac_val);
//---work-around for in-correct TxOP behavior of TMAC when run in M2M mode

#ifdef CFG_SUPPORT_MU_MIMO
#ifdef MANUAL_MU
    mt7615_mu_init(pAd);
#endif /* MANUAL_MU */
#endif /* CFG_SUPPORT_MU_MIMO */
}
#endif /* CONFIG_FPGA_MODE */
#endif /* MAC_INIT_OFFLOAD */

#ifdef CONFIG_RALINK_MT7621
static VOID mt7615_hif_set_pcie_read_params(RTMP_ADAPTER *pAd)
{
	UINT32 reg_val;

/*
	PDMA setting
	0x50002088 [5:7] = 3'b101; // Set max_payload_ctrl as 4KB
	0x50003014 [0:2] = 3'b101; // Set k_cnt_max_payload_size as 4KB
	0x50003110 [0:2] = 3'b101; // Set k_conf_max_payload_size as 4KB
	0x50002088 [12:14] = 3'b101; // Set max_readreq_ctrl as 4KB
*/
	HIF_IO_READ32(pAd, PCI_CFG_DEVICE_CONTROL, &reg_val);

	reg_val &= PCI_CFG_MAX_PAYLOAD_SIZE_UMASK;
	reg_val &= PCI_CFG_MAX_READ_REQ_UMASK;

	reg_val |= (PCI_CFG_MAX_PAYLOAD_SIZE_4K | PCI_CFG_MAX_READ_REQ_4K);

	HIF_IO_WRITE32(pAd, PCI_CFG_DEVICE_CONTROL, reg_val);


	HIF_IO_READ32(pAd, PCI_K_CNT2, &reg_val);
	reg_val &= K_CNT_MAX_PAYLOAD_SIZE_UMASK;
	reg_val |= (K_CNT_MAX_PAYLOAD_SIZE_4K);
	HIF_IO_WRITE32(pAd, PCI_K_CNT2, reg_val);

	HIF_IO_READ32(pAd, PCI_K_CONF_FUNC0_4, &reg_val);
	reg_val &= K_CONF_MAX_PAYLOAD_SIZE_UMASK;
	reg_val |= (K_CONF_MAX_PAYLOAD_SIZE_4K);
	HIF_IO_WRITE32(pAd, PCI_K_CONF_FUNC0_4, reg_val);
/*
	PDMA setting
	0x50000260 [28:30] = 3'b000 
*/

	HIF_IO_READ32(pAd, MT_WPDMA_PAUSE_RX_Q_TH10, &reg_val);
	reg_val &= TX_PRE_ADDR_ALIGN_MODE_UMASK;
	HIF_IO_WRITE32(pAd, MT_WPDMA_PAUSE_RX_Q_TH10, reg_val);
}
#endif /* CONFIG_RALINK_MT7621 */

static VOID mt7615_init_mac_cr(RTMP_ADAPTER *pAd)
{
	UINT32 mac_val;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

#ifdef CONFIG_RALINK_MT7621
	mt7615_hif_set_pcie_read_params(pAd);
#endif /* CONFIG_RALINK_MT7621*/

#ifndef  MAC_INIT_OFFLOAD

#ifdef CONFIG_FPGA_MODE
    mt7615_init_mac_cr_for_fpga(pAd);
#endif /* CONFIG_FPGA_MODE */

	/* Set TxFreeEvent packet only go through CR4 */
	HW_IO_READ32(pAd, PLE_HIF_REPORT, &mac_val);
	mac_val |= 0x1;
	HW_IO_WRITE32(pAd, PLE_HIF_REPORT, mac_val);
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("%s(): Set TxRxEventPkt path 0x%0x = 0x%08x\n",
                        __FUNCTION__, PLE_HIF_REPORT, mac_val));

	/* Set PP Flow control */
	HW_IO_READ32(pAd, PP_PAGECTL_0, &mac_val);
	mac_val &= ~(PAGECTL_0_PSE_PG_CNT_MASK);
	mac_val |= 0x30;
	HW_IO_WRITE32(pAd, PP_PAGECTL_0, mac_val);

	HW_IO_READ32(pAd, PP_PAGECTL_1, &mac_val);
	mac_val &= ~(PAGECTL_1_PLE_PG_CNT_MASK);
	mac_val |= 0x10;
	HW_IO_WRITE32(pAd, PP_PAGECTL_1, mac_val);

	HW_IO_READ32(pAd, PP_PAGECTL_2, &mac_val);
	mac_val &= ~(PAGECTL_2_CUT_PG_CNT_MASK);
	mac_val |= 0x30;
	HW_IO_WRITE32(pAd, PP_PAGECTL_2, mac_val);

	/* Check PP CT setting */
	HW_IO_READ32(pAd, PP_RXCUTDISP, &mac_val);
    //mac_val |= 0x2;
    //RTMP_IO_WRITE32(pAd, mac_reg, mac_val);
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("%s(): Get CutThroughPathController CR 0x%0x = 0x%08x\n",
                        __FUNCTION__, PP_RXCUTDISP, mac_val));

#if defined(COMPOS_WIN) ||defined(COMPOS_TESTMODE_WIN)
#else
	/* TxS Setting */
	InitTxSTypeTable(pAd);
#endif

	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00,0);
#ifdef DBDC_MODE
	MtAsicSetTxSClassifyFilter(pAd, TXS2HOST, TXS2H_QID1, TXS2HOST_AGGNUMS, 0x00,1);
#endif /*DBDC_MODE*/
#endif /*MAC_INIT_OFFLOAD*/

	/* MAC D0 2x / MAC D0 1x clock enable */
	MAC_IO_READ32(pAd, CFG_CCR, &mac_val);
	mac_val |= (BIT31 | BIT25);
	MAC_IO_WRITE32(pAd, CFG_CCR, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                            ("%s: MAC D0 2x 1x initial(val=%x)\n", __FUNCTION__, mac_val));

#ifdef DBDC_MODE
	MAC_IO_READ32(pAd, CFG_CCR, &mac_val);
	mac_val |= (BIT30 | BIT24);
	MAC_IO_WRITE32(pAd, CFG_CCR, mac_val);
	MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_TRACE,
                            ("%s: MAC D1 2x 1x initial(val=%x)\n", __FUNCTION__, mac_val));
#endif /* DBDC_MODE */

    /*  Disable RX Header Translation */
    MAC_IO_READ32(pAd, DMA_DCR0, &mac_val);
    mac_val &= ~(DMA_DCR0_RX_HDR_TRANS_EN_BIT |
                            DMA_DCR0_RX_HDR_TRANS_MODE_BIT |
                            DMA_DCR0_RX_RM_VLAN_BIT | DMA_DCR0_RX_INS_VLAN_BIT |
                            DMA_DCR0_RX_HDR_TRANS_CHK_BSSID);
#ifdef HDR_TRANS_RX_SUPPORT
    if ((pAd->chipCap.asic_caps & fASIC_CAP_RX_HDR_TRANS) == fASIC_CAP_RX_HDR_TRANS)
    {
        UINT32 mac_val2;

        mac_val |= DMA_DCR0_RX_HDR_TRANS_EN_BIT;
        // TODO: UnifiedSW, take care about Windows for translation mode!
        //mac_val |= DMA_DCR0_RX_HDR_TRANS_MODE_BIT;
        mac_val |= DMA_DCR0_RX_HDR_TRANS_CHK_BSSID | DMA_DCR0_RX_RM_VLAN_BIT;

        MAC_IO_READ32(pAd, DMA_DCR1, &mac_val2);
        mac_val2 |= RHTR_AMS_VLAN_EN;
        MAC_IO_WRITE32(pAd, DMA_DCR1, mac_val2);
    }
#endif /* HDR_TRANS_RX_SUPPORT */
	MAC_IO_WRITE32(pAd, DMA_DCR0, mac_val);

    /* CCA Setting */
    MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
    mac_val &= ~CCA_SRC_SEL_MASK;
    mac_val |= CCA_SRC_SEL(0x2);
    mac_val &= ~CCA_SEC_SRC_SEL_MASK;
    mac_val |= CCA_SEC_SRC_SEL(0x0);
    MAC_IO_WRITE32(pAd, TMAC_TRCR0, mac_val);

    MAC_IO_READ32(pAd, TMAC_TRCR0, &mac_val);
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("%s(): TMAC_TRCR0=0x%x\n", __FUNCTION__, mac_val));

#ifdef DBDC_MODE
    MAC_IO_WRITE32(pAd, TMAC_TRCR1, mac_val);

    MAC_IO_READ32(pAd, TMAC_TRCR1, &mac_val);
    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
                        ("%s(): TMAC_TRCR1=0x%x\n", __FUNCTION__, mac_val));
#endif /*DBDC_MODE*/
    //---Add by shiang for MT7615 RFB ED issue

	/* Set BAR rate as 0FDM 6M default, remove after fw set */
	MAC_IO_WRITE32(pAd, AGG_ACR0, 0x04b00496);
	MAC_IO_WRITE32(pAd, AGG_ACR1, 0x04b00496);

	/*Add by Star for zero delimiter*/
	MAC_IO_READ32(pAd,TMAC_CTCR0,&mac_val);
	mac_val &= ~INS_DDLMT_REFTIME_MASK;
	mac_val |= INS_DDLMT_REFTIME(0x3f);
	mac_val |= DUMMY_DELIMIT_INSERTION;
	mac_val |= INS_DDLMT_DENSITY(3);
	MAC_IO_WRITE32(pAd,TMAC_CTCR0,mac_val);

	MAC_IO_READ32(pAd, DMA_BN0TCFR0, &mac_val);
	mac_val &= ~TXS_BAF;
	MAC_IO_WRITE32(pAd, DMA_BN0TCFR0, mac_val);

	/* Temporary setting for RTS */
	/*if no protect should enable for CTS-2-Self, WHQA_00025629*/
	if(MTK_REV_GTE(pAd, MT7615, MT7615E1) && MTK_REV_LT(pAd, MT7615, MT7615E3) && pAd->CommonCfg.dbdc_mode)
	{
		MAC_IO_WRITE32(pAd, AGG_PCR1, 0xfe0fffff);
	}else
	{
		MAC_IO_WRITE32(pAd, AGG_PCR1, 0x060003e8);
		MAC_IO_READ32(pAd, AGG_SCR, &mac_val);
		mac_val |= NLNAV_MID_PTEC_DIS;
		MAC_IO_WRITE32(pAd, AGG_SCR, mac_val);
	}
	/*Default disable rf low power beacon mode*/
	#define WIFI_SYS_PHY 0x10000
	#define RF_LOW_BEACON_BAND0 WIFI_SYS_PHY+0x1900
	#define RF_LOW_BEACON_BAND1 WIFI_SYS_PHY+0x1d00
	PHY_IO_READ32(pAd,RF_LOW_BEACON_BAND0,&mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd,RF_LOW_BEACON_BAND0,mac_val);
	PHY_IO_READ32(pAd,RF_LOW_BEACON_BAND1,&mac_val);
	mac_val &= ~(0x3 << 8);
	mac_val |= (0x2 << 8);
	PHY_IO_WRITE32(pAd,RF_LOW_BEACON_BAND1,mac_val);
}




#ifdef PALLADIUM
#define PALLADIUM_CM_ALL	0xff
INT MT7615_ChannelModelCfg(RTMP_ADAPTER *pAd, INT cm_id, BOOLEAN re_init, UCHAR band)
{
	UINT32 bbp_val, mask, bbp_cr, print_cr;
	UINT32 start_cm, end_cm, cm_num, cm_cr_offset;


	printk("%s(): Run in Palladium mode! Do Channel Model initialization....\n", __FUNCTION__);
	printk("\tcm_id=%d, re_init=%d\n", cm_id, re_init);

	if (cm_id == PALLADIUM_CM_ALL) {
		start_cm = 0;
		end_cm = 3;
	} else {
		start_cm = cm_id;
		end_cm = cm_id;
	}

	cm_cr_offset = 0x100;
	if (re_init == TRUE)
	{
		// step 0-0
		for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
		{
			bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
			print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
			PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
			printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
			mask = (CR_CM_TOP_RST_MASK << CR_CM_TOP_RST_BITS);
			bbp_val &= (~mask);
			PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
			printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);

			PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
			printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
			mask = (CR_CM_INTF_RST_MASK << CR_CM_INTF_RST_BITS);
			bbp_val &= (~mask);
			PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
			printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
		}

		// step 0-1
		for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
		{
			bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
			print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
			PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
			printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
			mask = CR_CM_LOAD_EN_MASK << CR_CM_LOAD_EN_BITS;
			bbp_val &= (~mask);
			PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
			printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);

			PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
			printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
			mask = CR_CM_INTF_EN_MASK << CR_CM_INTF_EN_BITS;
			bbp_val &= (~mask);
			PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
			printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
		}
	}


	//Step 1, start initialization
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_CM_TOP_RST_MASK << CR_CM_TOP_RST_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x1<<CR_CM_TOP_RST_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);

		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_CM_INTF_RST_MASK << CR_CM_INTF_RST_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x1<<CR_CM_INTF_RST_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	// Step 2
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		// AFE band, 2.4G or 5G
		bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_AFE_BAND_MASK << CR_AFE_BAND_BITS;
		bbp_val &= (~mask);
		if (band == BAND_24G)
			bbp_val |= (0x0<<CR_AFE_BAND_BITS);
		else if (band == BAND_5G)
			bbp_val |= (0x1<<CR_AFE_BAND_BITS);

		// Channel Rx DBM, -50dBm
		mask = CR_CH_RX_DBM_MASK << CR_CH_RX_DBM_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x738 << CR_CH_RX_DBM_BITS);

		//AFE AWGN Noise Enable
		mask = CR_AFE_AWGN_NOISE_EN_MASK << CR_AFE_AWGN_NOISE_EN_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x0<<CR_AFE_AWGN_NOISE_EN_BITS);

		// Channel type
		mask = CR_CH_TYPE_MASK << CR_CH_TYPE_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x0<<CR_CH_TYPE_BITS);

		// transpose mode enable
		mask = CR_TRANSPOSE_MODE_EN_MASK << CR_TRANSPOSE_MODE_EN_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x0<<CR_TRANSPOSE_MODE_EN_BITS);

		// Number of RX
		mask = CR_N_RX_MASK << CR_N_RX_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x4<<CR_N_RX_BITS);


		// Number of TX
		mask = CR_N_TX_MASK << CR_N_TX_BITS;
		bbp_val &= (~mask);
		bbp_val |= (0x4<<CR_N_TX_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	//Step 3, Configure channel ID
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		UINT32 seed;

		seed = 1<<(cm_num * 8);
		bbp_cr = CR_CM_CH_SEED_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_CH_ID_MASK << CR_CH_ID_BITS;
		bbp_val &= (~mask);
		bbp_val |= (seed<<CR_CH_ID_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	//Step 4, Configure AFE seed
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		UINT32 seed;

		seed = 1<<(cm_num * 8);
		bbp_cr = CR_CM_AFE_SEED_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_AFE_SEED_MASK << CR_AFE_SEED_BITS;
		bbp_val &= (~mask);
		bbp_val |= (seed<<CR_AFE_SEED_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	// Step 11, Set Noise level
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		bbp_cr = CR_CM_DC_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = CR_AFE_DC_NOISE_EN_MASK << CR_AFE_DC_NOISE_EN_BITS;
		bbp_val &= (~mask);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	// Step 13, take effect
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\tRead CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = (CR_CM_LOAD_EN_MASK << CR_CM_LOAD_EN_BITS) | (CR_CM_INTF_EN_MASK << CR_CM_INTF_EN_BITS);
		bbp_val &= (~mask);
		bbp_val |= (CR_CM_LOAD_EN_MASK << CR_CM_LOAD_EN_BITS) | (CR_CM_INTF_EN_MASK << CR_CM_INTF_EN_BITS);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\twrite CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	// Step 14, delay for a while
	RtmpusecDelay(1);

	// Step 15, pull LOAD_EN_BITS to low af
	for (cm_num = start_cm; cm_num <= end_cm; cm_num++)
	{
		bbp_cr = CR_CM_TOP_CTRL + cm_num * cm_cr_offset;
		print_cr = (bbp_cr - WF_PHY_BASE) + 0x82070000;
		PHY_IO_READ32(pAd, bbp_cr, &bbp_val);
		printk("\Read CR 0x%x = 0x%x\n", print_cr, bbp_val);
		mask = (CR_CM_LOAD_EN_MASK << CR_CM_LOAD_EN_BITS);
		bbp_val &= (~mask);
		PHY_IO_WRITE32(pAd, bbp_cr, bbp_val);
		printk("\write CR 0x%x = 0x%x\n", print_cr, bbp_val);
	}

	// Step 16, delay for a while
	RtmpusecDelay(100);

	// final dump all configurations
	{

	}
	return TRUE;
}

#define MAX_USER    4
#define MAX_CORR    4

INT16 highcorr_channel_real[MAX_USER][MAX_CORR] =
{
    {   -15136,     29632,      -9088,      9504    },      // User0HCR
    {   -13600,     30176,      -10048,     5632    },      // User1HCR
    {   -13696,     31936,      -7200,      1312    },      // User2HCR
    {   -8800,      32736,      -4928,      12640   },      // User3HCR
};

INT16 lowcorr_channel_real[MAX_USER][MAX_CORR] =
{
    {   -15264,     29856,      -9152,      9568    },      // User0LCR
    {   4544,       5920,       -5088,      -14816  },      // User1LCR
    {   4192,       13248,      6624,       -32736  },      // User2LCR
    {   24544,      16512,      16064,      14272  },      // User3LCR
};

INT16 highcorr_channel_imag[MAX_USER][MAX_CORR] =
{
    {   -10016,     12544,      -5056,      0       },      // User0HCI
    {   -4928,      12864,      -12064,     0       },      // User1HCI
    {   -11168,     13280,      -1344,      0       },      // User2HCI
    {   -6464,      13344,      -3328,      0       },      // User3HCI
};

INT16 lowcorr_channel_imag[MAX_USER][MAX_CORR] =
{
    {   -10080,     12640,      -5088,      0       },      // User0LCI
    {   19840,      2880,       -29728,     0       },      // User1LCI
    {   -6048,      4512,       14848,      0       },      // User2LCI
    {   13472,      4896,       6624,       0       },      // User3LCI
};



UINT32 bbp_performance_reg_pair[] = {
	/* CR */				/* mask */	/* bit */	/* 1R/2R */	/* 3R */		/* 4R */
	PHY_RXTD_58, 		0x1ff, 	9,		0x150, 		0x14c, 		0x14a,
	PHY_RXTD1_4, 		0x7, 	25,		0x0, 		0x1, 		0x0,
	PHY_RXTD1_1, 		0x3f,	12,		0x13,		0x18,		0x12,
	PHY_LTFSYNC_6,		0x7,		0,		0x4,			0x3,			0x2,
	PHY_RXTD_44,		0x3f,	22,		0x17,		0x19,		0x17,
	PHY_RXTD_56,		0x3f,	6,		0x23,		0x26,		0x22,
	PHY_RXTD_CCKPD_3,	0x3f,	24,		0x8,			0x8,			0x8,
	PHY_RXTD_CCKPD_3,	0xf,		20,		0x8,			0x8,			0x8,
	PHY_RXTD_CCKPD_3,	0xf,		16,		0x8,			0x8,			0x8,
	PHY_RXTD_CCKPD_3,	0x3f,	10,		0xe,			0xd,			0xc,
	PHY_RXTD_CCKPD_3,	0x1f,	5,		0x10,		0x10,		0x10,
	PHY_RXTD_CCKPD_3,	0x1f,	0,		0x10,		0x10,		0x10,
	PHY_RXTD_CCKPD_4,	0x3f,	24,		0x1c,		0x14,		0x08,
	PHY_RXTD_CCKPD_4,	0x3f,	18,		0x1c,		0x1c,		0x1c,
	PHY_RXTD_CCKPD_4,	0x3f,	12,		0x3f,		0x3f,		0x3f,
	PHY_RXTD_CCKPD_4,	0x3f,	6,		0x1c,		0x1c,		0x1c,
	PHY_RXTD_CCKPD_4,	0x1f,	1,		0x10,		0x10,		0x10,
	PHY_RXTD_CCKPD_7,	0x3f,	1,		0x1c,		0x1c,		0x1c,
	PHY_RXTD1_1,		0x1f,	6,		0x21,		0x28,		0x29,
	PHY_RXTD1_1,		0x1f,	0,		0x21,		0x33,		0x35,
	PHY_RXTD1_0,		0x1f,	6,		0x23,		0x30,		0x31,
	PHY_RXTD1_0,		0x1f,	0,		0x23,		0x35,		0x39,
};


UINT32 bbp_workaround_reg_pair[]={
	/* CR, mask, bit,	OFDM*/
	PHY_RXTD_BAND0_AGC_23_RX0,	0x3f,	14,	0x0c,
	PHY_RXTD_BAND0_AGC_23_RX1,	0x3f,	14,	0x0c,
	PHY_FSD_CTRL_1,					0x1,		0,	0x01,
	PHY_FSD_CTRL_1,					0x1,		1,	0x01,
	PHY_FSD_CTRL_1,					0x1,	15,	0x01,
	PHY_FSD_CTRL_1,					0x1,	23,	0x01,
	PHY_FSD_CTRL_1,					0x1,	31,	0x01,
	PHY_FSD_CTRL_1,					0x1f,	24,	0x1f,
	PHY_FSD_CTRL_1,					0x1f,	16,	0x1f,
	PHY_FSD_CTRL_1,					0x1f,	8,	0x1f,
	PHY_RXTD_CCKPD_4,				0x3f,	12,	0x0,
	PHY_RXTD_CCKPD_6,				0x3f,	12,	0x0,
	PHY_CTRL_TSSI_9,				0xffffffff,	0,	0x40800000,
	PHY_CTRL_WF1_TSSI_9,			0xffffffff,	0,	0x40800000,
	PHY_CTRL_WF2_TSSI_9,			0xffffffff,	0,	0x40800000,
	PHY_CTRL_WF3_TSSI_9,			0xffffffff,	0,	0x40800000,

	PHY_TX_BAND0_WF0_CR_TXFE_3,	0xf,	28,	0xf,
	PHY_TX_BAND0_WF1_CR_TXFE_3,	0xf,	28,	0xf,
	PHY_TX_BAND1_WF0_CR_TXFE_3,	0xf,	28,	0xf,
	PHY_TX_BAND1_WF1_CR_TXFE_3,	0xf,	28,	0xf,
	PHY_RXTD_43,					0x1,	31,	0x0,
};


UINT32 bbp_dot11v_setting[]={
	/* CR */					/* Mask */	/* bit */	/* Value */
	PHY_RXTD_RXFE_01_B0,	0x1,		2,		0x1,
	PHY_RXTD_RXFE_01_B1,	0x1,		2,		0x1,
};


UINT32 bbp_txbf_setting[]={
	/* CR */				/* Mask */	/* bit */ /* TxBF On*/		/* TxBF Off */
	PHY_PHYCK_CTRL,		0x7,	11,		0x7,				0x0,
	PHY_PHYCK_CTRL,		0x7,	27,		0x7,				0x0,
};
#endif /* PALLADIUM */


static VOID MT7615BBPInit(RTMP_ADAPTER *pAd)
{
	BOOLEAN isDBDC = FALSE, band_vld[2];
	INT idx, cbw[2] = {0};
	INT cent_ch[2] ={0}, prim_ch[2]={0}, prim_ch_idx[2] = {0};
	INT band[2]={0};
	INT txStream[2]={0};
	UCHAR use_bands;

	band_vld[0] = TRUE;
	cbw[0] = RF_BW_20;
	cent_ch[0] = 1;
	prim_ch[0] = 1;
	band[0] = BAND_24G;
	txStream[0] = 2;

#ifdef DOT11_VHT_AC
	prim_ch_idx[0] = vht_prim_ch_idx(cent_ch[0], prim_ch[0], cbw[0]);
#endif /* DOT11_VHT_AC */

	if (isDBDC) {
		band_vld[1] = TRUE;
		band[1] = BAND_5G;
		cbw[1] = RF_BW_20;
		cent_ch[1] = 36;
		prim_ch[1] = 36;
#ifdef DOT11_VHT_AC
		prim_ch_idx[1] = vht_prim_ch_idx(cent_ch[1], prim_ch[1], cbw[1]);
#endif /* DOT11_VHT_AC */
		txStream[1] = 2;

		use_bands = 2;
	} else {
		band_vld[1] = FALSE;
		use_bands = 1;
	}

printk("%s():BBP Initialization.....\n", __FUNCTION__);
for (idx = 0; idx < 2; idx++) {
	printk("\tBand %d: valid=%d, isDBDC=%d, Band=%d, CBW=%d, CentCh/PrimCh=%d/%d, prim_ch_idx=%d, txStream=%d\n",
			idx, band_vld[idx], isDBDC, band[idx], cbw[idx], cent_ch[idx], prim_ch[idx],
			prim_ch_idx[idx], txStream[idx]);
}

#ifdef PALLADIUM
	if (pAd->fpga_ctl.run_palladium)
	{
		UINT32 bbp_reg, bbp_val, mask, bits, val;
		BOOLEAN bTxBF = TRUE, bDot11v = FALSE;
                //UINT8 high_corr = 0;
                //INT cm_id = 0;

printk("%s(): Run in Palladium mode! Do BBP initialization....\n", __FUNCTION__);

		/* PLL divider */
		PHY_IO_READ32(pAd, PHY_PHYSYS_CTRL, &bbp_val);
		bbp_val &= (~(0x3));
		PHY_IO_WRITE32(pAd, PHY_PHYSYS_CTRL, bbp_val);

		/* DBDC mode */
		PHY_IO_READ32(pAd, PHY_PHYSYS_CTRL, &bbp_val);
		if (isDBDC == TRUE)
			bbp_val |= (1<<31);
		else
			bbp_val &= (~(1<<31));
		PHY_IO_WRITE32(pAd, PHY_PHYSYS_CTRL, bbp_val);


	printk("%s():isDBDC=%d, CBW[0]=%d,CBW[1]=%d\n", __FUNCTION__, isDBDC, cbw[0],cbw[1]);

		/*
			[29:28]CR_BAND0_BAND: G band =0; A band=1;
			[27:24]CR_BAND0_CBW: BW20=0, BW40=1, BW80=2, BW160NC=7, BW160C=15;
			[22:20]CR_BAND0_PRI_CH: BW20=0, BW40=0/1, BW80=0/1/2/3, BW160NC/C=0/1/2/3/4/5/6/7

		*/
		for (idx = 0; idx < use_bands; idx++)
		{
			if (idx == 0)
				bbp_reg = PHY_BAND0_PHY_CTRL_0;
			else
				bbp_reg = PHY_BAND1_PHY_CTRL_0;

			/* band */
			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~(0x3 << 28));
			if (band[idx] == BAND_24G)
				bbp_val |= (0x0<<28);
			else if (band[idx] == BAND_5G)
				bbp_val |= (0x1<<28);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);

			/* CBW */
			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~(0xf << 24));
			switch (cbw[idx])
			{
				case RF_BW_20:
					val = 0;
					break;
				case RF_BW_40:
					val = 1;
					break;
				case RF_BW_80:
					val = 2;
					break;
				case RF_BW_8080:
					val = 0x7;
					break;
				case RF_BW_160:
					val = 0xf;
					break;
			}
			bbp_val |= ((val & 0xf) << 24);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);

			/* Primary Channel */
			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~(0x7 << 24));
			bbp_val |= ((prim_ch_idx[idx] & 0x7) << 24);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);

			/*
				TX_FD1_ADDC
					Tx/Rx
					[30:30]
					[29:28]
			*/
			if (idx == 0) {
				PHY_IO_READ32(pAd, PHY_TXFD_1, &bbp_val);
				bbp_val &= (~(0xf<<28));
				bbp_val |= ((txStream[0] - 1) << 30);
				if (band_vld[1] == TRUE)
					bbp_val |= ((txStream[1] - 1) << 28);
				PHY_IO_WRITE32(pAd, PHY_TXFD_1, bbp_val);
			}

			/* CCK PD Enable */
			if (idx == 0)
				bbp_reg = PHY_RXTD_0;
			else
				bbp_reg = PHY_RXTD2_0;
			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~(0x1<<21));
			if (band[idx] == BAND_24G)
				bbp_val |= (1<< 21);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);
		}


		/* Performance setting */
		for (idx = 0; idx < (sizeof(bbp_performance_reg_pair)/sizeof(UINT32)); idx+=6) {
			bbp_reg = bbp_performance_reg_pair[idx];
			bits = bbp_performance_reg_pair[idx+2];
			mask = bbp_performance_reg_pair[idx+1] << bits;
			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~mask);
			switch (txStream[0])
			{
				case 1:
				case 2:
					bbp_val |= (bbp_performance_reg_pair[idx+3] << bits);
					break;
				case 3:
					bbp_val |= (bbp_performance_reg_pair[idx+4] << bits);
					break;
				case 4:
					bbp_val |= (bbp_performance_reg_pair[idx+5] << bits);
					break;
			}
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);
		}

		/* Work-around setting */
		for (idx = 0; idx < (sizeof(bbp_workaround_reg_pair)/sizeof(UINT32)); idx+=4) {
			bbp_reg = bbp_workaround_reg_pair[idx];
			bits = bbp_workaround_reg_pair[idx+2];
			mask = bbp_workaround_reg_pair[idx+1] << bits;

			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~mask);
			bbp_val |= (bbp_workaround_reg_pair[idx+3]<<bits);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);
		}


		/* 802.11 V setting if required */
		if (bDot11v == TRUE) {
			for (idx = 0; idx < (sizeof(bbp_dot11v_setting)/sizeof(UINT32)); idx+=4) {
				bbp_reg = bbp_dot11v_setting[idx];
				bits = bbp_dot11v_setting[idx+2];
				mask = bbp_dot11v_setting[idx+1] << bits;

				PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
				bbp_val &= (~mask);
				bbp_val |= (bbp_dot11v_setting[idx+3]<<bits);
				PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);
			}
		}

		/* TxBF setting if required */
		for (idx = 0; idx < (sizeof(bbp_txbf_setting)/sizeof(UINT32)); idx+=5) {
			bbp_reg = bbp_txbf_setting[idx];
			bits = bbp_txbf_setting[idx+2];
			mask = bbp_txbf_setting[idx+1] << bits;

			PHY_IO_READ32(pAd, bbp_reg, &bbp_val);
			bbp_val &= (~mask);
			if (bTxBF == TRUE)
				bbp_val |= (bbp_txbf_setting[idx+3]<<bits);
			else
				bbp_val |= (bbp_txbf_setting[idx+4]<<bits);
			PHY_IO_WRITE32(pAd, bbp_reg, bbp_val);
		}


		MT7615_ChannelModelCfg(pAd, 0, 0, band[0]);
		//MT7615_ChannelModelCfg(pAd, 0xff, 0, band[0]);


        //TODO: For MU-MIMO, Disable the line above.
        //MT7615_MuChannelModelCfg(pAd, cm_id, high_corr, re_init, band);
        //cm_id: speed bridge channel id for STA mode; reserved for AP mode
        //high_corr: 1 for high correlation channel model; 0 for low correlation channel model.
        //re_init: 1 for reinitialize; 0 for initialize.
        //band: 1 for 5 GHz; 2 for 2.4 GHz.
        //Enable below line with proper values.
        //RTMP_IO_READ32(pAd, (0x2708), &cm_id);
        //printk("channel id %x\n...\n",PHY_IO_READ32(pAd, (WF_PHY_BASE + 0x2708)));

        //cmd_id =4;
	}
#endif /* PALLADIUM */

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo \n", __FUNCTION__));

	return;
}


static void mt7615_init_rf_cr(RTMP_ADAPTER *ad)
{
	return;
}


int mt7615_read_chl_pwr(RTMP_ADAPTER *pAd)
{
	return TRUE;
}


/* Read power per rate */
void mt7615_get_tx_pwr_per_rate(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}


void mt7615_get_tx_pwr_info(RTMP_ADAPTER *pAd)
{
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s() todo \n", __FUNCTION__));
	return;
}


static void mt7615_antenna_default_reset(
	struct _RTMP_ADAPTER *pAd,
	EEPROM_ANTENNA_STRUC *pAntenna)
{
	USHORT value;
	pAntenna->word = 0;
	pAd->RfIcType = RFIC_7615;

	pAntenna->field.TxPath = (pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] >> 4) & 0x0F;
	pAntenna->field.RxPath = pAd->EEPROMDefaultValue[EEPROM_NIC_CFG1_OFFSET] & 0x0F;

	if (pAntenna->field.TxPath > pAd->chipCap.max_nss)
	{
		pAntenna->field.TxPath = pAd->chipCap.max_nss;
	}

	if (pAntenna->field.RxPath > pAd->chipCap.max_nss)
	{
		pAntenna->field.RxPath = pAd->chipCap.max_nss;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): TxPath = %d, RxPath = %d\n",
            __FUNCTION__, pAntenna->field.TxPath, pAntenna->field.RxPath));

	if (pAd->chipCap.max_nss == 4) {
		RT28xx_EEPROM_READ16(pAd, EEPROM_DBDC_ANTENNA_CFG_OFFSET, value);
		value &= 0xFF;
		pAd->dbdc_2G_rx_stream = (value & DBDC_2G_RX_MASK) >> DBDC_2G_RX_OFFSET;
		pAd->dbdc_2G_tx_stream = (value & DBDC_2G_TX_MASK) >> DBDC_2G_TX_OFFSET;
		pAd->dbdc_5G_rx_stream = (value & DBDC_5G_RX_MASK) >> DBDC_5G_RX_OFFSET;
		pAd->dbdc_5G_tx_stream = (value & DBDC_5G_TX_MASK) >> DBDC_5G_TX_OFFSET;

		if ((pAd->dbdc_2G_rx_stream == 0) || (pAd->dbdc_2G_rx_stream > 2)) {
			pAd->dbdc_2G_rx_stream = 2;
		}
		if ((pAd->dbdc_2G_tx_stream == 0) || (pAd->dbdc_2G_tx_stream > 2)) {
			pAd->dbdc_2G_tx_stream = 2;
		}
		if ((pAd->dbdc_5G_rx_stream == 0) || (pAd->dbdc_5G_rx_stream > 2)) {
			pAd->dbdc_5G_rx_stream = 2;
		}
		if ((pAd->dbdc_5G_tx_stream == 0) || (pAd->dbdc_5G_tx_stream > 2)) {
			pAd->dbdc_5G_tx_stream = 2;
		}
	} else {
		pAd->dbdc_2G_rx_stream = 1;
		pAd->dbdc_2G_tx_stream = 1;
		pAd->dbdc_5G_rx_stream = 1;
		pAd->dbdc_5G_tx_stream = 1;
	}

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DBDC 2G TxPath = %d, 2G RxPath = %d\n",
            __FUNCTION__, pAd->dbdc_2G_tx_stream, pAd->dbdc_2G_rx_stream));
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s(): DBDC 5G TxPath = %d, 2G RxPath = %d\n",
            __FUNCTION__, pAd->dbdc_5G_tx_stream, pAd->dbdc_5G_rx_stream));

}


static VOID mt7615_fw_prepare(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("%s():FW(%x), HW(%x), CHIPID(%x))\n",
				__FUNCTION__,  pAd->FWVersion, pAd->HWVersion, pAd->ChipID));

	if (IS_MT7615_FW_VER_E3(pAd))
    {
#ifdef NEED_ROM_PATCH
        pChipCap->rom_patch_header_image = mt7615_rom_patch;
        pChipCap->rom_patch_len = sizeof(mt7615_rom_patch);
#endif /* NEED_ROM_PATCH */
    
        /* FW IMAGE */
        pChipCap->fw_header_image = MT7615_FirmwareImage;
        pChipCap->fw_bin_file_name = "";
        pChipCap->fw_len = sizeof(MT7615_FirmwareImage);


        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%s(%d): MT7615_E3, USE E3 patch and ram code binary image\n", 
                                            __FUNCTION__, __LINE__));
	#ifdef CONFIG_RALINK_MT7621
	{
		UINT32 reg_val;
		HIF_IO_READ32(pAd, MT_WPDMA_GLO_CFG, &reg_val);
		reg_val &= ~(BIT(9)| BITS(22, 23));
		reg_val |= (BIT(9) | BIT(22));
		HIF_IO_WRITE32(pAd, MT_WPDMA_GLO_CFG, reg_val);
	}
	#endif /* CONFIG_RALINK_MT7621 */
    }
    else
    {
#ifdef NEED_ROM_PATCH
        pChipCap->rom_patch_header_image = mt7615_rom_patch;
        pChipCap->rom_patch_len = sizeof(mt7615_rom_patch);
#endif /* NEED_ROM_PATCH */

        /* Use E3 FW IMAGE as default */
        /* FW IMAGE */
        pChipCap->fw_header_image = MT7615_FirmwareImage;
        pChipCap->fw_bin_file_name = "";
        pChipCap->fw_len = sizeof(MT7615_FirmwareImage);


        MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
            ("%s(%d): Default use MT7615_E3, USE E3 patch and ram code binary image\n", 
                                            __FUNCTION__, __LINE__));

    }

	return ;
}




#ifdef DBDC_MODE
static UCHAR MT7615BandGetByIdx(RTMP_ADAPTER *pAd, UCHAR BandIdx)
{
	switch(BandIdx){
	case 0:
		return RFIC_24GHZ;
	break;
	case 1:
		return RFIC_5GHZ;
	break;
	default:
		return RFIC_DUAL_BAND;
	}
}
#endif


void mt7615_heart_beat_check(RTMP_ADAPTER *pAd)
{
#define HEART_BEAT_CHECK_PERIOD 30
#define N9_HEART_BEAT_ADDR 0xc2ec //0x820682ec, PSE dummy CR
#define CR4_HEART_BEAT_ADDR 0x80200
	UINT32 mac_val;
	UINT8 cr4_detect = FALSE;
	UINT8 n9_detect = FALSE;
	RTMP_STRING *str = NULL;
	if ((pAd->Mlme.PeriodicRound % HEART_BEAT_CHECK_PERIOD) == 0) {

		if (pAd->heart_beat_stop == TRUE)
			return;

		MAC_IO_READ32(pAd, N9_HEART_BEAT_ADDR, &mac_val);
		if (mac_val == pAd->pre_n9_heart_beat_cnt)
			pAd->pre_n9_heart_beat_cnt = ~mac_val;
		else if (~mac_val == pAd->pre_n9_heart_beat_cnt)
			n9_detect = TRUE;
		else
			pAd->pre_n9_heart_beat_cnt = mac_val;

		MAC_IO_READ32(pAd, CR4_HEART_BEAT_ADDR, &mac_val);
		if (mac_val == pAd->pre_cr4_heart_beat_cnt)
			pAd->pre_cr4_heart_beat_cnt = ~mac_val;
		else if (~mac_val == pAd->pre_cr4_heart_beat_cnt)
			cr4_detect = TRUE;
		else
			pAd->pre_cr4_heart_beat_cnt = mac_val;

		if (n9_detect && cr4_detect)
			str = "N9 and CR4 heart beat stop!!\n";
		else if (n9_detect)
			str = "N9 heart beat stop!!\n";
		else if (cr4_detect)
			str = "CR4 heart beat stop!!\n";

		if (str != NULL) {
			pAd->heart_beat_stop = TRUE;
			MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_OFF, 
				("[%s]:%s", RtmpOsGetNetDevName(pAd->net_dev), str));
#ifdef MT_FDB
			show_fdb_n9_log(pAd, NULL);
			show_fdb_cr4_log(pAd, NULL);
#endif /* MT_FDB */
#ifdef ERR_RECOVERY
			ser_sys_reset(str);
#endif
		}
	}
}


#ifdef TXBF_SUPPORT
void mt7615_setETxBFCap(
    IN  RTMP_ADAPTER      *pAd,
    IN  TXBF_STATUS_INFO  *pTxBfInfo)
{

    HT_BF_CAP *pTxBFCap = pTxBfInfo->pHtTxBFCap;

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0)
	{
	    switch (pTxBfInfo->cmmCfgETxBfEnCond)
        {
        case SUBF_ALL:
        default:
		    pTxBFCap->RxNDPCapable         = TRUE;
		    pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
		    pTxBFCap->ExpNoComSteerCapable = FALSE;
		    pTxBFCap->ExpComSteerCapable   = TRUE;//!pTxBfInfo->cmmCfgETxBfNoncompress;
		    pTxBFCap->ExpNoComBF           = 0; // HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->ExpComBF             = HT_ExBF_FB_CAP_IMMEDIATE;//pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->MinGrouping          = 3;
		    pTxBFCap->NoComSteerBFAntSup   = 0;
		    pTxBFCap->ComSteerBFAntSup     = 3;

		    pTxBFCap->TxSoundCapable       = FALSE;  // Support staggered sounding frames
		    pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum-1;
		    break;
		case SUBF_BFER:
		    pTxBFCap->RxNDPCapable         = FALSE;
		    pTxBFCap->TxNDPCapable         = (pTxBfInfo->ucRxPathNum > 1) ? TRUE : FALSE;
		    pTxBFCap->ExpNoComSteerCapable = FALSE;
		    pTxBFCap->ExpComSteerCapable   = TRUE;//!pTxBfInfo->cmmCfgETxBfNoncompress;
		    pTxBFCap->ExpNoComBF           = 0; // HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->ExpComBF             = HT_ExBF_FB_CAP_IMMEDIATE;//pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->MinGrouping          = 3;
		    pTxBFCap->NoComSteerBFAntSup   = 0;
		    pTxBFCap->ComSteerBFAntSup     = 3;

		    pTxBFCap->TxSoundCapable       = FALSE;  // Support staggered sounding frames
		    pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum-1;
		    break;
        case SUBF_BFEE:
		    pTxBFCap->RxNDPCapable         = TRUE;
		    pTxBFCap->TxNDPCapable         = FALSE;
		    pTxBFCap->ExpNoComSteerCapable = FALSE;
		    pTxBFCap->ExpComSteerCapable   = TRUE;//!pTxBfInfo->cmmCfgETxBfNoncompress;
		    pTxBFCap->ExpNoComBF           = 0; // HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->ExpComBF             = HT_ExBF_FB_CAP_IMMEDIATE;//pTxBfInfo->cmmCfgETxBfNoncompress? HT_ExBF_FB_CAP_NONE: HT_ExBF_FB_CAP_IMMEDIATE;
		    pTxBFCap->MinGrouping          = 3;
		    pTxBFCap->NoComSteerBFAntSup   = 0;
		    pTxBFCap->ComSteerBFAntSup     = 3;

		    pTxBFCap->TxSoundCapable       = FALSE;  // Support staggered sounding frames
		    pTxBFCap->ChanEstimation       = pTxBfInfo->ucRxPathNum-1;
		    break;
	    }
	}
    else
    {
        memset(pTxBFCap, 0, sizeof(*pTxBFCap));
    }
}


#ifdef VHT_TXBF_SUPPORT
void mt7615_setVHTETxBFCap(
    IN  RTMP_ADAPTER *pAd,
    IN  TXBF_STATUS_INFO  *pTxBfInfo)
{
    VHT_CAP_INFO *pTxBFCap = pTxBfInfo->pVhtTxBFCap;

    //MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: cmmCfgETxBfEnCond = %d\n", __FUNCTION__, (UCHAR)pTxBfInfo->cmmCfgETxBfEnCond));

	if (pTxBfInfo->cmmCfgETxBfEnCond > 0)
    {
        switch (pTxBfInfo->cmmCfgETxBfEnCond)
        {
        case SUBF_ALL:
        default:
            pTxBFCap->bfee_cap_su       = 1;
            pTxBFCap->bfer_cap_su       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;

#ifdef CFG_SUPPORT_MU_MIMO
            switch (pAd->CommonCfg.MUTxRxEnable)
            {
            case MUBF_OFF:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_BFER:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
                break;
            case MUBF_BFEE:
                pTxBFCap->bfee_cap_mu = 1;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_ALL:
                pTxBFCap->bfee_cap_mu = 1;
                pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
                break;
            default:
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;  
            }
#else
            pTxBFCap->bfee_cap_mu = 0;
            pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
            pTxBFCap->bfee_sts_cap      = 3;
            pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
            break;
        case SUBF_BFER:
            pTxBFCap->bfee_cap_su       = 0;
            pTxBFCap->bfer_cap_su       = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;

#ifdef CFG_SUPPORT_MU_MIMO
            switch (pAd->CommonCfg.MUTxRxEnable)
            {
            case MUBF_OFF:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_BFER:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
                break;
            case MUBF_BFEE:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_ALL:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = (pTxBfInfo->ucTxPathNum > 1) ? 1 : 0;
                break;
            default:
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;  
            }
#else
            pTxBFCap->bfee_cap_mu = 0;
            pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
            pTxBFCap->bfee_sts_cap      = 0;
            pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
            break;
        case SUBF_BFEE:
            pTxBFCap->bfee_cap_su       = 1;
            pTxBFCap->bfer_cap_su       = 0;

#ifdef CFG_SUPPORT_MU_MIMO
            switch (pAd->CommonCfg.MUTxRxEnable)
            {
            case MUBF_OFF:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_BFER:
                pTxBFCap->bfee_cap_mu = 0;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_BFEE:
                pTxBFCap->bfee_cap_mu = 1;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            case MUBF_ALL:
                pTxBFCap->bfee_cap_mu = 1;
                pTxBFCap->bfer_cap_mu = 0;
                break;
            default:
                MTWF_LOG(DBG_CAT_ALL, DBG_SUBCAT_ALL, DBG_LVL_ERROR, ("%s: set wrong parameters\n", __FUNCTION__));
                break;  
            }
#else
            pTxBFCap->bfee_cap_mu = 0;
            pTxBFCap->bfer_cap_mu = 0;
#endif /* CFG_SUPPORT_MU_MIMO */
            pTxBFCap->bfee_sts_cap      = 3;
            pTxBFCap->num_snd_dimension = pTxBfInfo->ucTxPathNum - 1;
            break;            
        }
    }
	else
	{
		pTxBFCap->num_snd_dimension = 0;
        pTxBFCap->bfee_cap_mu       = 0;
        pTxBFCap->bfee_cap_su       = 0;
        pTxBFCap->bfer_cap_mu       = 0;
        pTxBFCap->bfer_cap_su       = 0;
        pTxBFCap->bfee_sts_cap      = 0;
    }
}
#endif /* VHT_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */

#ifdef SMART_CARRIER_SENSE_SUPPORT
VOID mt7615_SmartCarrierSense(
 IN  RTMP_ADAPTER *pAd)
{
    //UINT32      lv0, lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8, lv9, lv10, CrValue, PdBlkTh;
    //UCHAR PER=0;
    PSMART_CARRIER_SENSE_CTRL    pSCSCtrl;
    BOOL	RxOnly=FALSE;
    UINT32 TotalTP=0, CrValue=0;
    //INT32   PdBlkTh=0, OfdmPdBlkTh = 0;
    INT32	 CckPdBlkBundry = 0, OfdmPdBlkBundry = 0 ;
    UCHAR i;
    //UCHAR       concurrent_bands = HcGetAmountOfBand(pAd);    
    UCHAR idx;
    UINT32 MaxRtsRtyCount = 0;
    UINT32 MaxRtsCount = 0;
    UINT32 TempValue = 0;
    BOOL WriteCr = FALSE;
    UINT32 PdCount = 0, MdrdyCount = 0;
    

    pSCSCtrl = &pAd->SCSCtrl;
 
    /* 2. Tx/Rx */
//    MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
//		("%s Band0:Tx/Rx=%d/%d MinRSSI=%d, Band1:Tx/Rx=%d/%d, MinRSSI=%d \n",
//				__FUNCTION__, pAd->SCSCtrl.OneSecTxByteCount[0], pAd->SCSCtrl.OneSecRxByteCount[0], pAd->SCSCtrl.SCSMinRssi[0],
//		pAd->SCSCtrl.OneSecTxByteCount[1], pAd->SCSCtrl.OneSecRxByteCount[1], pAd->SCSCtrl.SCSMinRssi[1]));

    /* 3. based on minRssi to adjust PD_BLOCK_TH */
    for(i=0;i<1;i++) //NO DBDC support.
    {
    	for (idx = 0; idx < 4; idx++) {
		HW_IO_READ32(pAd, MIB_MB0SDR0 + (idx * BssOffset) + (i*BandOffset), &CrValue);
		TempValue = (CrValue >> RtsRtyCountOffset) & RtsCountMask;
		if (TempValue > MaxRtsRtyCount) {                                
			MaxRtsRtyCount = TempValue;
			MaxRtsCount = CrValue & RtsCountMask;
		}        
               }

	pSCSCtrl->RtsCount[i] = MaxRtsCount;
	pSCSCtrl->RtsRtyCount[i] = MaxRtsRtyCount;

	PdCount = pAd->MsMibBucket.PdCount[i][pAd->MsMibBucket.CurIdx];
	MdrdyCount = pAd->MsMibBucket.MdrdyCount[i][pAd->MsMibBucket.CurIdx];
	//printk("PD_count=%x, MDRSY_count=%x \n", CrValue, CrValue2);
	pSCSCtrl->CckFalseCcaCount[i] = ( PdCount & 0xffff) - (MdrdyCount & 0xffff);
	pSCSCtrl->OfdmFalseCcaCount[i] = ((PdCount & 0xffff0000 ) >> 16) - ((MdrdyCount & 0xffff0000 ) >> 16) ;

	
        if (pSCSCtrl->SCSEnable[i] == SCS_ENABLE)
        {

	TotalTP = (pSCSCtrl->OneSecTxByteCount[i] + pSCSCtrl->OneSecRxByteCount[i]);

	if ((pSCSCtrl->OneSecTxByteCount[i]) * 9 <  pSCSCtrl->OneSecRxByteCount[i])
		RxOnly = TRUE;	

	//if (1/*TotalTP > pSCSCtrl->SCSTrafficThreshold[i]*/) { //default 2M
	if ((pSCSCtrl->RtsCount[i] > 0 || pSCSCtrl->RtsRtyCount[i] > 0) && RxOnly == FALSE) {
		/* Set PD_BLOCKING_BOUNDARY */
		CckPdBlkBundry=min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i])+256), pSCSCtrl->CckFixedRssiBond[i]);
		/* CCK part */
		if ((pSCSCtrl->CckFalseCcaCount[i]  > pSCSCtrl->CckFalseCcaUpBond[i])) { //Decrease coverage
			if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {//RTS PER < 40% 
				
				if (pAd->SCSCtrl.CckPdBlkTh[i] == PdBlkCckThDefault && CckPdBlkBundry > FastInitTh) {
					pAd->SCSCtrl.CckPdBlkTh[i] = FastInitTh;
					WriteCr = TRUE;
				}
				//pSCSCtrl->CckPdBlkTh[i] += 2; //One step is 2dB.
				else if ((pSCSCtrl->CckPdBlkTh[i] + OneStep) <= CckPdBlkBundry) {
					pSCSCtrl->CckPdBlkTh[i] += OneStep;
					/* Write to CR */
					WriteCr = TRUE;
				}
				else  if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry ){					
					pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
					/* Write to CR */	
					WriteCr = TRUE;
				}
			}				
		}
		else if (pSCSCtrl->CckFalseCcaCount[i] < pSCSCtrl->CckFalseCcaLowBond[i] || (MaxRtsCount + (MaxRtsCount >>1)) < MaxRtsRtyCount) { //Increase coverage
			if (pSCSCtrl->CckPdBlkTh[i] -OneStep >= PdBlkCckThDefault) {
				pSCSCtrl->CckPdBlkTh[i] -= OneStep;
				if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) //Tracking mini RSSI to prevent out of service rage.
					pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
				/* Write to CR */
				WriteCr = TRUE;
			}
			
		}else { //Stable stat
			if (pSCSCtrl->CckPdBlkTh[i] > CckPdBlkBundry) {//Tracking mini RSSI to prevent out of service rage.
				pSCSCtrl->CckPdBlkTh[i] = CckPdBlkBundry;
				WriteCr = TRUE;
			}		
		}

		if (WriteCr) { //Write for CCK PD blocking
			HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
			CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
			CrValue |= (pSCSCtrl->CckPdBlkTh[i]  <<PdBlkCckThOffset);
			HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);

			HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
			CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
			CrValue |= (pSCSCtrl->CckPdBlkTh[i]  <<PdBlkCck1RThOffset);
			HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
		}	

		WriteCr = FALSE; //Clear
		/* OFDM part */
		/* Set PD_BLOCKING_BOUNDARY */
		OfdmPdBlkBundry = min(((pSCSCtrl->SCSMinRssi[i] - pSCSCtrl->SCSMinRssiTolerance[i]) *2 + 512),  pSCSCtrl->OfdmFixedRssiBond[i]);
		if (pSCSCtrl->OfdmFalseCcaCount[i] > pSCSCtrl->OfdmFalseCcaUpBond[i] ) { //Decrease coverage
			if (MaxRtsCount > (MaxRtsRtyCount + (MaxRtsRtyCount >> 1))) {//RTS PER < 40% 
				
				if (pAd->SCSCtrl.OfdmPdBlkTh[i] == PdBlkOfmdThDefault && OfdmPdBlkBundry > FastInitThOfdm) {
					pAd->SCSCtrl.OfdmPdBlkTh[i] = FastInitThOfdm;
					WriteCr = TRUE;
				}
				if ((pSCSCtrl->OfdmPdBlkTh[i] + OneStep) <= OfdmPdBlkBundry) {
					pSCSCtrl->OfdmPdBlkTh[i] += OneStep;
					/* Write to CR */
					WriteCr = TRUE;
				}
				else  if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry ){					
					pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
					/* Write to CR */	
					WriteCr = TRUE;
				}	
			}
		}
		else if (pSCSCtrl->OfdmFalseCcaCount[i] < pSCSCtrl->OfdmFalseCcaLowBond[i] ||(MaxRtsCount + (MaxRtsCount >>1)) < MaxRtsRtyCount) { //Increase coverage
			if (pSCSCtrl->OfdmPdBlkTh[i] -OneStep >= PdBlkOfmdThDefault) {
				pSCSCtrl->OfdmPdBlkTh[i] -= OneStep;
				
				if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) //Tracking mini RSSI to prevent out of service rage.
					pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
				
				/* Write to CR */
				WriteCr = TRUE;
			}
		}else { //Stable stat
			if (pSCSCtrl->OfdmPdBlkTh[i] > OfdmPdBlkBundry) {//Tracking mini RSSI to prevent out of service rage.
				pSCSCtrl->OfdmPdBlkTh[i] = OfdmPdBlkBundry;
				WriteCr = TRUE;
			}
		}
		
		if (WriteCr) {//Write for OFDM PD blocking
			if (i == 0) {
		                    HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
		                    CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		                    CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffset);
		                    HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
			} else if (i == 1) { //DBDC
				HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);
				CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);  /* OFDM PD BLOCKING TH */
				CrValue |= (pSCSCtrl->OfdmPdBlkTh[i] << PdBlkOfmdThOffsetB1);
				HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
			}
				
		}	

	} else { //Disable SCS  No traffic
 
		if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {

			MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_TRACE, ("Disable SCS due to RtsCount=%d RxOnly=%d\n", pSCSCtrl->RtsCount[i], RxOnly));
			pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
			HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
			CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
			CrValue |= (PdBlkCckThDefault <<PdBlkCckThOffset); /* 0x92 is default value -110dBm */
			HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);

			HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
			CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
			CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset); /* 0x92 is default value -110dBm */
			HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
		}	
		
		if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
			if (i == 0) {
			pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
			HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);		
			CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
			CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffset);
			HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
			} else if (i == 1) {
				pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
				HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);		
				CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);  /* OFDM PD BLOCKING TH */
				CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffsetB1);
				HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
			}
		}
		


			
	}
        }else if ( pSCSCtrl->SCSEnable[i] == SCS_DISABLE) {
	if (pSCSCtrl->CckPdBlkTh[i] != PdBlkCckThDefault) {
		pSCSCtrl->CckPdBlkTh[i] = PdBlkCckThDefault;
		HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
		CrValue |= (PdBlkCckThDefault <<PdBlkCckThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);

		HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
		CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
		CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset); /* 0x92 is default value -110dBm */
		HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);
	}	
	
	if (pSCSCtrl->OfdmPdBlkTh[i] != PdBlkOfmdThDefault) {
		if ( i == 0) {
		pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
		HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);		
		CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
		CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffset);
		HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);
		} else if ( i == 1) {
			pSCSCtrl->OfdmPdBlkTh[i] = PdBlkOfmdThDefault;
			HW_IO_READ32(pAd, BAND1_PHY_MIN_PRI_PWR, &CrValue);		
			CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffsetB1);  /* OFDM PD BLOCKING TH */
			CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffsetB1);
			HW_IO_WRITE32(pAd, BAND1_PHY_MIN_PRI_PWR, CrValue);
		}
		}
        }
    }

    return;

}

VOID mt7615_SetSCS(
 IN  RTMP_ADAPTER *pAd,
 IN  UCHAR              BandIdx,
 IN  UINT32             value)
{
    UINT32 CrValue;

    printk("%s()  BandIdx=%d, SCSEnable=%d \n", __FUNCTION__, BandIdx, value);

    if (value > 500) /* traffic threshold.*/
    {
        pAd->SCSCtrl.SCSTrafficThreshold[BandIdx] = value;
    }
    else if (value == SCS_DISABLE)
    {
        pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_DISABLE;
        pAd->SCSCtrl.SCSStatus[BandIdx] = PD_BLOCKING_OFF;

        /* Disable PD blocking and reset related CR */
        HW_IO_READ32(pAd, PHY_MIN_PRI_PWR, &CrValue);
        //CrValue &= ~(0x1 << PdBlkEnabeOffset); /* Bit[19] */
        CrValue &= ~(PdBlkOfmdThMask << PdBlkOfmdThOffset);  /* OFDM PD BLOCKING TH */
        CrValue |= (PdBlkOfmdThDefault <<PdBlkOfmdThOffset);
        HW_IO_WRITE32(pAd, PHY_MIN_PRI_PWR, CrValue);

        HW_IO_READ32(pAd, PHY_RXTD_CCKPD_7, &CrValue);
        CrValue &= ~(PdBlkCckThMask << PdBlkCckThOffset); /* Bit[8:1] */
        CrValue |= (PdBlkCckThDefault <<PdBlkCckThOffset); /* 0x92 is default value -110dBm */
        HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_7, CrValue);

        HW_IO_READ32(pAd, PHY_RXTD_CCKPD_8, &CrValue);
        CrValue &= ~(PdBlkCckThMask << PdBlkCck1RThOffset); /* Bit[31:24] */
        CrValue |= (PdBlkCckThDefault << PdBlkCck1RThOffset); /* 0x92 is default value -110dBm */
        HW_IO_WRITE32(pAd, PHY_RXTD_CCKPD_8, CrValue);

    }
    else if (value == SCS_ENABLE)
    {
        pAd->SCSCtrl.SCSEnable[BandIdx] = SCS_ENABLE;
    }

}
#endif /* SMART_CARRIER_SENSE_SUPPORT */


UCHAR* mt7615_get_default_bin_image(RTMP_ADAPTER *pAd)
{
#ifdef MULTI_INF_SUPPORT
	if (multi_inf_get_idx(pAd) == 0){		
#if defined (CONFIG_FIRST_IF_IPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st iPAiLNA default bin.\n"));
		return MT7615_E2PImage1_iPAiLNA;
#elif defined (CONFIG_FIRST_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st iPAeLNA default bin.\n"));
		return MT7615_E2PImage1_iPAeLNA;
#elif defined (CONFIG_FIRST_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st ePAiLNA default bin.\n"));
		return MT7615_E2PImage1_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 1st ePAeLNA default bin.\n"));
		return MT7615_E2PImage1_ePAeLNA;
#endif

	}
#if defined(MT_SECOND_CARD)
	else if (multi_inf_get_idx(pAd) == 1){		
#if defined (CONFIG_SECOND_IF_IPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd iPAiLNA default bin.\n"));
		return MT7615_E2PImage2_iPAiLNA;
#elif defined (CONFIG_SECOND_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd iPAeLNA default bin.\n"));
		return MT7615_E2PImage2_iPAeLNA;
#elif defined (CONFIG_SECOND_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd ePAiLNA default bin.\n"));
		return MT7615_E2PImage2_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 2nd ePAeLNA default bin.\n"));
		return MT7615_E2PImage2_ePAeLNA;
#endif

	}
#endif /* MT_SECOND_CARD */
#if defined(MT_THIRD_CARD)
	else if (multi_inf_get_idx(pAd) == 2){		
#if defined (CONFIG_THIRD_IF_IPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd iPAiLNA default bin.\n"));
		return MT7615_E2PImage3_iPAiLNA;
#elif defined (CONFIG_THIRD_IF_IPAELNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd iPAeLNA default bin.\n"));
		return MT7615_E2PImage3_iPAeLNA;
#elif defined (CONFIG_THIRD_IF_EPAILNA)
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd ePAiLNA default bin.\n"));
		return MT7615_E2PImage3_ePAiLNA;
#else
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use 3rd ePAeLNA default bin.\n"));
		return MT7615_E2PImage3_ePAeLNA;
#endif

	}
#endif /* MT_THIRD_CARD */
	else
#endif /* MULTI_INF_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Use the default ePAeLNA bin image!\n"));
		return MT7615_E2PImage1_ePAeLNA;
	}

	return NULL;
}

UCHAR* mt7615_get_default_bin_image_file(RTMP_ADAPTER *pAd)
{
#ifdef MULTI_INF_SUPPORT
	if (multi_inf_get_idx(pAd) == 0) {
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), DEFAULT_BIN_FILE));
		return DEFAULT_BIN_FILE;
	}
#if defined(MT_SECOND_CARD)
	else if (multi_inf_get_idx(pAd) == 1) {		
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), SECOND_BIN_FILE));
		return SECOND_BIN_FILE;
#endif /* MT_SECOND_CARD */
	}
#if defined(MT_THIRD_CARD)
	else if (multi_inf_get_idx(pAd) == 2) {		
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("Use %dst %s default bin.\n", multi_inf_get_idx(pAd), THIRD_BIN_FILE));
		return THIRD_BIN_FILE;
	}
#endif /* MT_THIRD_CARD */
	else
#endif /* MULTI_INF_SUPPORT */
	{
		MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF,
				("Use the default %s bin image!\n", DEFAULT_BIN_FILE));
				
		return DEFAULT_BIN_FILE;
	}

	return NULL;
}

static struct rx_dly_ctl_cfg mt7615_rx_dly_ctl_ul_tbl[] = {
	{0, 0x811c},
	{700, 0xc01f},
};

static struct rx_dly_ctl_cfg mt7615_rx_dly_ctl_dl_tbl[] = {
	{0, 0x811c},
};

static RTMP_CHIP_OP MT7615_ChipOp = {0};
static RTMP_CHIP_CAP MT7615_ChipCap = {0};


static VOID mt7615_chipCap_init(RTMP_ADAPTER *pAd, BOOLEAN b11nOnly, BOOLEAN bThreeAnt)
{
	if (bThreeAnt)
	{
		MT7615_ChipCap.max_nss = 3;
	}
	else
	{
		MT7615_ChipCap.max_nss = 4;
	}
#ifdef DOT11_VHT_AC
	MT7615_ChipCap.max_vht_mcs = VHT_MCS_CAP_9;
    MT7615_ChipCap.g_band_256_qam = TRUE;
    	MT7615_ChipCap.max_bw160_nss = 2;
#endif /* DOT11_VHT_AC */

	MT7615_ChipCap.TXWISize = sizeof(TMAC_TXD_L); /* 32 */
	MT7615_ChipCap.RXWISize = 28;
	MT7615_ChipCap.WtblHwNum = MT7615_MT_WTBL_SIZE;
#ifdef RTMP_MAC_PCI
	MT7615_ChipCap.WPDMABurstSIZE = 3;
#endif
	MT7615_ChipCap.SnrFormula = SNR_FORMULA4;
	MT7615_ChipCap.FlgIsHwWapiSup = TRUE;
	MT7615_ChipCap.FlgIsHwAntennaDiversitySup = FALSE;
#ifdef STREAM_MODE_SUPPORT
	MT7615_ChipCap.FlgHwStreamMode = FALSE;
#endif
#ifdef TXBF_SUPPORT
	MT7615_ChipCap.FlgHwTxBfCap = TRUE;
#endif
#ifdef FIFO_EXT_SUPPORT
	MT7615_ChipCap.FlgHwFifoExtCap = FALSE;
#endif

	MT7615_ChipCap.asic_caps = (fASIC_CAP_PMF_ENC | fASIC_CAP_MCS_LUT);
#ifdef RX_CUT_THROUGH
	MT7615_ChipCap.asic_caps |= fASIC_CAP_BA_OFFLOAD;
#endif
	MT7615_ChipCap.asic_caps |=  fASIC_CAP_HW_DAMSDU;
#ifdef HDR_TRANS_TX_SUPPORT
	MT7615_ChipCap.asic_caps |= fASIC_CAP_TX_HDR_TRANS;
#endif /* HDR_TRANS_TX_SUPPORT */
#ifdef HDR_TRANS_RX_SUPPORT
	MT7615_ChipCap.asic_caps |= fASIC_CAP_RX_HDR_TRANS;
#endif /* HDR_TRANS_RX_SUPPORT */

#ifdef IGMP_SNOOP_SUPPORT
	MT7615_ChipCap.asic_caps |= fASIC_CAP_IGMP_SNOOP_OFFLOAD; 
#endif

#ifdef DBDC_MODE
	MT7615_ChipCap.asic_caps |= fASIC_CAP_DBDC;
#endif /* DBDC_MODE */

	MT7615_ChipCap.asic_caps |= fASIC_CAP_WMM_PKTDETECT_OFFLOAD;

	if (b11nOnly)
	{
		MT7615_ChipCap.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | \
									fPHY_CAP_HT | \
									fPHY_CAP_TXBF | fPHY_CAP_LDPC | \
									fPHY_CAP_BW40);
	}
	else
	{
		MT7615_ChipCap.phy_caps = (fPHY_CAP_24G | fPHY_CAP_5G | \
									fPHY_CAP_HT | fPHY_CAP_VHT | \
									fPHY_CAP_TXBF | fPHY_CAP_LDPC | fPHY_CAP_MUMIMO | \
									fPHY_CAP_BW40 | fPHY_CAP_BW80 | fPHY_CAP_BW160C | fPHY_CAP_BW160NC);
	}

	MT7615_ChipCap.MaxNumOfRfId = MAX_RF_ID;
	MT7615_ChipCap.pRFRegTable = NULL;
	MT7615_ChipCap.MaxNumOfBbpId = 200;
	MT7615_ChipCap.pBBPRegTable = NULL;
	MT7615_ChipCap.bbpRegTbSize = 0;
#ifdef NEW_MBSSID_MODE
#ifdef ENHANCE_NEW_MBSSID_MODE
	MT7615_ChipCap.MBSSIDMode = MBSSID_MODE4;
#else
	MT7615_ChipCap.MBSSIDMode = MBSSID_MODE1;
#endif /* ENHANCE_NEW_MBSSID_MODE */
#else
	MT7615_ChipCap.MBSSIDMode = MBSSID_MODE0;
#endif /* NEW_MBSSID_MODE */
#ifdef RTMP_EFUSE_SUPPORT
	MT7615_ChipCap.EFUSE_USAGE_MAP_START = 0x3c0;
	MT7615_ChipCap.EFUSE_USAGE_MAP_END = 0x3fb;
	MT7615_ChipCap.EFUSE_USAGE_MAP_SIZE = 60;
	MT7615_ChipCap.EFUSE_RESERVED_SIZE = 59;	// Cal-Free is 22 free block
#endif
	MT7615_ChipCap.EEPROM_DEFAULT_BIN = mt7615_get_default_bin_image(pAd);
	MT7615_ChipCap.EEPROM_DEFAULT_BIN_FILE = mt7615_get_default_bin_image_file(pAd);
	MT7615_ChipCap.EEPROM_DEFAULT_BIN_SIZE = sizeof(MT7615_E2PImage1_ePAeLNA);
    MT7615_ChipCap.EFUSE_BUFFER_CONTENT_SIZE = 0x378;

#ifdef CONFIG_ANDES_SUPPORT
	MT7615_ChipCap.CmdRspRxRing = RX_RING1;
	// TODO: shiang-MT7615, need load fw!
	MT7615_ChipCap.need_load_fw = TRUE;
	MT7615_ChipCap.DownLoadType = DownLoadTypeC;

#ifdef CONFIG_LOAD_CODE_BIN_METHOD
    MT7615_ChipCap.load_code_method = BIN_FILE_METHOD;
#else
	MT7615_ChipCap.load_code_method = HEADER_METHOD;
#endif
	// TODO: shiang-MT7615, need load patch!
#ifdef NEED_ROM_PATCH
	MT7615_ChipCap.need_load_rom_patch = TRUE;
#else
	MT7615_ChipCap.need_load_rom_patch = FALSE;
#endif /* NEED_ROM_PATCH */
	MT7615_ChipCap.ram_code_protect = FALSE;
	MT7615_ChipCap.rom_code_protect = TRUE;
	MT7615_ChipCap.ilm_offset = 0x00080000;
	MT7615_ChipCap.dlm_offset = 0x02090000;
	MT7615_ChipCap.rom_patch_offset = 0x80000;
#endif
	MT7615_ChipCap.MCUType = ANDES|CR4;
	// TODO: shiang-MT7615, fix me for this
#ifdef UNIFY_FW_CMD
	MT7615_ChipCap.cmd_header_len = sizeof(FW_TXD) + sizeof(TMAC_TXD_L);
#else
	MT7615_ChipCap.cmd_header_len = 12; /* sizeof(FW_TXD),*/
#endif /* UNIFY_FW_CMD */
#ifdef RTMP_PCI_SUPPORT
	MT7615_ChipCap.cmd_padding_len = 0;
#endif
	MT7615_ChipCap.fw_header_image = MT7615_FirmwareImage;
	MT7615_ChipCap.fw_len = sizeof(MT7615_FirmwareImage);
	MT7615_ChipCap.fw_header_image_ext = MT7615_CR4_FirmwareImage;
	MT7615_ChipCap.fw_len_ext = sizeof(MT7615_CR4_FirmwareImage);

    MT7615_ChipCap.fw_bin_file_name = MT7615_BIN_FILE_PATH;

#ifdef CARRIER_DETECTION_SUPPORT
	MT7615_ChipCap.carrier_func = TONE_RADAR_V2;
#endif
	MT7615_ChipCap.hif_type = HIF_MT;
	MT7615_ChipCap.rf_type = RF_MT;
	MT7615_ChipCap.TxAggLimit = 64;
	MT7615_ChipCap.RxBAWinSize = 64;
	MT7615_ChipCap.AMPDUFactor = 3;
#ifdef DOT11_VHT_AC
    MT7615_ChipCap.MaxMPDULength = MPDU_7991_OCTETS;
    MT7615_ChipCap.MaxAMPDULengthExp = 7;
#endif /* DOT11_VHT_AC */

    MT7615_ChipCap.default_txop = 0x60;
    MT7615_ChipCap.CurrentTxOP = 0x0;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
	MT7615_ChipCap.fgRateAdaptFWOffload = TRUE;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */

	/*for multi-wmm*/
	MT7615_ChipCap.WmmHwNum = MT7615_MT_WMM_SIZE;

    MT7615_ChipCap.PDA_PORT = MT7615_PDA_PORT;
	MT7615_ChipCap.SupportAMSDU = TRUE;

#ifdef BA_TRIGGER_OFFLOAD
	MT7615_ChipCap.BATriggerOffload = TRUE;
#endif

    MT7615_ChipCap.APPSMode = APPS_MODE2;

#ifdef MT_WOW_SUPPORT
    MT7615_ChipCap.nWakeupInterface = WOW_WAKEUP_BY_PCIE;
#endif /* MT_WOW_SUPPORT */

	MT7615_ChipCap.CtParseLen = MT7615_CT_PARSE_LEN;
}


static VOID mt7615_chipOp_init(void)
{
	MT7615_ChipOp.ChipBBPAdjust = mt7615_bbp_adjust;
	MT7615_ChipOp.ChipSwitchChannel = mt7615_switch_channel;
#ifdef NEW_SET_RX_STREAM
    MT7615_ChipOp.ChipSetRxStream = mt7615_set_RxStream;
#endif
	MT7615_ChipOp.AsicMacInit = mt7615_init_mac_cr;
	MT7615_ChipOp.AsicBbpInit = MT7615BBPInit;
	MT7615_ChipOp.AsicRfInit = mt7615_init_rf_cr;
	MT7615_ChipOp.AsicAntennaDefaultReset = mt7615_antenna_default_reset;
	MT7615_ChipOp.ChipAGCInit = NULL;
	MT7615_ChipOp.AsicRfTurnOn = NULL;
	MT7615_ChipOp.AsicHaltAction = NULL;
	MT7615_ChipOp.AsicRfTurnOff = NULL;
	MT7615_ChipOp.AsicReverseRfFromSleepMode = NULL;
#ifdef CARRIER_DETECTION_SUPPORT
	MT7615_ChipOp.ToneRadarProgram = ToneRadarProgram_v2;
#endif
	MT7615_ChipOp.RxSensitivityTuning = NULL;
	MT7615_ChipOp.DisableTxRx = NULL;
#ifdef RTMP_PCI_SUPPORT
	//MT7615_ChipOp.AsicRadioOn = RT28xxPciAsicRadioOn;
	//MT7615_ChipOp.AsicRadioOff = RT28xxPciAsicRadioOff;
#endif
	MT7615_ChipOp.show_pwr_info = NULL;

#ifdef CAL_FREE_IC_SUPPORT
	MT7615_ChipOp.is_cal_free_ic = mt7615_is_cal_free_ic;
	MT7615_ChipOp.cal_free_data_get = mt7615_cal_free_data_get;
	MT7615_ChipOp.check_is_cal_free_merge = mt7615_check_is_cal_free_merge;
#endif /* CAL_FREE_IC_SUPPORT */

#ifdef RF_LOCKDOWN
    MT7615_ChipOp.check_RF_lock_down = mt7615_check_RF_lock_down;
    MT7615_ChipOp.write_RF_lock_parameter = mt7615_write_RF_lock_parameter;
    MT7615_ChipOp.merge_RF_lock_parameter = mt7615_merge_RF_lock_parameter;
    MT7615_ChipOp.Read_Effuse_parameter = mt7615_Read_Effuse_parameter;
    MT7615_ChipOp.Config_Effuse_Country = mt7615_Config_Effuse_Country;
#endif /* RF_LOCKDOWN */

#ifdef MT_WOW_SUPPORT
	MT7615_ChipOp.AsicWOWEnable = MT76xxAndesWOWEnable;
	MT7615_ChipOp.AsicWOWDisable = MT76xxAndesWOWDisable;
	//MT7615_ChipOp.AsicWOWInit = MT76xxAndesWOWInit,
#endif /* MT_WOW_SUPPORT */

	MT7615_ChipOp.MtCmdTx = MtCmdSendMsg;
	MT7615_ChipOp.fw_prepare = mt7615_fw_prepare;
#ifdef DBDC_MODE
	MT7615_ChipOp.BandGetByIdx = MT7615BandGetByIdx;
#endif

#ifdef TXBF_SUPPORT
    MT7615_ChipOp.TxBFInit                   = mt_WrapTxBFInit;
    MT7615_ChipOp.ClientSupportsETxBF        = mt_WrapClientSupportsETxBF;
#ifdef VHT_TXBF_SUPPORT
    MT7615_ChipOp.ClientSupportsVhtETxBF     = mt_WrapClientSupportsVhtETxBF;
#endif
    MT7615_ChipOp.TxBFInit                   = mt_WrapTxBFInit;
    MT7615_ChipOp.setETxBFCap                = mt7615_setETxBFCap;
    MT7615_ChipOp.BfStaRecUpdate             = mt_AsicBfStaRecUpdate;
    MT7615_ChipOp.BfStaRecRelease            = mt_AsicBfStaRecRelease;
    MT7615_ChipOp.BfPfmuMemAlloc             = CmdPfmuMemAlloc;
    MT7615_ChipOp.TxBfTxApplyCtrl            = CmdTxBfTxApplyCtrl;
    MT7615_ChipOp.BfApClientCluster          = CmdTxBfApClientCluster;
    MT7615_ChipOp.BfReptClonedStaToNormalSta = CmdTxBfReptClonedStaToNormalSta;
    MT7615_ChipOp.BfPfmuMemRelease           = CmdPfmuMemRelease;
    MT7615_ChipOp.BfHwEnStatusUpdate         = CmdTxBfHwEnableStatusUpdate;
    MT7615_ChipOp.BfeeHwCtrl                 = CmdTxBfeeHwCtrl;
#ifdef VHT_TXBF_SUPPORT
    MT7615_ChipOp.ClientSupportsVhtETxBF     = mt_WrapClientSupportsVhtETxBF;
    MT7615_ChipOp.setVHTETxBFCap             = mt7615_setVHTETxBFCap;
#endif /* VHT_TXBF_SUPPORT */
#endif /* TXBF_SUPPORT */
	MT7615_ChipOp.bufferModeEfuseFill = mt7615_bufferModeEfuseFill;
#ifdef SMART_CARRIER_SENSE_SUPPORT
    MT7615_ChipOp.SmartCarrierSense = mt7615_SmartCarrierSense;
    MT7615_ChipOp.ChipSetSCS = mt7615_SetSCS;
#endif /* SMART_CARRIER_SENSE_SUPPORT */
#ifdef GREENAP_SUPPORT
    MT7615_ChipOp.EnableAPMIMOPS = enable_greenap;
    MT7615_ChipOp.DisableAPMIMOPS = disable_greenap;
#endif /* GREENAP_SUPPORT */
	MT7615_ChipOp.heart_beat_check = mt7615_heart_beat_check;

#ifdef CONFIG_RALINK_MT7621
	MT7615_ChipOp.hif_set_pcie_read_params =
					mt7615_hif_set_pcie_read_params;
#endif /* CONFIG_RALINK_MT7621*/
}


VOID mt7615_init(RTMP_ADAPTER *pAd)
{
	RTMP_CHIP_CAP *pChipCap = &pAd->chipCap;
	struct rx_delay_control *rx_delay_ctl = &pAd->tr_ctl.rx_delay_ctl;
	UINT32 Value;
	BOOLEAN b11nOnly = FALSE, bThreeAnt = FALSE;

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("%s()-->\n", __FUNCTION__));

	RTMP_IO_READ32(pAd, STRAP_STA, &Value);
	if (GET_THREE_ANT(Value))
	{
		bThreeAnt = TRUE;

		if (GET_11N_ONLY(Value))
		{
			b11nOnly = TRUE;
		}
	}

	mt7615_chipCap_init(pAd, b11nOnly, bThreeAnt);
	mt7615_chipOp_init();
	NdisMoveMemory(&pAd->chipCap, &MT7615_ChipCap, sizeof(RTMP_CHIP_CAP));
	NdisMoveMemory(&pAd->chipOps, &MT7615_ChipOp, sizeof(RTMP_CHIP_OP));

	pAd->chipCap.hif_type = HIF_MT;
#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
	Mt7615AsicArchOpsInit(pAd);
#endif

	pAd->chipCap.mac_type = MAC_MT;
	pAd->TxSwRingNum = 2;

	mt_phy_probe(pAd);

// TODO: shiang-MT7615, debug for firmware download!!
	pChipCap->tx_hw_hdr_len = pChipCap->TXWISize;// + sizeof(CR4_TXP_MSDU_INFO);
	pChipCap->rx_hw_hdr_len = pChipCap->RXWISize;


	RTMP_DRS_ALG_INIT(pAd, RATE_ALG_AGBS);

	/*
		Following function configure beacon related parameters
		in pChipCap
			FlgIsSupSpecBcnBuf / BcnMaxHwNum /
			WcidHwRsvNum / BcnMaxHwSize / BcnBase[]
	*/
	mt_chip_bcn_parameter_init(pAd);

    pChipCap->OmacNums = 5;
    pChipCap->BssNums = 4;
    pChipCap->ExtMbssOmacStartIdx = 0x10;
    pChipCap->RepeaterStartIdx = 0x20;
#ifdef AIR_MONITOR
    pChipCap->MaxRepeaterNum = 16;
#else
    pChipCap->MaxRepeaterNum = 32;
#endif /* AIR_MONITOR */

#ifdef BCN_OFFLOAD_SUPPORT
    pChipCap->fgBcnOffloadSupport = TRUE;
#endif

    if (MTK_REV_GTE(pAd, MT7615, MT7615E3))
        pChipCap->TmrHwVer = TMR_VER_1_5;
    else
        pChipCap->TmrHwVer = TMR_VER_1_0;

#ifdef DOT11W_PMF_SUPPORT
	pChipCap->FlgPMFEncrtptMode = PMF_ENCRYPT_MODE_2;
#endif /* DOT11W_PMF_SUPPORT */

#ifdef RX_SCATTER
    if (MTK_REV_GTE(pAd, MT7615, MT7615E3))
        pChipCap->RxDMAScatter = RX_DMA_SCATTER_ENABLE;
    else
        pChipCap->RxDMAScatter = RX_DMA_SCATTER_DISABLE;
#endif /* RX_SCATTER */

	/* For calibration log buffer size limitation issue */
	pAd->fgQAtoolBatchDumpSupport = TRUE;

	rx_delay_ctl->ul_rx_dly_ctl_tbl = mt7615_rx_dly_ctl_ul_tbl;
	rx_delay_ctl->ul_rx_dly_ctl_tbl_size = (sizeof(mt7615_rx_dly_ctl_ul_tbl) / sizeof(mt7615_rx_dly_ctl_ul_tbl[0]));
	rx_delay_ctl->dl_rx_dly_ctl_tbl = mt7615_rx_dly_ctl_dl_tbl;
	rx_delay_ctl->dl_rx_dly_ctl_tbl_size = (sizeof(mt7615_rx_dly_ctl_dl_tbl) / sizeof(mt7615_rx_dly_ctl_dl_tbl[0]));

	MTWF_LOG(DBG_CAT_HW, DBG_SUBCAT_ALL, DBG_LVL_OFF, ("<--%s()\n", __FUNCTION__));
}

VOID Mt7615DisableBcnSntReq(struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev)
{
#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = NULL;	
	INT BssIdx;
	INT MaxNumBss;
	INT BandIdx = 0;
	
	MaxNumBss = pAd->ApCfg.BssidNum;

	if (wifiDev != NULL)
		BandIdx = HcGetBandByWdev(wifiDev);
	
	for(BssIdx=0; BssIdx < MaxNumBss; BssIdx++)
	{
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

		/*  Only close same band interface beacon*/
		if (wifiDev != NULL) {
			if (BandIdx != HcGetBandByWdev(wdev))
				continue;
		}

		if (wdev->bAllowBeaconing == TRUE)
		{
			wdev->bcn_buf.bBcnSntReq = FALSE;
			UpdateBeaconHandler(
				pAd,
				wdev,
				INTERFACE_STATE_CHANGE);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}

VOID Mt7615EnableBcnSntReq (struct _RTMP_ADAPTER *pAd, struct wifi_dev *wifiDev)
{
#ifdef CONFIG_AP_SUPPORT
	struct wifi_dev *wdev = NULL;			
	INT BssIdx;
	INT MaxNumBss;

	// TODO:  It need to consider wifiDev to send beacon when Scan is prepare to modify
	
	MaxNumBss = pAd->ApCfg.BssidNum;
	for(BssIdx=0; BssIdx < MaxNumBss; BssIdx++)
	{
		wdev = &pAd->ApCfg.MBSSID[BssIdx].wdev;

		if (wdev->bAllowBeaconing)
		{
			wdev->bcn_buf.bBcnSntReq = TRUE;
			UpdateBeaconHandler(
				pAd,
				wdev,
				INTERFACE_STATE_CHANGE);
		}
	}
#endif /* CONFIG_AP_SUPPORT */
	return;
}



#if defined(COMPOS_WIN) || defined(COMPOS_TESTMODE_WIN)
#else
INT Mt7615AsicArchOpsInit(RTMP_ADAPTER *pAd)
{
	RTMP_ARCH_OP *arch_ops = &pAd->archOps;

	arch_ops->archGetCrcErrCnt = MtAsicGetCrcErrCnt;
	arch_ops->archGetCCACnt = MtAsicGetCCACnt;
	arch_ops->archGetChBusyCnt = MtAsicGetChBusyCnt;

	arch_ops->archSetAutoFallBack = MtAsicSetAutoFallBack;
	arch_ops->archAutoFallbackInit = MtAsicAutoFallbackInit;
	arch_ops->archUpdateProtect = MtAsicUpdateProtectByFw;
	arch_ops->archUpdateRtsThld = MtAsicUpdateRtsThldByFw;
	arch_ops->archSwitchChannel = MtAsicSwitchChannel;
    arch_ops->archSetRDG = NULL; //MtAsicSetRDGByFw;
	arch_ops->asic_ampdu_efficiency_on_off = MtAsicAMPDUEfficiencyAdjust;
	arch_ops->asic_rts_on_off = MtAsicRTSOnOff;
#ifdef ANT_DIVERSITY_SUPPORT
	arch_ops->archAntennaSelect = MtAsicAntennaSelect;
#endif /* ANT_DIVERSITY_SUPPORT */

	arch_ops->archResetBBPAgent = MtAsicResetBBPAgent;

	arch_ops->archSetDevMac = MtAsicSetDevMacByFw;
	arch_ops->archSetBssid = MtAsicSetBssidByFw;
	arch_ops->archSetStaRec = MtAsicSetStaRecByFw;
	arch_ops->archUpdateStaRecBa = MtAsicUpdateStaRecBaByFw;

#ifdef CONFIG_AP_SUPPORT
	arch_ops->archSetMbssMode = MtAsicSetMbssMode;
#endif /* CONFIG_AP_SUPPORT */
	arch_ops->archDelWcidTab = MtAsicDelWcidTabByFw;

#ifdef HTC_DECRYPT_IOT
	arch_ops->archSetWcidAAD_OM = MtAsicSetWcidAAD_OMByFw;
#endif /* HTC_DECRYPT_IOT */


	arch_ops->archAddRemoveKeyTab = MtAsicAddRemoveKeyTabByFw;
#ifdef BCN_OFFLOAD_SUPPORT
    arch_ops->archEnableBeacon = NULL;
    arch_ops->archDisableBeacon = NULL;
#else
	arch_ops->archEnableBeacon = MtDmacAsicEnableBeacon;
	arch_ops->archDisableBeacon = MtDmacAsicDisableBeacon;
#endif

#ifdef APCLI_SUPPORT
#ifdef MAC_REPEATER_SUPPORT
	arch_ops->archSetReptFuncEnable = MtAsicSetReptFuncEnableByFw;
	arch_ops->archInsertRepeaterEntry = MtAsicInsertRepeaterEntryByFw;
	arch_ops->archRemoveRepeaterEntry = MtAsicRemoveRepeaterEntryByFw;
	arch_ops->archInsertRepeaterRootEntry = MtAsicInsertRepeaterRootEntryByFw;
#endif /* MAC_REPEATER_SUPPORT */
#endif /* APCLI_SUPPORT */

	arch_ops->archSetPiggyBack = MtAsicSetPiggyBack;
	arch_ops->archSetPreTbtt = NULL;//offload to BssInfoUpdateByFw
	arch_ops->archSetGPTimer = MtAsicSetGPTimer;
	arch_ops->archSetChBusyStat =MtAsicSetChBusyStat;
	arch_ops->archGetTsfTime = MtAsicGetTsfTimeByFirmware;
	arch_ops->archDisableSync = NULL;//MtAsicDisableSyncByDriver;
	arch_ops->archSetSyncModeAndEnable = NULL;//MtAsicEnableBssSyncByDriver;
	arch_ops->archDisableBcnSntReq = Mt7615DisableBcnSntReq;
	arch_ops->archEnableBcnSntReq = Mt7615EnableBcnSntReq;

	arch_ops->archSetWmmParam = MtAsicSetWmmParam;
	arch_ops->archGetWmmParam = MtAsicGetWmmParam;
	arch_ops->archSetEdcaParm = MtAsicSetEdcaParm;
	arch_ops->archSetRetryLimit = MtAsicSetRetryLimit;
	arch_ops->archGetRetryLimit = MtAsicGetRetryLimit;
	arch_ops->archSetSlotTime = MtAsicSetSlotTime;
	arch_ops->archGetTxTsc = MtAsicGetTxTscByDriver;
	arch_ops->archAddSharedKeyEntry = MtAsicAddSharedKeyEntry;
	arch_ops->archRemoveSharedKeyEntry = MtAsicRemoveSharedKeyEntry;
	arch_ops->archAddPairwiseKeyEntry = MtAsicAddPairwiseKeyEntry;
	arch_ops->archUpdateWCIDIVEIV = MtAsicUpdateWCIDIVEIV;
	arch_ops->archUpdateWcidAttributeEntry = MtAsicUpdateWcidAttributeEntry;
	arch_ops->archRemovePairwiseKeyEntry = MtAsicRemovePairwiseKeyEntry;
	arch_ops->archSetBW = MtAsicSetBW;
	arch_ops->archSetCtrlCh = mt_mac_set_ctrlch;

	arch_ops->archWaitMacTxRxIdle = MtAsicWaitMacTxRxIdle;
#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRxByFw;
#else
	arch_ops->archSetMacTxRx = MtAsicSetMacTxRx;
#endif /*MAC_INIT_OFFLOAD*/
	// TODO: Fix me
	//arch_ops->archSetWPDMA = MtAsicSetWPDMA;
	arch_ops->archWaitPDMAIdle = MtAsicWaitPDMAIdle;
	arch_ops->archCheckDMAIdle =MtAsicCheckDMAIdle;
	arch_ops->archSetMacWD = MtAsicSetMacWD;
#ifdef MAC_APCLI_SUPPORT
	arch_ops->archSetApCliBssid = MtAsicSetApCliBssid;
#endif /* MAC_APCLI_SUPPORT */

	arch_ops->archTOPInit = MtAsicTOPInit;
    arch_ops->archSetTmrCR = MtSetTmrCRByFw;

#ifdef CONFIG_AP_SUPPORT
	// TODO: Fix me
    arch_ops->archSetMbssWdevIfAddr = MtAsicSetMbssWdevIfAddrGen2;
    arch_ops->archSetMbssHwCRSetting = MtDmacSetMbssHwCRSetting;
    arch_ops->archSetExtTTTTHwCRSetting = MtDmacSetExtTTTTHwCRSetting;
    arch_ops->archSetExtMbssEnableCR = MtDmacSetExtMbssEnableCR;
#endif /* CONFIG_AP_SUPPORT */

#ifdef DBDC_MODE
	arch_ops->archSetDbdcCtrl = MtAsicSetDbdcCtrlByFw;
	arch_ops->archGetDbdcCtrl = MtAsicGetDbdcCtrlByFw;
#endif /*DBDC_MODE*/
	arch_ops->archUpdateRxWCIDTable = MtAsicUpdateRxWCIDTableByFw;
#ifdef TXBF_SUPPORT
    arch_ops->archUpdateClientBfCap = mt_AsicClientBfCap;
#endif /* TXBF_SUPPORT */
	arch_ops->archUpdateBASession = MtAsicUpdateBASessionByFw;
    arch_ops->archGetTidSn = MtAsicGetTidSnByDriver;
	arch_ops->archSetSMPS = MtAsicSetSMPSByFw;
#ifdef MT_PS
	arch_ops->archSetIgnorePsm = MtSetIgnorePsmByDriver;
#endif
	arch_ops->archRxHeaderTransCtl = MtAsicRxHeaderTransCtl;
	arch_ops->archRxHeaderTaranBLCtl = MtAsicRxHeaderTaranBLCtl;
	arch_ops->archFillRxBlkAndPktProcess = MTFillRxBlkAndPacketProcess;

#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetMacMaxLen = NULL;
#else
	arch_ops->archSetMacMaxLen = MtAsicSetMacMaxLen;
#endif

#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetTxStream = NULL;
#else
	arch_ops->archSetTxStream = MtAsicSetTxStream;
#endif

	arch_ops->archSetRxStream = NULL;//MtAsicSetRxStream;

#ifdef MAC_INIT_OFFLOAD
	arch_ops->archSetRxFilter = NULL;//MtAsicSetRxFilter;
#else
	arch_ops->archSetRxFilter = MtAsicSetRxFilter;
#endif

#ifdef DOT11_VHT_AC
    arch_ops->archSetRtsSignalTA = MtAsicSetRtsSignalTA;
#endif /*  DOT11_VHT_AC */

#ifdef IGMP_SNOOP_SUPPORT
	arch_ops->archMcastEntryInsert = CmdMcastEntryInsert; 
	arch_ops->archMcastEntryDelete = CmdMcastEntryDelete;
#endif

	return TRUE;
}
#endif


