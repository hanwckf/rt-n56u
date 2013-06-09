#ifndef __ATE_H__
#define __ATE_H__

#ifdef RALINK_28xx_QA
/* command id with Cmd Type == 0x0008(for 28xx)/0x0005(for iNIC) */
#define RACFG_CMD_RF_WRITE_ALL		0x0000
#define RACFG_CMD_E2PROM_READ16		0x0001
#define RACFG_CMD_E2PROM_WRITE16	0x0002
#define RACFG_CMD_E2PROM_READ_ALL	0x0003
#define RACFG_CMD_E2PROM_WRITE_ALL	0x0004
#define RACFG_CMD_IO_READ			0x0005
#define RACFG_CMD_IO_WRITE			0x0006
#define RACFG_CMD_IO_READ_BULK		0x0007
#define RACFG_CMD_BBP_READ8			0x0008
#define RACFG_CMD_BBP_WRITE8		0x0009
#define RACFG_CMD_BBP_READ_ALL		0x000a
#define RACFG_CMD_GET_COUNTER		0x000b
#define RACFG_CMD_CLEAR_COUNTER		0x000c

#define RACFG_CMD_RSV1				0x000d
#define RACFG_CMD_RSV2				0x000e
#define RACFG_CMD_RSV3				0x000f

#define RACFG_CMD_TX_START			0x0010
#define RACFG_CMD_GET_TX_STATUS		0x0011
#define RACFG_CMD_TX_STOP			0x0012
#define RACFG_CMD_RX_START			0x0013
#define RACFG_CMD_RX_STOP			0x0014
#define RACFG_CMD_GET_NOISE_LEVEL	0x0015
#if defined(RT2883) || defined(RT3883)
#define RACFG_CMD_QUERY_BF_RSP		0x0016
#define RACFG_CMD_QUERY_IBF_TAG		0x0017
#define RACFG_CMD_QUERY_EBF_TAG		0x0018
#define RACFG_CMD_QUERY_IBF_PROFILE	0x0019
#define RACFG_CMD_QUERY_EBF_PROFILE	0x001a
#define RACFG_CMD_WRITE_IBF_TAG		0x001b
#define RACFG_CMD_WRITE_EBF_TAG		0x001c
#define RACFG_CMD_WRITE_IBF_PROFILE	0x001d
#define RACFG_CMD_WRITE_EBF_PROFILE	0x001e
#define RACFG_CMD_CALIBRATION_CAPTURE 0x0020
#endif // defined(RT2883) || defined(RT3883) //

#define RACFG_CMD_ATE_START			0x0080
#define RACFG_CMD_ATE_STOP			0x0081

#define RACFG_CMD_ATE_START_TX_CARRIER		0x0100
#define RACFG_CMD_ATE_START_TX_CONT			0x0101
#define RACFG_CMD_ATE_START_TX_FRAME		0x0102
#define RACFG_CMD_ATE_SET_BW	            0x0103
#define RACFG_CMD_ATE_SET_TX_POWER0	        0x0104
#define RACFG_CMD_ATE_SET_TX_POWER1			0x0105
#define RACFG_CMD_ATE_SET_FREQ_OFFSET		0x0106
#define RACFG_CMD_ATE_GET_STATISTICS		0x0107
#define RACFG_CMD_ATE_RESET_COUNTER			0x0108
#define RACFG_CMD_ATE_SEL_TX_ANTENNA		0x0109
#define RACFG_CMD_ATE_SEL_RX_ANTENNA		0x010a
#define RACFG_CMD_ATE_SET_PREAMBLE			0x010b
#define RACFG_CMD_ATE_SET_CHANNEL			0x010c
#define RACFG_CMD_ATE_SET_ADDR1				0x010d
#define RACFG_CMD_ATE_SET_ADDR2				0x010e
#define RACFG_CMD_ATE_SET_ADDR3				0x010f
#define RACFG_CMD_ATE_SET_RATE				0x0110
#define RACFG_CMD_ATE_SET_TX_FRAME_LEN		0x0111
#define RACFG_CMD_ATE_SET_TX_FRAME_COUNT	0x0112
#define RACFG_CMD_ATE_START_RX_FRAME		0x0113
#define RACFG_CMD_ATE_E2PROM_READ_BULK	0x0114
#define RACFG_CMD_ATE_E2PROM_WRITE_BULK	0x0115
#define RACFG_CMD_ATE_IO_WRITE_BULK		0x0116
#define RACFG_CMD_ATE_BBP_READ_BULK		0x0117
#define RACFG_CMD_ATE_BBP_WRITE_BULK	0x0118
#define RACFG_CMD_ATE_RF_READ_BULK		0x0119
#define RACFG_CMD_ATE_RF_WRITE_BULK		0x011a
#define RACFG_CMD_ATE_SET_TX_POWER2			0x011b
#ifdef TXBF_SUPPORT
#define RACFG_CMD_ATE_TXBF_DUT_INIT		0x011c
#define RACFG_CMD_ATE_TXBF_LNA_CAL		0x011d
#define RACFG_CMD_ATE_TXBF_DIV_CAL		0x011e
#define RACFG_CMD_ATE_TXBF_PHASE_CAL		0x011f
#define RACFG_CMD_ATE_TXBF_GOLDEN_INIT		0x0120
#define RACFG_CMD_ATE_TXBF_VERIFY		0x0121
#endif // TXBF_SUPPORT //

#ifdef RT3883
#define RACFG_CMD_ATE_ETH_EXT_SETTING	0x0200
#endif // RT3883 //


#endif // RALINK_28xx_QA



#define ATE_ON(_p)              (((_p)->ate.Mode) != ATE_STOP)

#ifdef RTMP_MAC_PCI
NTSTATUS	ATE_RT3562WriteBBPR66(
	IN	PRTMP_ADAPTER	pAd,
	IN	UCHAR			Value);

#define ATEPCIReadBBPRegister(_A, _I, _pV)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             j, k;                               \
    for (j=0; j<MAX_BUSY_COUNT; j++)                    \
    {                                                   \
        RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
        if (BbpCsr.field.Busy == BUSY)                  \
        {                                               \
            continue;                                   \
        }                                               \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 1;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.RegNum = _I;                       \
        RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
        for (k=0; k<MAX_BUSY_COUNT; k++)                \
        {                                               \
            RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word); \
            if (BbpCsr.field.Busy == IDLE)              \
                break;                                  \
        }                                               \
        if ((BbpCsr.field.Busy == IDLE) &&              \
            (BbpCsr.field.RegNum == _I))                \
        {                                               \
            *(_pV) = (UCHAR)BbpCsr.field.Value;         \
            break;                                      \
        }                                               \
    }                                                   \
    if (BbpCsr.field.Busy == BUSY)                      \
    {                                                   \
        DBGPRINT(RT_DEBUG_ERROR, ("BBP read R%d fail\n", _I));      \
        *(_pV) = (_A)->BbpWriteLatch[_I];               \
    }                                                   \
}

#define ATEPCIWriteBBPRegister(_A, _I, _V)        \
{                                                       \
    BBP_CSR_CFG_STRUC  BbpCsr;                             \
    int             BusyCnt;                            \
    for (BusyCnt=0; BusyCnt<MAX_BUSY_COUNT; BusyCnt++)  \
    {                                                   \
        RTMP_IO_READ32(_A, BBP_CSR_CFG, &BbpCsr.word);     \
        if (BbpCsr.field.Busy == BUSY)                  \
            continue;                                   \
        BbpCsr.word = 0;                                \
        BbpCsr.field.fRead = 0;                         \
        BbpCsr.field.BBP_RW_MODE = 1;                         \
        BbpCsr.field.Busy = 1;                          \
        BbpCsr.field.Value = _V;                        \
        BbpCsr.field.RegNum = _I;                       \
        RTMP_IO_WRITE32(_A, BBP_CSR_CFG, BbpCsr.word);     \
        (_A)->BbpWriteLatch[_I] = _V;                   \
        break;                                          \
    }                                                   \
    if (BusyCnt == MAX_BUSY_COUNT)                      \
    {                                                   \
        DBGPRINT(RT_DEBUG_ERROR, ("BBP write R%d fail\n", _I));     \
    }                                                   \
}

#define ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)		ATEPCIReadBBPRegister(_A, _I, _pV)

#define ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _V)		ATEPCIWriteBBPRegister(_A, _I, _V)

#endif // RTMP_MAC_PCI //


#ifdef RTMP_RF_RW_SUPPORT
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _I, _pV)     RT30xxReadRFRegister(_A, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _I, _V)     RT30xxWriteRFRegister(_A, _I, _V)
#endif // RTMP_RF_RW_SUPPORT //

VOID rt_ee_read_all(
	IN  PRTMP_ADAPTER   pAd,
	OUT USHORT *Data);

VOID rt_ee_write_all(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT *Data);

INT Set_ATE_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_DA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_SA_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_BSSID_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_CHANNEL_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_INIT_CHAN_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ATE_TX_POWER0_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TX_POWER1_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef DOT11N_SS3_SUPPORT
INT	Set_ATE_TX_POWER2_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif // DOT11N_SS3_SUPPORT //

INT	Set_ATE_TX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ATE_RX_Antenna_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

#ifdef RT3350
INT	Set_ATE_PA_Bias_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // RT3350 //

INT	Set_ATE_TX_FREQOFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TX_LENGTH_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TX_COUNT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TX_MCS_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ATE_TX_MODE_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ATE_TX_GI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);


INT	Set_ATE_RX_FER_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Read_RF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifndef RTMP_RF_RW_SUPPORT
INT Set_ATE_Write_RF1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Write_RF2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Write_RF3_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Write_RF4_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // RTMP_RF_RW_SUPPORT //

INT Set_ATE_Load_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Read_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);


INT	Set_ATE_AUTO_ALC_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_IPG_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Payload_Proc(
    IN  PRTMP_ADAPTER   pAd, 
    IN  PSTRING         arg);

#ifdef TXBF_SUPPORT
INT	Set_ATE_TXBF_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TXSOUNDING_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_TXBF_DIVCAL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_ATE_TXBF_LNACAL_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_ATE_TXBF_INIT_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_TXBF_CAL_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_TXBF_GOLDEN_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_TXBF_VERIFY_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_TXBF_VERIFY_NoComp_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_ForceBBP_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // TXBF_SUPPORT //

INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_Help_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef RALINK_28xx_QA
VOID ATE_QA_Statistics(
	IN PRTMP_ADAPTER		pAd,
	IN PRXWI_STRUC			pRxWI,
	IN PRT28XX_RXD_STRUC    p28xxRxD,
	IN PHEADER_802_11		pHeader);
	
INT RtmpDoAte(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	struct iwreq	*wrq);

VOID BubbleSort(
	IN  INT32 n,
	IN  INT32 a[]);

VOID CalNoiseLevel(
	IN  PRTMP_ADAPTER   pAdapter,
	IN  UCHAR           channel,
	OUT INT32           buffer[3][10]);

BOOLEAN SyncTxRxConfig(
	IN	PRTMP_ADAPTER	pAdapter, 
	IN	USHORT			offset, 
	IN	UCHAR			value);

INT Set_TxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_RxStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DBG
INT Set_EERead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_EEWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_BBPRead_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_BBPWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_RFWrite_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif // DBG // 
#endif // RALINK_28xx_QA //


VOID ATEAsicSwitchChannel(
	IN PRTMP_ADAPTER pAd); 

VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);

VOID ATEDisableAsicProtect(
	IN		PRTMP_ADAPTER	pAd);

CHAR ATEConvertToRssi(
	IN PRTMP_ADAPTER  pAd,
	IN CHAR				Rssi,
	IN UCHAR    RssiNumber);

VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI);

#ifdef CONFIG_AP_SUPPORT
VOID ATEAPStop(
	IN PRTMP_ADAPTER pAd);
#endif // CONFIG_AP_SUPPORT //

#ifdef CONFIG_STA_SUPPORT
VOID RTMPStationStop(
    IN  PRTMP_ADAPTER   pAd);

VOID RTMPStationStart(
    IN  PRTMP_ADAPTER   pAd);
#endif // CONFIG_STA_SUPPORT //
#endif // __ATE_H__ //
