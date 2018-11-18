
#ifndef _MT_TESTMODE_H
#define _MT_TESTMODE_H


#define TESTMODE_WCID_BAND0 2
#define TESTMODE_WCID_BAND1 3

#define ATE_SINGLE_BAND	1
#define ATE_DUAL_BAND	2
#define ATE_GBAND_TYPE	1
#define ATE_ABAND_TYPE	2

#define ATE_BAND_WIDTH_20		0
#define ATE_BAND_WIDTH_40		1
#define ATE_BAND_WIDTH_80		2
#define ATE_BAND_WIDTH_10		3
#define ATE_BAND_WIDTH_5		4
#define ATE_BAND_WIDTH_160		5
#define ATE_BAND_WIDTH_8080		6

enum {
	ATE_USER_PAYLOAD,
	ATE_FIXED_PAYLOAD,
	ATE_RANDOM_PAYLOAD,
};

typedef struct _ATE_DATA_RATE_MAP {
    UCHAR   mcs;            /* MCS index */
    UINT32  tx_data_rate;   /* Data rate in K Bit */
} ATE_DATA_RATE_MAP;

typedef struct _ATE_ANT_MAP {
	UINT32 ant_sel;
	UINT32 spe_idx;
} ATE_ANT_MAP;

typedef struct _ATE_TXPWR_GROUP_MAP {
	UINT32 start;
	UINT32 end;
	UINT32 group[4];
} ATE_TXPWR_GROUP_MAP;

typedef struct _ATE_CH_KHZ_MAP {
	UINT32 Channel;
	UINT32 Freq;
} ATE_CH_KHZ_MAP;

#if defined(COMPOS_TESTMODE_WIN)// for MT_testmode.c

#define INC_RING_INDEX(_idx, _RingSize)          \
    {                                            \
        (_idx)++;                                \
        if ((_idx) >= (_RingSize)) { _idx = 0; } \
    }

#define BAND_WIDTH_20		0
#define BAND_WIDTH_40		1
#define BAND_WIDTH_80		2
#define BAND_WIDTH_160		3
#define BAND_WIDTH_10		4	/* 802.11j has 10MHz. This definition is for internal usage. doesn't fill in the IE or other field. */
#define BAND_WIDTH_BOTH     5	/* BW20 + BW40 */
#define BAND_WIDTH_5		6
#define BAND_WIDTH_8080     7

#define TX1_G_BAND_TARGET_PWR 0x5E
#define TX0_G_BAND_TARGET_PWR 0x58
enum {
	PDMA_TX,
	PDMA_RX,
	PDMA_TX_RX,
};
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) (&_pstruct->_member)
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) (_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) (_pstruct->_member = _val)
#ifndef COMPOS_TESTMODE_WIN//NDIS only
#define MAC_ADDR_LEN    6
// 2-byte Frame control field
typedef	struct	{
	UINT16		Ver:2;				// Protocol version
	UINT16		Type:2;				// MSDU type
	UINT16		SubType:4;			// MSDU subtype
	UINT16		ToDs:1;				// To DS indication
	UINT16		FrDs:1;				// From DS indication
	UINT16		MoreFrag:1;			// More fragment bit
	UINT16		Retry:1;			// Retry status bit
	UINT16		PwrMgmt:1;			// Power management bit
	UINT16		MoreData:1;			// More data bit
	UINT16		Wep:1;				// Wep data
	UINT16		Order:1;			// Strict order expected
}	QAFRAME_CONTROL, *PQAFRAME_CONTROL;

typedef	struct	_QAHEADER_802_11	{
	QAFRAME_CONTROL   FC;
	UINT16          Duration;
	UCHAR           Addr1[MAC_ADDR_LEN];
	UCHAR           Addr2[MAC_ADDR_LEN];
	UCHAR			Addr3[MAC_ADDR_LEN];
	UINT16			Frag:4;
	UINT16			Sequence:12;
}	QAHEADER_802_11, *PQAHEADER_802_11;
#endif /* NOT COMPOS_TESTMODE_WIN */
#else
#ifdef DBDC_MODE
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) ((_band)?_pstruct->band_ext[_band-1]._member:_pstruct->_member)
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) ((_band)?&_pstruct->band_ext[_band-1]._member:&_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) ({	\
	UINT32 _ret=_val;													\
	if(_band){ 														\
		BAND_INFO *_info = &(_pstruct->band_ext[_band-1]);			\
		_info->_member = _val;										\
	} else															\
		_pstruct->_member = _val;									\
	_ret;															\
})
#else
#define TESTMODE_GET_PADDR(_pstruct, _band, _member) (&_pstruct->_member)
#define TESTMODE_GET_PARAM(_pstruct, _band, _member) (_pstruct->_member)
#define TESTMODE_SET_PARAM(_pstruct, _band, _member, _val) ({	\
	UINT32 _ret=_val;											\
	_pstruct->_member = _val;									\
	_ret;															\
})
#endif /* DBDC_MODE */
#endif /* defined(COMPOS_TESTMODE_WIN) */

typedef enum _TESTMODE_STAT_TYPE{
	 TESTMODE_RXV,
	 TESTMODE_PER_PKT,
	 TESTMODE_RESET_CNT,
     TESTMODE_COUNTER_802_11,
	 TESTMODE_STAT_TYPE_NUM,
} TESTMODE_STAT_TYPE;

typedef struct _RATE_TO_BE_FIX{
	UINT32	TXRate:6;
	UINT32	TXMode:3;
	UINT32	Nsts:2;
	UINT32	STBC:1;
	UINT32	Reserved:20;
}RATE_TO_BE_FIX, *PRATE_TO_BE_FIX;

struct rssi_offset_eeprom {
	UINT32 **rssi_eeprom_band_offset;
	UINT32 *n_band_offset;
	UINT32 n_band;
};

INT32 MT_ATERFTestCB(struct _RTMP_ADAPTER *pAd, UINT8 *Data, UINT32 Length);
INT32 MT_SetATEMPSDump(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);
INT32 MtTestModeInit(struct _RTMP_ADAPTER *pAd);
INT32 MtTestModeExit(struct _RTMP_ADAPTER *pAd);
INT MtTestModeBkCr(struct _RTMP_ADAPTER *pAd, ULONG offset, enum _TEST_BK_CR_TYPE type);
INT MtTestModeRestoreCr(struct _RTMP_ADAPTER *pAd, ULONG offset);
#if !defined(COMPOS_WIN)
INT32 MT_ATETxControl(struct _RTMP_ADAPTER *pAd, UINT32 band_idx, PNDIS_PACKET pkt);
#endif
VOID MT_ATEUpdateRxStatistic(struct _RTMP_ADAPTER *pAd, TESTMODE_STAT_TYPE type, VOID *data);
INT Mt_TestModeInsertPeer(struct _RTMP_ADAPTER *pAd, UINT32 band_ext, CHAR *da, CHAR *sa, CHAR *bss);
INT32 MT_ATETxPkt(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);	//Export for Loopback
UINT8 MT_ATEGetBandIdxByIf(struct _RTMP_ADAPTER *pAd);
UINT8 MT_ATEGetWDevIdxByBand(struct _RTMP_ADAPTER *pAd, UINT32 band_idx);
INT MtATESetMacTxRx(struct _RTMP_ADAPTER *pAd, INT32 TxRx, BOOLEAN Enable,UCHAR BandIdx);
INT MtATESetTxStream(struct _RTMP_ADAPTER *pAd, UINT32 StreamNums, UCHAR BandIdx);
INT MtATESetRxPath(struct _RTMP_ADAPTER *pAd, UINT32 RxPathSel, UCHAR BandIdx);
INT MtATESetRxFilter(struct _RTMP_ADAPTER *pAd, MT_RX_FILTER_CTRL_T filter);
INT32 MT_ATEDumpLog(struct _RTMP_ADAPTER *pAd, struct _ATE_LOG_DUMP_CB *log_cb, UINT32 log_type);
VOID MtCmdATETestResp(struct cmd_msg *msg, char *data, UINT16 len);
INT32 MtATECh2Freq(UINT32 Channel, UINT32 band_idx);
INT32 MtATEGetTxPwrGroup(UINT32 Channel, UINT32 band_idx, UINT32 Ant_idx);
INT32 MT_ATEInsertLog(struct _RTMP_ADAPTER *pAd, UCHAR *log, UINT32 log_type, UINT32 len);
#if !defined(COMPOS_WIN) && !defined(COMPOS_TESTMODE_WIN)// for MT_testmode.c
INT MT_ATERxDoneHandle(struct _RTMP_ADAPTER *pAd, RX_BLK *pRxBlk);
#endif
INT32 MtATERSSIOffset(struct _RTMP_ADAPTER *pAd, INT32 RSSI_org, UINT32 RSSI_idx, INT32 Ch_Band);
#ifdef PRE_CAL_TRX_SET1_SUPPORT
INT MtATE_DPD_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
INT MtATE_DCOC_Cal_Store_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);
#endif /* PRE_CAL_TRX_SET1_SUPPORT */

#ifdef PRE_CAL_TRX_SET2_SUPPORT
INT MtATE_Pre_Cal_Proc(RTMP_ADAPTER *pAd, UINT8 CalId, UINT32 ChGrpId);
#endif /* PRE_CAL_TRX_SET2_SUPPORT */

#ifdef PA_TRIM_SUPPORT
INT MtATE_PA_Trim_Proc(RTMP_ADAPTER *pAd, PUINT32 pData);
#endif /* PA_TRIM_SUPPORT */

#define MT_ATEInit( _pAd) ({		\
	UINT32 _ret;					\
    _ret = MtTestModeInit(_pAd);	\
	_ret;							\
})

#define MT_ATEExit( _pAd) ({		\
	UINT32 _ret;					\
    _ret = MtTestModeExit(_pAd);	\
	_ret;							\
})
#endif //_MT_TESTMODE_H
