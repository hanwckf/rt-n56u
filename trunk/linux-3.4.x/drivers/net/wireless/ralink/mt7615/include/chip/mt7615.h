#ifndef __MT7615_H__
#define __MT7615_H__

#include "mcu/andes_core.h"
#include "phy/mt_rf.h"


struct _RTMP_ADAPTER;
struct _MT_SWITCH_CHANNEL_CFG;

#define MAX_RF_ID	127
#define MAC_RF_BANK 7

#define MT7615_MT_WTBL_SIZE	128
#define MT7615_MT_WMM_SIZE	4
#define MT7615_PDA_PORT		0xf800

#define MT7615_CT_PARSE_LEN	72
#define MT7615_BIN_FILE_PATH "/etc/Wireless/WIFI_RAM_CODE_MT7615.bin"

#ifdef RF_LOCKDOWN
#define COUNTRY_CODE_BYTE0_EEPROME_OFFSET   0x11C
#define COUNTRY_CODE_BYTE1_EEPROME_OFFSET   0x11D

#define COUNTRY_REGION_2G_EEPROME_OFFSET    0x11E
#define COUNTRY_REGION_5G_EEPROME_OFFSET    0x11F
#define COUNTRY_REGION_VALIDATION_MASK      BIT(7)
#define COUNTRY_REGION_VALIDATION_OFFSET    7
#define COUNTRY_REGION_CONTENT_MASK         BITS(0,6)

#define RF_LOCKDOWN_EEPROME_BLOCK_OFFSET    0x120
#define RF_LOCKDOWN_EEPROME_COLUMN_OFFSET   0xC
#define RF_LOCKDOWN_EEPROME_BIT             BIT(7)
#define RF_LOCKDOWN_EEPROME_BIT_OFFSET      7
#define RF_VALIDATION_NUMBER                10
#endif /* RF_LOCKDOWN */

#define EEPROM_DBDC_ANTENNA_CFG_OFFSET      0x12F
#define DBDC_2G_RX_MASK                     BITS(0,1)
#define DBDC_2G_RX_OFFSET                   0
#define DBDC_2G_TX_MASK                     BITS(2,3)
#define DBDC_2G_TX_OFFSET                   2
#define DBDC_5G_RX_MASK                     BITS(4,5)
#define DBDC_5G_RX_OFFSET                   4
#define DBDC_5G_TX_MASK                     BITS(6,7)
#define DBDC_5G_TX_OFFSET                   6

void mt7615_init(struct _RTMP_ADAPTER *pAd);
void mt7615_get_tx_pwr_per_rate(struct _RTMP_ADAPTER *pAd);
void mt7615_get_tx_pwr_info(struct _RTMP_ADAPTER *pAd);
void mt7615_antenna_sel_ctl(struct _RTMP_ADAPTER *pAd);
int mt7615_read_chl_pwr(struct _RTMP_ADAPTER *pAd);
void mt7615_pwrOn(struct _RTMP_ADAPTER *pAd);
void mt7615_calibration(struct _RTMP_ADAPTER *pAd, UCHAR channel);
void mt7615_tssi_compensation(struct _RTMP_ADAPTER *pAd, UCHAR channel);

#ifdef SINGLE_SKU_V2
VOID mt_FillBfBackOffParameter(struct _RTMP_ADAPTER *pAd, UINT8 Nsstream, UINT8 *BFGainTable);
VOID mt_FillBFBackoff(struct _RTMP_ADAPTER *pAd,UINT8 channel, UCHAR Band, UINT8 *BFPowerBackOff);
#endif /* SINGLE_SKU_V2 */

#ifdef MT7615_FPGA
INT mt7615_chk_top_default_cr_setting(struct _RTMP_ADAPTER *pAd);
INT mt7615_chk_hif_default_cr_setting(struct _RTMP_ADAPTER *pAd);
#endif /* MT7615_FPGA */

typedef enum _ENUM_SUBF_CAP_T {
    SUBF_OFF,
    SUBF_ALL,
    SUBF_BFER,
    SUBF_BFEE
} ENUM_SUBF_CAP_T, *P_ENUM_SUBF_CAP_T;

typedef enum _ENUM_MUBF_CAP_T {
    MUBF_OFF,
    MUBF_BFER,
    MUBF_BFEE,
    MUBF_ALL
} ENUM_MUBF_CAP_T, *P_ENUM_MUBF_CAP_T;

typedef enum _ENUM_BF_BACKOFF_TYPE_T {
    BF_BACKOFF_4T = 4,
    BF_BACKOFF_3T = 3,
    BF_BACKOFF_2T = 2
} ENUM_BF_BACKOFF_TYPE_T, *P_ENUM_BF_BACKOFF_TYPE_T;


INT Mt7615AsicArchOpsInit(struct _RTMP_ADAPTER *pAd);

#ifdef PRE_CAL_TRX_SET1_SUPPORT
enum {
	GBAND=0,
	ABAND=1,
};

void ShowDCOCData(struct _RTMP_ADAPTER *pAd, RXDCOC_RESULT_T RxDcocResult);
void ShowDPDData(struct _RTMP_ADAPTER *pAd, TXDPD_RESULT_T TxDPDResult);
void mt7615_apply_cal_data(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#if defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
/* RXDCOC */
#ifdef RTMP_FLASH_SUPPORT
#define DCOC_FLASH_OFFSET	1024
#else
#define DCOC_FLASH_OFFSET	0
#endif

extern UINT16 K_A20Freq[];
extern UINT16 K_A40Freq[];
extern UINT16 K_A80Freq[];
extern UINT16 K_G20Freq[];
extern UINT16 K_AllFreq[];

extern UINT16 K_A20_SIZE;
extern UINT16 K_A40_SIZE;
extern UINT16 K_A80_SIZE;
extern UINT16 K_G20_SIZE;
extern UINT16 K_ALL_SIZE;

/* TXDPD */
#define DCOC_OFFSET 		(DCOC_FLASH_OFFSET)
#define DPDPART1_OFFSET		(DCOC_OFFSET + K_ALL_SIZE * RXDCOC_SIZE)
#define DPDPART2_OFFSET		(DCOC_OFFSET + K_ALL_SIZE * RXDCOC_SIZE + 36 * TXDPD_SIZE)
#define TXDPD_PART1_LIMIT 36  /* 36*216 won't exceed 8K , array size exceeding 8K will cause problem */

extern UINT16 DPD_A20Freq[];
extern UINT16 DPD_G20Freq[];
extern UINT16 DPD_AllFreq[];
   
extern UINT16 DPD_A20_SIZE;
extern UINT16 DPD_G20_SIZE;
extern UINT16 DPD_ALL_SIZE;

#endif /* defined(PRE_CAL_TRX_SET1_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) || defined(RLM_CAL_CACHE_SUPPORT) */

#if defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT)
/* Pre-Cal */
#define PRECALPART_OFFSET    (DPDPART1_OFFSET + DPD_ALL_SIZE * TXDPD_SIZE)
#define PRE_CAL_SIZE     (16 * 1024) 
#endif /* defined(RLM_CAL_CACHE_SUPPORT) || defined(PRE_CAL_TRX_SET2_SUPPORT) */

#ifdef PA_TRIM_SUPPORT
#define PA_TRIM_OFFSET        0
#define PA_TRIM_SIZE          16
#endif /* PA_TRIM_SUPPORT */
#endif // __MT7615_H__

