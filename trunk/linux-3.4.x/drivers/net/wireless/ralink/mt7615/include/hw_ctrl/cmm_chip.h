/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering	the source code	is stricitly prohibited, unless	the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	cmm_chip.h

	Abstract:
	Ralink Wireless Chip HW related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/


#ifndef __CMM_CHIP_H__
#define __CMM_CHIP_H__

struct _EXT_CMD_EFUSE_BUFFER_MODE_T;
struct _RTMP_ADAPTER;
struct _MAC_TABLE_ENTRY;
struct _CIPHER_KEY;
struct _MT_TX_COUNTER;
struct _EDCA_PARM;
struct _RTMP_REG_PAIR;
struct _BANK_RF_REG_PAIR;
struct _R_M_W_REG;
struct _RF_R_M_W_REG;
struct _CR_REG;
struct _REG_PAIR;
struct MT_TX_PWR_CAP;
struct _EXT_CMD_CHAN_SWITCH_T;
struct wifi_dev;


#ifdef COMPOS_WIN
// TODO: Star-usw, fix it
typedef char RTMP_STRING;
#endif /* COMPOS_WIN */


enum ASIC_CAP {
    fASIC_CAP_RX_SC = 0x1,
    fASIC_CAP_CSO = 0x2,
    fASIC_CAP_TSO = 0x4,
    fASIC_CAP_MCS_LUT = 0x8,

    fASIC_CAP_PMF_ENC = 0x10,
	fASIC_CAP_DBDC = 0x20,
	fASIC_CAP_TX_HDR_TRANS = 0x40,
	fASIC_CAP_RX_HDR_TRANS = 0x80,
	fASIC_CAP_BA_OFFLOAD = 0x100,
	fASIC_CAP_HW_DAMSDU = 0x200,
	fASIC_CAP_IGMP_SNOOP_OFFLOAD = 0x400,
	fASIC_CAP_WMM_PKTDETECT_OFFLOAD = 0x00000800,
};

enum PHY_CAP {
    fPHY_CAP_24G = 0x1,
    fPHY_CAP_5G = 0x2,

    fPHY_CAP_HT = 0x10,
    fPHY_CAP_VHT = 0x20,

    fPHY_CAP_TXBF = 0x100,
    fPHY_CAP_LDPC = 0x200,
	fPHY_CAP_MUMIMO = 0x400,

	fPHY_CAP_BW40 = 0x1000,
	fPHY_CAP_BW80 = 0x2000,
	fPHY_CAP_BW160NC = 0x4000,
	fPHY_CAP_BW160C = 0x8000,
};

enum HIF_TYPE {
    HIF_RTMP = 0x0,
    HIF_RLT = 0x1,
    HIF_MT = 0x2,
    HIF_MAX = HIF_MT,
};

enum MAC_TYPE {
    MAC_RTMP = 0x0,
    MAC_MT = 0x1,
};

enum RF_TYPE {
    RF_RT,
    RF_RLT,
    RF_MT76x2,
    RF_MT,
};

enum BBP_TYPE {
    BBP_RTMP = 0x0,
    BBP_RLT = 0x1,
    BBP_MT = 0x2,
};


#define PHY_CAP_2G(_x)		(((_x) & fPHY_CAP_24G) == fPHY_CAP_24G)
#define PHY_CAP_5G(_x)		(((_x) & fPHY_CAP_5G) == fPHY_CAP_5G)
#define PHY_CAP_N(_x)		(((_x) & fPHY_CAP_HT) == fPHY_CAP_HT)
#define PHY_CAP_AC(_x)		(((_x) & fPHY_CAP_VHT) == fPHY_CAP_VHT)


enum EFUSE_TYPE {
	EFUSE_MT,
	EFUSE_MAX,
} ;

#define MBSSID_MODE0 	0
#define MBSSID_MODE1 	1	/* Enhance NEW MBSSID MODE mapping to mode 0 */
#ifdef ENHANCE_NEW_MBSSID_MODE
#define MBSSID_MODE2	2	/* Enhance NEW MBSSID MODE mapping to mode 1 */
#define MBSSID_MODE3	3	/* Enhance NEW MBSSID MODE mapping to mode 2 */
#define MBSSID_MODE4	4	/* Enhance NEW MBSSID MODE mapping to mode 3 */
#define MBSSID_MODE5	5	/* Enhance NEW MBSSID MODE mapping to mode 4 */
#define MBSSID_MODE6	6	/* Enhance NEW MBSSID MODE mapping to mode 5 */
#endif /* ENHANCE_NEW_MBSSID_MODE */

enum APPS_MODE {
    APPS_MODE0 = 0x0,	/* MT7603, host handle APPS */
    APPS_MODE1 = 0x1,	/* MT7637 */
    APPS_MODE2 = 0x2,	/* MT7615, FW handle APPS */
    APPS_MODEMAX = 0x3,
};

#ifdef RX_SCATTER
#define RX_DMA_SCATTER_DISABLE 0
#define RX_DMA_SCATTER_ENABLE 1
#endif /* RX_SCATTER */


#ifdef MT_MAC
/*
    these functions is common setting and could be used by sMAC or dMAC.
    move to here to common use.
*/
#ifdef CONFIG_AP_SUPPORT
VOID MtAsicSetMbssWdevIfAddrGen1(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
VOID MtAsicSetMbssWdevIfAddrGen2(struct _RTMP_ADAPTER *pAd, INT idx, UCHAR *if_addr, INT opmode);
#endif /*CONFIG_AP_SUPPROT */
#endif /*MT_MAC*/

typedef struct _RTMP_CHIP_OP {
	int (*sys_onoff)(struct _RTMP_ADAPTER *pAd, BOOLEAN on, BOOLEAN reser);

	/*  Calibration access related callback functions */
	int (*eeinit)(struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*eeread)(struct _RTMP_ADAPTER *pAd, UINT16 offset, UINT16 *pValue);
	int (*eewrite)(struct _RTMP_ADAPTER *pAd, USHORT offset, USHORT value);
	BOOLEAN (*eeread_range)(struct _RTMP_ADAPTER *pAd, UINT16 start, UINT16 length, UCHAR *pbuf);
	int (*eewrite_range)(struct _RTMP_ADAPTER *pAd, USHORT start, USHORT length, UCHAR *pbuf);


	/* ITxBf calibration */
	int (*fITxBfDividerCalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, UCHAR *divPhase);
	void (*fITxBfLNAPhaseCompensate)(struct _RTMP_ADAPTER *pAd);
	int (*fITxBfCal)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	int (*fITxBfLNACalibration)(struct _RTMP_ADAPTER *pAd, int calFunction, int calMethod, BOOLEAN gBand);

	/* MCU related callback functions */
	int (*load_rom_patch)(struct _RTMP_ADAPTER *ad);
	int (*erase_rom_patch)(struct _RTMP_ADAPTER *ad);
	int (*loadFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*eraseFirmware)(struct _RTMP_ADAPTER *pAd);
	int (*restartFirmware)(struct _RTMP_ADAPTER *pAd);
#ifdef MT7615    
#if defined(RTMP_PCI_SUPPORT)
	int (*resetFirmware)(struct _RTMP_ADAPTER *pAd);
#endif
#endif
    int (*sendCommandToMcu)(struct _RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1, BOOLEAN FlgIsNeedLocked);	/* int (*sendCommandToMcu)(RTMP_ADAPTER *pAd, UCHAR cmd, UCHAR token, UCHAR arg0, UCHAR arg1); */
#ifdef CONFIG_ANDES_SUPPORT
	int (*sendCommandToAndesMcu)(struct _RTMP_ADAPTER *pAd, UCHAR QueIdx, UCHAR cmd, UCHAR *pData, USHORT DataLen, BOOLEAN FlgIsNeedLocked);
#endif

	void (*AsicRfInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicBbpInit)(struct _RTMP_ADAPTER *pAd);
	void (*AsicMacInit)(struct _RTMP_ADAPTER *pAd);

	void (*AsicRfTurnOn)(struct _RTMP_ADAPTER *pAd);
	void (*AsicRfTurnOff)(struct _RTMP_ADAPTER *pAd);
	void (*AsicReverseRfFromSleepMode)(struct _RTMP_ADAPTER *pAd, BOOLEAN FlgIsInitState);
	void (*AsicHaltAction)(struct _RTMP_ADAPTER *pAd);

	/* Power save */
#ifdef GREENAP_SUPPORT	
        VOID (*EnableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
        VOID (*DisableAPMIMOPS)(struct _RTMP_ADAPTER *pAd, struct greenap_on_off_ctrl *greenap_on_off);
#endif /* GREENAP_SUPPORT */

	INT (*PwrSavingOP)(struct _RTMP_ADAPTER *pAd, UINT32 PwrOP, UINT32 PwrLevel,
							UINT32 ListenInterval, UINT32 PreTBTTLeadTime,
							UINT8 TIMByteOffset, UINT8 TIMBytePattern);

	/* Chip tuning */
	VOID (*RxSensitivityTuning)(IN struct _RTMP_ADAPTER *pAd);

	/* MAC */
	VOID (*BeaconUpdate)(struct _RTMP_ADAPTER *pAd, USHORT Offset, UINT32 Value, UINT8 Unit);

	/* BBP adjust */
	VOID (*ChipBBPAdjust)(IN struct _RTMP_ADAPTER *pAd, UCHAR Channel);

	/* AGC */
	VOID (*ChipAGCInit)(struct _RTMP_ADAPTER *pAd, UCHAR bw);
	UCHAR (*ChipAGCAdjust)(struct _RTMP_ADAPTER *pAd, CHAR Rssi, UCHAR OrigR66Value);
	VOID (*BbpInitFromEEPROM)(struct _RTMP_ADAPTER *pAd);

	VOID (*ChipSwitchChannel)(struct _RTMP_ADAPTER *pAd, struct _MT_SWITCH_CHANNEL_CFG SwChCfg);

#ifdef NEW_SET_RX_STREAM
    INT (*ChipSetRxStream)(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
#endif

	/* EDCCA */
	VOID (*ChipSetEDCCA)(struct _RTMP_ADAPTER *pAd, BOOLEAN bOn);

	/* IQ Calibration */
	VOID (*ChipIQCalibration)(struct _RTMP_ADAPTER *pAd, UCHAR Channel);

	/* TX ALC */
	UINT32 (*TSSIRatio)(INT32 delta_power);
	VOID (*InitDesiredTSSITable)(IN struct _RTMP_ADAPTER *pAd, UCHAR Channel);
	int (*ATETssiCalibration)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	int (*ATETssiCalibrationExtend)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);
	int (*ATEReadExternalTSSI)(struct _RTMP_ADAPTER *pAd, RTMP_STRING *arg);

	VOID (*AsicTxAlcGetAutoAgcOffset)(
				IN struct _RTMP_ADAPTER	*pAd,
				IN PCHAR				pDeltaPwr,
				IN PCHAR				pTotalDeltaPwr,
				IN PCHAR				pAgcCompensate,
				IN PCHAR 				pDeltaPowerByBbpR1,
				IN UCHAR 				Channel);

	VOID (*AsicGetTxPowerOffset)(struct _RTMP_ADAPTER *pAd, ULONG *TxPwr);
	VOID (*AsicExtraPowerOverMAC)(struct _RTMP_ADAPTER *pAd);

	VOID (*AsicAdjustTxPower)(struct _RTMP_ADAPTER *pAd);

	/* Antenna */
	VOID (*AsicAntennaDefaultReset)(struct _RTMP_ADAPTER *pAd, union _EEPROM_ANTENNA_STRUC *pAntenna);
	VOID (*SetRxAnt)(struct _RTMP_ADAPTER *pAd, UCHAR Ant);

	/* EEPROM */
	VOID (*NICInitAsicFromEEPROM)(IN struct _RTMP_ADAPTER *pAd);

	/* Temperature Compensation */
	VOID (*InitTemperCompensation)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*TemperCompensation)(IN struct _RTMP_ADAPTER *pAd);

	/* high power tuning */
	VOID (*HighPowerTuning)(struct _RTMP_ADAPTER *pAd, struct _RSSI_SAMPLE *pRssi);

	/* Others */
	VOID (*NetDevNickNameInit)(IN struct _RTMP_ADAPTER *pAd);
#ifdef CAL_FREE_IC_SUPPORT
	BOOLEAN (*is_cal_free_ic)(IN struct _RTMP_ADAPTER *pAd);
	VOID (*cal_free_data_get)(IN struct _RTMP_ADAPTER *pAd);
	BOOLEAN (*check_is_cal_free_merge)(IN struct _RTMP_ADAPTER *pAd);
#endif /* CAL_FREE_IC_SUPPORT */

#ifdef RF_LOCKDOWN
    BOOLEAN (*check_RF_lock_down)(IN struct _RTMP_ADAPTER *pAd);
    BOOLEAN (*write_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
    BOOLEAN (*merge_RF_lock_parameter)(IN struct _RTMP_ADAPTER *pAd);
    UCHAR   (*Read_Effuse_parameter)(IN struct _RTMP_ADAPTER *pAd, IN USHORT offset);
    BOOLEAN (*Config_Effuse_Country)(IN struct _RTMP_ADAPTER *pAd);
#endif /* RF_LOCKDOWN */

	/* The chip specific function list */

	VOID (*AsicResetBbpAgent)(IN struct _RTMP_ADAPTER *pAd);

#ifdef ANT_DIVERSITY_SUPPORT
	VOID (*HwAntEnable)(struct _RTMP_ADAPTER *pAd);
#endif /* ANT_DIVERSITY_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	VOID (*ToneRadarProgram)(struct _RTMP_ADAPTER *pAd, ULONG  threshold);
#endif /* CARRIER_DETECTION_SUPPORT */
	VOID (*CckMrcStatusCtrl)(struct _RTMP_ADAPTER *pAd);
	VOID (*RadarGLRTCompensate)(struct _RTMP_ADAPTER *pAd);
	VOID (*SecondCCADetection)(struct _RTMP_ADAPTER *pAd);

	/* MCU */
	void (*MCUCtrlInit)(struct _RTMP_ADAPTER *ad);
	void (*MCUCtrlExit)(struct _RTMP_ADAPTER *ad);
	VOID (*FwInit)(struct _RTMP_ADAPTER *pAd);
	VOID (*FwExit)(struct _RTMP_ADAPTER *pAd);
	int (*BurstWrite)(struct _RTMP_ADAPTER *ad, UINT32 Offset, UINT32 *Data, UINT32 Cnt);
	int (*BurstRead)(struct _RTMP_ADAPTER *ad, UINT32 Offset, UINT32 Cnt, UINT32 *Data);
	int (*RandomRead)(struct _RTMP_ADAPTER *ad,struct  _RTMP_REG_PAIR *RegPair, UINT32 Num);
	int (*RFRandomRead)(struct _RTMP_ADAPTER *ad, struct _BANK_RF_REG_PAIR *RegPair, UINT32 Num);
	int (*ReadModifyWrite)(struct _RTMP_ADAPTER *ad, struct _R_M_W_REG *RegPair, UINT32 Num);
	int (*RFReadModifyWrite)(struct _RTMP_ADAPTER *ad, struct _RF_R_M_W_REG *RegPair, UINT32 Num);
	int (*RandomWrite)(struct _RTMP_ADAPTER *ad, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);
	int (*RFRandomWrite)(struct _RTMP_ADAPTER *ad, struct _BANK_RF_REG_PAIR *RegPair, UINT32 Num);
#ifdef CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT
	int (*BBPRandomWrite)(struct _RTMP_ADAPTER *ad, struct _RTMP_REG_PAIR *RegPair, UINT32 Num);
#endif /* CONFIG_ANDES_BBP_RANDOM_WRITE_SUPPORT */
	int (*sc_random_write)(struct _RTMP_ADAPTER *ad, struct _CR_REG *table, UINT32 num, UINT32 flags);
	int (*sc_rf_random_write)(struct _RTMP_ADAPTER *ad, struct _BANK_RF_CR_REG *table, UINT32 num, UINT32 flags);
	int (*DisableTxRx)(struct _RTMP_ADAPTER *ad, UCHAR Level);
	void (*AsicRadioOn)(struct _RTMP_ADAPTER *ad, UCHAR Stage);
	void (*AsicRadioOff)(struct _RTMP_ADAPTER *ad, UINT8 Stage);
#ifdef RLT_MAC
	// TODO: shiang-usw, currently the "ANDES_CALIBRATION_PARAM" is defined in andes_rlt.h
	int (*Calibration)(struct _RTMP_ADAPTER *pAd, UINT32 CalibrationID, ANDES_CALIBRATION_PARAM *param);
#endif /* RLT_MAC */
#ifdef CONFIG_ANDES_SUPPORT
#if defined(RTMP_PCI_SUPPORT) || defined(RTMP_RBUS_SUPPORT)
	int (*pci_kick_out_cmd_msg)(struct _RTMP_ADAPTER *ad, struct cmd_msg *msg);
#endif

	void (*andes_fill_cmd_header)(struct cmd_msg *msg, VOID *net_pkt);
	INT32 (*AndesMTChkCrc)(struct _RTMP_ADAPTER *pAd, UINT32 checksum_len);
	UINT16 (*AndesMTGetCrc)(struct _RTMP_ADAPTER *pAd);
#endif /* CONFIG_ANDES_SUPPORT */
	void (*rx_event_handler)(struct _RTMP_ADAPTER *ad, UCHAR *data);

#ifdef MICROWAVE_OVEN_SUPPORT
	VOID (*AsicMeasureFalseCCA)(IN struct _RTMP_ADAPTER *pAd);

	VOID (*AsicMitigateMicrowave)(IN struct _RTMP_ADAPTER *pAd);
#endif /* MICROWAVE_OVEN_SUPPORT */

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
	VOID (*AsicWOWEnable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWDisable)(struct _RTMP_ADAPTER *ad, struct _STA_ADMIN_CONFIG *pStaCfg);
	VOID (*AsicWOWInit)(struct _RTMP_ADAPTER *ad);
#endif /* (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) */

	void (*usb_cfg_read)(struct _RTMP_ADAPTER *ad, UINT32 *value);
	void (*usb_cfg_write)(struct _RTMP_ADAPTER *ad, UINT32 value);
	void (*show_pwr_info)(struct _RTMP_ADAPTER *ad);
	void (*cal_test)(struct _RTMP_ADAPTER *ad, UINT32 type);
	void (*bufferModeEfuseFill)(struct _RTMP_ADAPTER *ad, struct _EXT_CMD_EFUSE_BUFFER_MODE_T *pCmd);
	INT32 (*MtCmdTx)(struct _RTMP_ADAPTER *pAd,struct cmd_msg *msg);
	void (*fw_prepare)(struct _RTMP_ADAPTER *pAd);
#ifdef DBDC_MODE
	UCHAR (*BandGetByIdx)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx);
#endif

#ifdef TXBF_SUPPORT
    VOID (*TxBFInit)(struct _RTMP_ADAPTER *ad, struct _MAC_TABLE_ENTRY *pEntry, struct _IE_lists *ie_list, BOOLEAN supportsETxBF);
    BOOLEAN (*ClientSupportsVhtETxBF)(struct _RTMP_ADAPTER *ad, struct _VHT_CAP_INFO *pTxBFCap);
    BOOLEAN (*ClientSupportsETxBF)(struct _RTMP_ADAPTER *ad, struct _HT_BF_CAP *pTxBFCap);
    VOID (*setETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
    VOID (*setVHTETxBFCap)(struct _RTMP_ADAPTER *ad, struct _TXBF_STATUS_INFO  *pTxBfInfo);
#ifdef MT_MAC
    INT32 (*BfStaRecUpdate)(struct _RTMP_ADAPTER *ad, UCHAR ucPhyMode, UCHAR ucBssIdx, UINT8 ucWlanIdx);
    INT32 (*BfStaRecRelease)(struct _RTMP_ADAPTER *ad, UCHAR ucBssIdx, UINT8 ucWlanIdx);
    INT32 (*BfPfmuMemAlloc)(struct _RTMP_ADAPTER *ad, UCHAR ucSu_Mu, UCHAR ucWlanId);
    INT32 (*BfPfmuMemRelease)(struct _RTMP_ADAPTER *ad, UCHAR ucWlanId);
    INT32 (*BfHwEnStatusUpdate)(struct _RTMP_ADAPTER *ad, BOOLEAN fgETxBf, BOOLEAN fgITxBf);
    INT32 (*TxBfTxApplyCtrl)(struct _RTMP_ADAPTER *pAd, UCHAR ucWlanId, BOOLEAN fgETxBf, BOOLEAN fgITxBf, BOOLEAN fgMuTxBf, BOOLEAN fgPhaseCali);
    INT32 (*archSetAid)(struct _RTMP_ADAPTER *pAd, UINT16 Aid);
    INT32 (*BfApClientCluster)(struct _RTMP_ADAPTER *pAd, UCHAR ucWlanId, UCHAR ucCmmWlanId);
    INT32 (*BfReptClonedStaToNormalSta)(struct _RTMP_ADAPTER *pAd, UCHAR ucWlanId, UCHAR ucCliIdx);
    INT32 (*BfeeHwCtrl)(struct _RTMP_ADAPTER *pAd, BOOLEAN fgBfeeEn);
#endif /* MT_MAC */
#endif /* TXBF_SUPPORT */
#ifdef SMART_CARRIER_SENSE_SUPPORT
    VOID (*SmartCarrierSense)(struct _RTMP_ADAPTER *pAd);
    VOID (*ChipSetSCS)(struct _RTMP_ADAPTER *pAd, UCHAR BandIdx, UINT32 value);
#endif /* SMART_CARRIER_SENSE_SUPPORT */
	INT32 (*hif_io_read32)(void *cookie,UINT32 addr,UINT32 *value);
	INT32 (*hif_io_write32)(void *cookie,UINT32 addr,UINT32 value);
	VOID (*heart_beat_check)(struct _RTMP_ADAPTER *ad);
	VOID (*hif_set_pcie_read_params)(struct _RTMP_ADAPTER *pAd);
}RTMP_CHIP_OP;


/*
	2860: 28xx
	2870: 28xx

	30xx:
		3090
		3070
		2070 3070

	33xx:	30xx
		3390 3090
		3370 3070

	35xx:	30xx
		3572, 2870, 28xx
		3062, 2860, 28xx
		3562, 2860, 28xx

	3593, 28xx, 30xx, 35xx

	< Note: 3050, 3052, 3350 can not be compiled simultaneously. >
	305x:
		3052
		3050
		3350, 3050

	3352: 305x

	2880: 28xx
	2883:
	3883:
*/
typedef struct _RTMP_CHIP_CAP {
	/* ------------------------ packet --------------------- */
	UINT8 TXWISize;	// TxWI or LMAC TxD max size
	UINT8 RXWISize; // RxWI or LMAC RxD max size
	UINT8 tx_hw_hdr_len;	// Tx Hw meta info size which including all hw info fields
	UINT8 rx_hw_hdr_len;	// Rx Hw meta info size

	enum ASIC_CAP asic_caps;
	enum PHY_CAP phy_caps;
	enum HIF_TYPE hif_type;
	enum MAC_TYPE mac_type;
	enum BBP_TYPE bbp_type;
	enum MCU_TYPE MCUType;
	enum RF_TYPE rf_type;
	enum EFUSE_TYPE efuse_type;

	/* register */
	struct _REG_PAIR *pRFRegTable;
	struct _REG_PAIR *pBBPRegTable;
	UCHAR bbpRegTbSize;

	UINT32 MaxNumOfRfId;
	UINT32 MaxNumOfBbpId;

#define RF_REG_WT_METHOD_NONE			0
#define RF_REG_WT_METHOD_STEP_ON		1
	UCHAR RfReg17WtMethod;

	/* beacon */
	BOOLEAN FlgIsSupSpecBcnBuf;	/* SPECIFIC_BCN_BUF_SUPPORT */
	UINT8 BcnMaxNum;	/* software use */
	UINT8 BcnMaxHwNum;	/* hardware limitation */
	UINT8 WcidHwRsvNum;	/* hardware available WCID number */
	UINT32 WtblHwNum;		/* hardware WTBL number */
	UINT32 WtblPseAddr;		/* */
	UINT16 BcnMaxHwSize;	/* hardware maximum beacon size */
	UINT16 BcnBase[HW_BEACON_MAX_NUM];	/* hardware beacon base address */

	/* function */
	/* use UINT8, not bit-or to speed up driver */
	BOOLEAN FlgIsHwWapiSup;

	/* VCO calibration mode */
	UINT8 VcoPeriod; /* default 10s */
#define VCO_CAL_DISABLE		0	/* not support */
#define VCO_CAL_MODE_1		1	/* toggle RF7[0] */
#define VCO_CAL_MODE_2		2	/* toggle RF3[7] */
#define VCO_CAL_MODE_3		3	/* toggle RF4[7] or RF5[7] */
	UINT8	FlgIsVcoReCalMode;

	BOOLEAN FlgIsHwAntennaDiversitySup;
	BOOLEAN Flg7662ChipCap;
#ifdef STREAM_MODE_SUPPORT
	BOOLEAN FlgHwStreamMode;
#endif /* STREAM_MODE_SUPPORT */
#ifdef TXBF_SUPPORT
	BOOLEAN FlgHwTxBfCap;
	BOOLEAN FlgITxBfBinWrite;
#endif /* TXBF_SUPPORT */
#ifdef FIFO_EXT_SUPPORT
	BOOLEAN FlgHwFifoExtCap;
#endif /* FIFO_EXT_SUPPORT */

	UCHAR ba_max_cnt;

#ifdef RTMP_MAC_PCI
#endif /* RTMP_MAC_PCI */

	BOOLEAN temp_tx_alc_enable;
	INT32 temp_25_ref; /* a quantification value of temperature, but not J */
	INT32 current_temp; /* unit J */
#ifdef RTMP_TEMPERATURE_TX_ALC
	UINT32 high_temp_slope_a_band; /* slope with unit (J /dB) */
	UINT32 low_temp_slope_a_band; /* slope with unit (J /dB) */
	UINT32 high_temp_slope_g_band; /* slope with unit (J /dB) */
	UINT32 low_temp_slope_g_band; /* slope with unit (J /dB) */
	INT32 tc_upper_bound_a_band; /* unit dB */
	INT32 tc_lower_bound_a_band; /* unit dB */
	INT32 tc_upper_bound_g_band; /* unit dB */
	INT32 tc_lower_bound_g_band; /* unit dB */
#endif /* RTMP_TEMPERATURE_TX_ALC */

#ifdef TXRX_SW_ANTDIV_SUPPORT
	BOOLEAN bTxRxSwAntDiv;
#endif /* TXRX_SW_ANTDIV_SUPPORT */

#ifdef DYNAMIC_VGA_SUPPORT
	BOOLEAN dynamic_vga_support;
	INT32 compensate_level;
	INT32 avg_rssi_0;
	INT32 avg_rssi_1;
	INT32 avg_rssi_all;
	UCHAR dynamic_chE_mode;
	BOOLEAN dynamic_chE_trigger;
#endif /* DYNAMIC_VGA_SUPPORT */

	/* ---------------------------- signal ---------------------------------- */
#define SNR_FORMULA1		0	/* ((0xeb     - pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA2		1	/* (pAd->StaCfg[0].wdev.LastSNR0 * 3 + 8) >> 4; */
#define SNR_FORMULA3		2	/* (pAd->StaCfg[0].wdev.LastSNR0) * 3) / 16; */
#define SNR_FORMULA4		3	/* for MT7603 */
	UINT8 SnrFormula;

	UINT8 max_nss;			/* maximum Nss, 3 for 3883 or 3593 */
	UINT8 max_bw160_nss;		/* maximum Nss for BW160 & 80+80, may not equal to BW80 */
	UINT8 max_vht_mcs;		/* Maximum Vht MCS */
	UINT8 max_mpdu_len;	/* Maximum ASIC capable MPDU length */

#ifdef DOT11_VHT_AC
	UINT8 ac_off_mode;		/* 11AC off mode */
#endif /* DOT11_VHT_AC */

	BOOLEAN bTempCompTxALC;
	BOOLEAN rx_temp_comp;

	BOOLEAN bLimitPowerRange; /* TSSI compensation range limit */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION)
	UINT8 TxAlcTxPowerUpperBound_2G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_2G;
#ifdef A_BAND_SUPPORT
	UINT8 TxAlcTxPowerUpperBound_5G;
	const TX_POWER_TUNING_ENTRY_STRUCT *TxPowerTuningTable_5G;
#endif /* A_BAND_SUPPORT */

#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(RTMP_TEMPERATURE_COMPENSATION) */

#if defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2)
	INT16	PAModeCCK[4];
	INT16	PAModeOFDM[8];
	INT16	PAModeHT[16];
#ifdef DOT11_VHT_AC
	INT16	PAModeVHT[10];
#endif /* DOT11_VHT_AC */
#endif /* defined(RTMP_INTERNAL_TX_ALC) || defined(SINGLE_SKU_V2) */


	/* ---------------------------- others ---------------------------------- */
#ifdef RTMP_EFUSE_SUPPORT
	UINT16 EFUSE_USAGE_MAP_START;
	UINT16 EFUSE_USAGE_MAP_END;
	UINT8 EFUSE_USAGE_MAP_SIZE;
	UINT8 EFUSE_RESERVED_SIZE;
#endif /* RTMP_EFUSE_SUPPORT */

	UCHAR *EEPROM_DEFAULT_BIN;
	UCHAR *EEPROM_DEFAULT_BIN_FILE;
	UINT16 EEPROM_DEFAULT_BIN_SIZE;
    UINT16 EFUSE_BUFFER_CONTENT_SIZE;

#ifdef RTMP_FLASH_SUPPORT
	BOOLEAN ee_inited;
#endif /* RTMP_FLASH_SUPPORT */
#ifdef CARRIER_DETECTION_SUPPORT
	UCHAR carrier_func;
#endif /* CARRIER_DETECTION_SUPPORT */
#ifdef DFS_SUPPORT
	UINT8 DfsEngineNum;
#endif /* DFS_SUPPORT */

	/*
		Define the burst size of WPDMA of PCI
		0 : 4 DWORD (16bytes)
		1 : 8 DWORD (32 bytes)
		2 : 16 DWORD (64 bytes)
		3 : 32 DWORD (128 bytes)
	*/
	UINT8 WPDMABurstSIZE;

	/*
 	 * 0: MBSSID_MODE0
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit2:bit0] of byte5)
 	 * 1: MBSSID_MODE1
 	 * (The multiple MAC_ADDR/BSSID are distinguished by [bit4:bit2] of byte0)
 	 */
	UINT8 MBSSIDMode;

#ifdef DOT11W_PMF_SUPPORT
#define PMF_ENCRYPT_MODE_0      0	/* All packets must software encryption. */
#define PMF_ENCRYPT_MODE_1      1	/* Data packets do hardware encryption, management packet do software encryption. */
#define PMF_ENCRYPT_MODE_2      2	/* Data and management packets do hardware encryption. */
	UINT8	FlgPMFEncrtptMode;
#endif /* DOT11W_PMF_SUPPORT */


#ifdef RT5592EP_SUPPORT
	UINT32 Priv; /* Flag for RT5592 EP */
#endif /* RT5592EP_SUPPORT */


#ifdef CONFIG_ANDES_SUPPORT
	UINT32 WlanMemmapOffset;
	UINT32 InbandPacketMaxLen; /* must be 48 multible */
	UINT8 CmdRspRxRing;
	BOOLEAN IsComboChip;
	BOOLEAN need_load_fw;
	BOOLEAN need_load_rom_patch;
	UINT8 ram_code_protect;
	UINT8 rom_code_protect;
	UINT8 load_iv;
	UINT32 ilm_offset;
	UINT32 dlm_offset;
	UINT32 rom_patch_offset;

#endif
       // TODO: Star.....
	UINT8 DownLoadType;

	UINT8 cmd_header_len;
	UINT8 cmd_padding_len;

#ifdef SINGLE_SKU_V2
	CHAR	Apwrdelta;
	CHAR	Gpwrdelta;
#endif /* SINGLE_SKU_V2 */


#ifdef CONFIG_SWITCH_CHANNEL_OFFLOAD
	UINT16 ChannelParamsSize;
	UCHAR *ChannelParam;
	INT XtalType;
#endif

	UCHAR load_code_method;
	UCHAR *FwImgBuf;
	UCHAR *fw_header_image;
	UCHAR *fw_bin_file_name;
	UCHAR *rom_patch;
	UCHAR *rom_patch_header_image;
	UCHAR *rom_patch_bin_file_name;
	UINT32 rom_patch_len;
	UINT32 fw_len;
	UCHAR *MACRegisterVer;
	UCHAR *BBPRegisterVer;
	UCHAR *RFRegisterVer;

#ifdef MT7615
	UCHAR *fw_header_image_ext; /* CR4 firmware */
    UINT32 fw_len_ext; /* CR4 firmware */
#endif

#ifdef CONFIG_WIFI_TEST
	UINT32 MemMapStart;
	UINT32 MemMapEnd;
	UINT32 BBPMemMapOffset;
	UINT16 BBPStart;
	UINT16 BBPEnd;
	UINT16 RFStart;
	UINT16 RFEnd;
#ifdef RLT_RF
	UINT8 RFBankNum;
	struct RF_BANK_OFFSET *RFBankOffset;
#endif
#ifdef MT_RF
	UINT8 RFIndexNum;
	struct RF_INDEX_OFFSET *RFIndexOffset;
#endif
	UINT32 MacMemMapOffset;
	UINT32 MacStart;
	UINT32 MacEnd;
	UINT16 E2PStart;
	UINT16 E2PEnd;
	BOOLEAN pbf_loopback;
	BOOLEAN pbf_rx_drop;
#endif /* CONFIG_WIFI_TEST */

	BOOLEAN tssi_enable;
	BOOLEAN ed_cca_enable;

#ifdef MT_MAC
	struct MT_TX_PWR_CAP MTTxPwrCap;
	UCHAR TmrEnable;
	UINT8 OmacNums;
	UINT8 BssNums;
	UINT8 MBSSStartIdx;
    UINT8 MaxRepeaterNum;
	UINT8 ExtMbssOmacStartIdx;
	UINT8 RepeaterStartIdx;
#endif
    BOOLEAN fgBcnOffloadSupport;
    UCHAR TmrHwVer;



    UINT8 TxAggLimit;
	UINT8 TxBAWinSize;
	UINT8 RxBAWinSize;
	UINT8 AMPDUFactor;
#ifdef DOT11_VHT_AC
#define MPDU_3895_OCTETS    0x0
#define MPDU_7991_OCTETS    0x1
#define MPDU_11454_OCTETS   0x2
    UINT8 MaxMPDULength;
    UINT8 MaxAMPDULengthExp; /* 2^(13+Exp) - 1 */
#endif /* DOT11_VHT_AC*/

    BOOLEAN g_band_256_qam;

#ifdef RACTRL_FW_OFFLOAD_SUPPORT
    BOOLEAN fgRateAdaptFWOffload;
#endif /* RACTRL_FW_OFFLOAD_SUPPORT */


    UINT32  TxOPScenario;

    UINT32  CurrentTxOP;
    UINT32  default_txop;
	/*Multi-WMM Control*/
	UCHAR WmmHwNum;
    /* specific PDA Port HW Address */
    UINT16 PDA_PORT;
    BOOLEAN SupportAMSDU;
	BOOLEAN BATriggerOffload;

    UINT8 APPSMode;

#if (defined(WOW_SUPPORT) && defined(RTMP_MAC_USB)) || defined(NEW_WOW_SUPPORT) || defined(MT_WOW_SUPPORT)
    UINT8   nWakeupInterface;
#endif /* MT_WOW_SUPPORT */

#ifdef RX_SCATTER
	BOOLEAN RxDMAScatter;
#endif /* RX_SCATTER */

	/* the length of partial payload delivered to MCU for further processing */
	UINT16 CtParseLen;
}RTMP_CHIP_CAP;

#endif

