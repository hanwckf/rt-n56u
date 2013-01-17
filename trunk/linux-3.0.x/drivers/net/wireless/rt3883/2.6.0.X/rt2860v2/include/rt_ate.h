/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************
 
 	Module Name:
	rt_ate.h

	Abstract:

	Revision History:
	Who			When	    What
	--------	----------  ----------------------------------------------
	Name		Date	    Modification logs
*/

#ifndef __RT_ATE_H__
#define __RT_ATE_H__

#ifdef RALINK_QA
typedef struct ate_racfghdr {
 	UINT32		magic_no;
	USHORT		command_type;
	USHORT		command_id;
	USHORT		length;
	USHORT		sequence;
	USHORT		status;
	UCHAR		data[2046];
}  __attribute__((packed))RACFGHDR, *pRACFGHDR;

/* Eth QA RACFG Command */
#define RACFG_MAGIC_NO			0x18142880
/* command id with Cmd Type == 0x0005(for iNIC)/0x0008(for others) */
#define RACFG_CMD_RF_WRITE_ALL			0x0000
#define RACFG_CMD_E2PROM_READ16			0x0001
#define RACFG_CMD_E2PROM_WRITE16		0x0002
#define RACFG_CMD_E2PROM_READ_ALL		0x0003
#define RACFG_CMD_E2PROM_WRITE_ALL		0x0004
#define RACFG_CMD_IO_READ				0x0005
#define RACFG_CMD_IO_WRITE				0x0006
#define RACFG_CMD_IO_READ_BULK			0x0007
#define RACFG_CMD_BBP_READ8				0x0008
#define RACFG_CMD_BBP_WRITE8			0x0009
#define RACFG_CMD_BBP_READ_ALL			0x000a
#define RACFG_CMD_GET_COUNTER			0x000b
#define RACFG_CMD_CLEAR_COUNTER			0x000c

#define RACFG_CMD_RSV1					0x000d
#define RACFG_CMD_RSV2					0x000e
#define RACFG_CMD_RSV3					0x000f

#define RACFG_CMD_TX_START				0x0010
#define RACFG_CMD_GET_TX_STATUS			0x0011
#define RACFG_CMD_TX_STOP				0x0012
#define RACFG_CMD_RX_START				0x0013
#define RACFG_CMD_RX_STOP				0x0014
#define RACFG_CMD_GET_NOISE_LEVEL		0x0015
#if defined(RT2883) || defined(RT3883)
#define RACFG_CMD_QUERY_BF_RSP			0x0016
#define RACFG_CMD_QUERY_IBF_TAG			0x0017
#define RACFG_CMD_QUERY_EBF_TAG			0x0018
#define RACFG_CMD_QUERY_IBF_PROFILE		0x0019
#define RACFG_CMD_QUERY_EBF_PROFILE		0x001a
#define RACFG_CMD_WRITE_IBF_TAG			0x001b
#define RACFG_CMD_WRITE_EBF_TAG			0x001c
#define RACFG_CMD_WRITE_IBF_PROFILE		0x001d
#define RACFG_CMD_WRITE_EBF_PROFILE		0x001e
#define RACFG_CMD_CALIBRATION_CAPTURE 	0x0020
#endif /* defined(RT2883) || defined(RT3883) */

#define RACFG_CMD_ATE_START				0x0080
#define RACFG_CMD_ATE_STOP				0x0081

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
#define RACFG_CMD_ATE_E2PROM_READ_BULK		0x0114
#define RACFG_CMD_ATE_E2PROM_WRITE_BULK		0x0115
#define RACFG_CMD_ATE_IO_WRITE_BULK			0x0116
#define RACFG_CMD_ATE_BBP_READ_BULK			0x0117
#define RACFG_CMD_ATE_BBP_WRITE_BULK		0x0118
#define RACFG_CMD_ATE_RF_READ_BULK			0x0119
#define RACFG_CMD_ATE_RF_WRITE_BULK			0x011a
#define RACFG_CMD_ATE_SET_TX_POWER2			0x011b
#ifdef TXBF_SUPPORT
#define RACFG_CMD_ATE_TXBF_DUT_INIT		0x011c
#define RACFG_CMD_ATE_TXBF_LNA_CAL		0x011d
#define RACFG_CMD_ATE_TXBF_DIV_CAL		0x011e
#define RACFG_CMD_ATE_TXBF_PHASE_CAL		0x011f
#define RACFG_CMD_ATE_TXBF_GOLDEN_INIT		0x0120
#define RACFG_CMD_ATE_TXBF_VERIFY		0x0121
#endif /* TXBF_SUPPORT */

#ifdef RT3883
#define RACFG_CMD_ATE_ETH_EXT_SETTING		0x0200
#endif /* RT3883 */
#if defined (RT3883) || defined (RT3352) || defined (RT5350)
#define RACFG_CMD_ATE_RUN_CPUBUSY	0x0202
#endif /* defined (RT3883) || defined (RT3352) || defined (RT5350) */

/* QA RACFG Command for ate test from localhost */
#define RACFG_CMD_ATE_SHOW_PARAM 0xff00

/* ATE export paramters to uppler layer */
typedef struct __ATE_EX_PARAM
{
	unsigned char mode;
	char TxPower0;
	char TxPower1;
#ifdef DOT11N_SS3_SUPPORT
	char TxPower2;
#endif /* DOT11N_SS3_SUPPORT */
	char TxAntennaSel;
	char RxAntennaSel;
	unsigned char DA[MAC_ADDR_LEN];
	unsigned char SA[MAC_ADDR_LEN];
	unsigned char BSSID[MAC_ADDR_LEN];
	unsigned char MCS;
	unsigned char PhyMode;
	BOOLEAN ShortGI;
	BOOLEAN BW;
	unsigned int Channel;
	unsigned int TxLength;
	unsigned int TxCount;
	unsigned int RFFreqOffset;
	unsigned int IPG;
	unsigned int RxTotalCnt;
	unsigned int RxCntPerSec;
	char LastSNR0;
	char LastSNR1;
	char LastSNR2;
	char LastRssi0;
	char LastRssi1;
	char LastRssi2;
	char AvgRssi0;
	char AvgRssi1;
	char AvgRssi2;
	short AvgRssi0X8;
	short AvgRssi1X8;
	short AvgRssi2X8;
}ATE_EX_PARAM, *pATE_EX_PARAM;

#endif /* RALINK_QA */

#define	LEN_OF_ARG 16
#define ATE_ON(_p)              (((_p)->ate.Mode) != ATE_STOP)
#define TX_METHOD_0 0 /* Early chipsets must be applied this original TXCONT/TXCARR/TXCARS mechanism. */
#define TX_METHOD_1 1 /* Default TXCONT/TXCARR/TXCARS mechanism is TX_METHOD_1 */

#define ATE_MAC_TX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_TX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 2);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_ENABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

#define ATE_MAC_RX_DISABLE(_A, _I, _pV)	\
{										\
	RTMP_IO_READ32(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 3);				\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));	\
}

/* Set MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 1. */
#define ATE_MAC_TX_CTS_ENABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) |= (1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear MAC_SYS_CTRL(0x1004) Continuous Tx Production Test (bit4) = 0. */
#define ATE_MAC_TX_CTS_DISABLE(_A, _I, _pV)	\
{											\
	RTMP_IO_READ32(_A, _I, _pV);			\
	(*(_pV)) &= ~(1 << 4);					\
	RTMP_IO_WRITE32(_A, _I, (*(_pV)));		\
}

/* Clear BBP R22 to reset Tx Mode (bit7~bit0) = 0. */
#define ATE_BBP_RESET_TX_MODE(_A, _I, _pV)			\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(0xFF);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Set BBP R22 to start Continuous Tx Mode (bit7) = 1. */
#define ATE_BBP_START_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R22 to stop Continuous Tx Mode (bit7) = 0. */
#define ATE_BBP_STOP_CTS_TX_MODE(_A, _I, _pV)		\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 7);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Set BBP R24 to send out Continuous Tx sin wave (bit0) = 1. */
#define ATE_BBP_CTS_TX_SIN_WAVE_ENABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) |= (1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

/* Clear BBP R24 to stop Continuous Tx sin wave (bit0) = 0. */
#define ATE_BBP_CTS_TX_SIN_WAVE_DISABLE(_A, _I, _pV)	\
{													\
	ATE_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV);		\
	(*(_pV)) &= ~(1 << 0);							\
	ATE_BBP_IO_WRITE8_BY_REG_ID(_A, _I, (*(_pV)));	\
}

#define ATE_CHIP_RX_VGA_GAIN_INIT(__pAd)								\
	if (__pAd->ate.pChipStruct->RxVariableGainAmplifierInit != NULL)	\
		__pAd->ate.pChipStruct->RxVariableGainAmplifierInit(__pAd)

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

#endif /* RTMP_MAC_PCI */


INT DefaultATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);

#ifdef RT3883
INT RT3883ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* RT3883 */

#ifdef RT305x
#if defined(RT3050) || defined(RT3052) || defined(RT3350)
/* This routine is also for RT3050/RT3052/RT3350 */
INT RT305xATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* defined(RT3050) || defined(RT3052) || defined(RT3350) */

#ifdef RT3352
INT RT3352ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* RT3352 */

#endif /* RT305x */


#ifdef RT2883
INT RT2883ATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* RT2883 */

#if defined(RT28xx) || defined(RT2880)
INT RT28xxATETxPwrHandler(
	IN PRTMP_ADAPTER pAd,
	IN char index);
#endif /* defined(RT28xx) || defined(RT2880) */

#ifdef RALINK_QA
VOID ATE_QA_Statistics(
	IN PRTMP_ADAPTER		pAd,
	IN PRXWI_STRUC			pRxWI,
	IN PRT28XX_RXD_STRUC    p28xxRxD,
	IN PHEADER_802_11		pHeader);
	
INT RtmpDoAte(
	IN	PRTMP_ADAPTER	pAd, 
	IN	RTMP_IOCTL_INPUT_STRUCT	*wrq,
	IN	PSTRING	wrq_name);

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
#endif /* DBG */ 
#endif /* RALINK_QA */


#ifdef RTMP_RF_RW_SUPPORT
#define ATE_RF_IO_READ8_BY_REG_ID(_A, _I, _pV)     RT30xxReadRFRegister(_A, _I, _pV)
#define ATE_RF_IO_WRITE8_BY_REG_ID(_A, _I, _V)     RT30xxWriteRFRegister(_A, _I, _V)
#endif /* RTMP_RF_RW_SUPPORT */

VOID rt_ee_read_all(
	IN  PRTMP_ADAPTER   pAd,
	OUT USHORT *Data);

VOID rt_ee_write_all(
	IN  PRTMP_ADAPTER   pAd,
	IN  USHORT *Data);

VOID rt_ee_write_bulk(
	IN  PRTMP_ADAPTER	pAd,
	IN  USHORT *Data,
	IN  USHORT offset,
	IN  USHORT length);

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

INT Set_ADCDump_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#ifdef RTMP_INTERNAL_TX_ALC
INT Set_ATE_TSSI_CALIBRATION_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_ATE_TSSI_CALIBRATION_EX_Proc(
	IN      PRTMP_ADAPTER   pAd,
	IN      PSTRING                 arg);


#if defined(RT3350) || defined(RT3352)
CHAR InsertTssi(
	IN UCHAR InChannel,
	IN UCHAR Channel0,
	IN UCHAR Channel1,
	IN CHAR Tssi0,
	IN CHAR Tssi1);

INT RT335xATETssiCalibrationExtend(
	IN struct _RTMP_ADAPTER		*pAd,
	IN PSTRING					arg);
#endif /* defined(RT3350) || defined(RT3352) */

CHAR ATEGetDesiredTSSI(
	IN PRTMP_ADAPTER		pAd);

#endif /* RTMP_INTERNAL_TX_ALC */

#ifdef HW_ANTENNA_DIVERSITY_SUPPORT
INT Set_ATE_DIV_ANTENNA_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT Set_ATE_DIV_ANTENNA_CALIBRATION_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif /* HW_ANTENNA_DIVERSITY_SUPPORT */

#ifdef RTMP_TEMPERATURE_COMPENSATION
INT Set_ATE_READ_EXTERNAL_TSSI_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);
#endif /* RTMP_TEMPERATURE_COMPENSATION */

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
#endif /* DOT11N_SS3_SUPPORT */

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
#endif /* RT3350 */

INT	Set_ATE_TX_FREQOFFSET_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Default_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef RT3883
INT	RT3883_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* RT3883 */

#ifdef RT305x
#if defined(RT3050) || defined(RT3052)
INT	RT305x_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* defined(RT3050) || defined(RT3052) */

#ifdef RT3350
INT	RT3350_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* RT3350 */

#ifdef RT3352
INT	RT3352_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* RT3352 */

#endif /* RT305x */


#ifdef RT2883
INT	RT2883_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* RT2883 */

#if defined(RT28xx) || defined(RT2880)
INT	RT28xx_Set_ATE_TX_BW_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* defined(RT28xx) || defined(RT2880) */

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
#endif /* RTMP_RF_RW_SUPPORT */

INT Set_ATE_Load_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_ATE_Read_E2P_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef LED_CONTROL_SUPPORT
#endif /* LED_CONTROL_SUPPORT */

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
#endif /* TXBF_SUPPORT */

INT	Set_ATE_Show_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ATE_Help_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

VOID ATEAsicAdjustTxPower(
	IN PRTMP_ADAPTER pAd);

VOID ATESampleRssi(
	IN PRTMP_ADAPTER	pAd,
	IN PRXWI_STRUC		pRxWI);	


NDIS_STATUS ChipStructAssign(
 IN	PRTMP_ADAPTER pAd);

NDIS_STATUS ATEInit(
 IN PRTMP_ADAPTER pAd);

#ifdef RALINK_QA
VOID ReadQATxTypeFromBBP(
 IN	PRTMP_ADAPTER	pAd);
#endif /* RALINK_QA */

NDIS_STATUS ATEBBPWriteWithRxChain(
 IN RTMP_ADAPTER *pAd,
 IN UCHAR bbpId,
 IN CHAR bbpVal,
 IN RX_CHAIN_IDX rx_ch_idx);

#ifdef RT3883
VOID RT3883ATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* RT3883 */

#ifdef RT305x
#ifdef RT3050
VOID RT3050ATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* RT3050 */


#ifdef RT3350
VOID RT3350ATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* RT3350 */

#ifdef RT3352
VOID RT3352ATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* RT3352 */

#endif /* RT305x */


#ifdef RT2883
VOID RT2883ATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* RT2883 */

#if defined(RT28xx) || defined(RT2880)
VOID RT28xxATERxVariableGainAmplifierInit(
 IN PRTMP_ADAPTER		pAd);
#endif /* defined(RT28xx) || defined(RT2880) */

#endif /* __RT_ATE_H__ */

