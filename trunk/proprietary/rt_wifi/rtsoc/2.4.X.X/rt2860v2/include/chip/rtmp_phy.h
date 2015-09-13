/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology	5th	Rd.
 * Science-based Industrial	Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2004, Ralink Technology, Inc.
 *
 * All rights reserved.	Ralink's source	code is	an unpublished work	and	the
 * use of a	copyright notice does not imply	otherwise. This	source code
 * contains	confidential trade secret material of Ralink Tech. Any attemp
 * or participation	in deciphering,	decoding, reverse engineering or in	any
 * way altering	the	source code	is stricitly prohibited, unless	the	prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

	Module Name:
	rtmp_phy.h

	Abstract:
	Ralink Wireless Chip PHY(BBP/RF) related definition & structures

	Revision History:
	Who			When		  What
	--------	----------	  ----------------------------------------------
*/

#ifndef __RTMP_PHY_H__
#define __RTMP_PHY_H__


/*
	RF sections
*/
#define RF_R00			0
#define RF_R01			1
#define RF_R02			2
#define RF_R03			3
#define RF_R04			4
#define RF_R05			5
#define RF_R06			6
#define RF_R07			7
#define RF_R08			8
#define RF_R09			9
#define RF_R10			10
#define RF_R11			11
#define RF_R12			12
#define RF_R13			13
#define RF_R14			14
#define RF_R15			15
#define RF_R16			16
#define RF_R17			17
#define RF_R18			18
#define RF_R19			19
#define RF_R20			20
#define RF_R21			21
#define RF_R22			22
#define RF_R23			23
#define RF_R24			24
#define RF_R25			25
#define RF_R26			26
#define RF_R27			27
#define RF_R28			28
#define RF_R29			29
#define RF_R30			30
#define RF_R31			31
#define	RF_R32			32
#define	RF_R33			33
#define	RF_R34			34
#define	RF_R35			35
#define	RF_R36			36
#define	RF_R37			37
#define	RF_R38			38
#define	RF_R39			39
#define	RF_R40			40
#define	RF_R41			41
#define	RF_R42			42
#define	RF_R43			43
#define	RF_R44			44
#define	RF_R45			45
#define	RF_R46			46
#define	RF_R47			47
#define	RF_R48			48
#define	RF_R49			49
#define	RF_R50			50
#define	RF_R51			51
#define	RF_R52			52
#define	RF_R53			53
#define	RF_R54			54
#define	RF_R55			55
#define	RF_R56			56
#define	RF_R57			57
#define	RF_R58			58
#define	RF_R59			59
#define	RF_R60			60
#define	RF_R61			61
#define	RF_R62			62
#define	RF_R63			63


// value domain of pAd->RfIcType
#define RFIC_2820                   1       // 2.4G 2T3R
#define RFIC_2850                   2       // 2.4G/5G 2T3R
#define RFIC_2720                   3       // 2.4G 1T2R
#define RFIC_2750                   4       // 2.4G/5G 1T2R
#define RFIC_3020                   5       // 2.4G 1T1R
#define RFIC_2020                   6       // 2.4G B/G
#define RFIC_3021                   7       // 2.4G 1T2R
#define RFIC_3022                   8       // 2.4G 2T2R
#define RFIC_3052                   9       // 2.4G/5G 2T2R
#define RFIC_2853					10		// 2.4G.5G 3T3R
#define RFIC_3320                   11      // 2.4G 1T1R with PA (RT3350/RT3370/RT3390)
#define RFIC_3322                   12      // 2.4G 2T2R with PA (RT3352/RT3371/RT3372/RT3391/RT3392)
#define RFIC_3053                   13      // 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662)
#define RFIC_3853                   13      // 2.4G/5G 3T3R (RT3883/RT3563/RT3573/RT3593/RT3662)

/*
	BBP sections
*/
#define BBP_R0			0  // version
#define BBP_R1			1  // TSSI
#define BBP_R2			2  // TX configure
#define BBP_R3			3
#define BBP_R4			4
#define BBP_R5			5
#define BBP_R6			6
#define BBP_R14			14 // RX configure
#define BBP_R16			16
#define BBP_R17			17 // RX sensibility
#define BBP_R18			18
#define BBP_R21			21
#define BBP_R22			22
#define BBP_R24			24
#define BBP_R25			25
#define BBP_R26			26
#define BBP_R27			27
#define BBP_R31			31
#define BBP_R47			47
#define BBP_R49			49 //TSSI
#define BBP_R50			50
#define BBP_R51			51
#define BBP_R52			52
#define BBP_R53			53
#define BBP_R54			54
#define BBP_R55			55
#define BBP_R62			62 // Rx SQ0 Threshold HIGH
#define BBP_R63			63
#define BBP_R64			64
#define BBP_R65			65
#define BBP_R66			66
#define BBP_R67			67
#define BBP_R68			68
#define BBP_R69			69
#define BBP_R70			70 // Rx AGC SQ CCK Xcorr threshold
#define BBP_R73			73
#define BBP_R75			75
#define BBP_R77			77
#define BBP_R78			78
#define BBP_R79			79
#define BBP_R80			80
#define BBP_R81			81
#define BBP_R82			82
#define BBP_R83			83
#define BBP_R84			84
#define BBP_R86			86
#define BBP_R88			88
#define BBP_R91			91
#define BBP_R92			92
#define BBP_R94			94 // Tx Gain Control
#define BBP_R103		103
#define BBP_R104		104
#define BBP_R105		105
#define BBP_R106		106
#define BBP_R109		109
#define BBP_R110		110
#define BBP_R113		113
#define BBP_R114		114
#define BBP_R115		115
#define BBP_R116		116
#define BBP_R117		117
#define BBP_R118		118
#define BBP_R119		119
#define BBP_R120		120
#define BBP_R121		121
#define BBP_R122		122
#define BBP_R123		123
#define BBP_R126		126
#define BBP_R127		127
#define BBP_R137		137
#define BBP_R138		138 // add by johnli, RF power sequence setup, ADC dynamic on/off control
#define BBP_R140		140
#define BBP_R141		141
#define BBP_R142		142
#define BBP_R143		143
#define BBP_R148		148

#define BBP_R160		160 // RT3883 Tx BF control
#define BBP_R161		161
#define BBP_R162		162
#define BBP_R163		163
#define BBP_R164		164

#define BBP_R173		173
#define BBP_R174		174
#define BBP_R175		175
#define BBP_R176		176
#define BBP_R177		177
#define BBP_R179		179
#define BBP_R180		180
#define BBP_R181		181
#define BBP_R182		182
#define BBP_R184		184
#define BBP_R185		185
#define BBP_R186		186
#define BBP_R187		187
#define BBP_R188		188
#define BBP_R189		189
#define BBP_R190		190
#define BBP_R191		191
#define BBP_R250		250
#define BBP_R255		255 // for TSSI and Tone Radar


#define BBPR94_DEFAULT	0x06 // Add 1 value will gain 1db


//
// BBP R49 TSSI (Transmit Signal Strength Indicator)
//
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R49_STRUC {
	struct
	{
		UCHAR	adc5_in_sel:1; // 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390)
		UCHAR	bypassTSSIAverage:1; // 0: the average TSSI (the average of the 16 samples), 1: the current TSSI
		UCHAR	Reserved:1; // Reserved field
		UCHAR	TSSI:5; // TSSI value
	} field;

	UCHAR		byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#else
typedef union _BBP_R49_STRUC {
	struct
	{
		UCHAR	TSSI:5; // TSSI value
		UCHAR	Reserved:1; // Reserved field
		UCHAR	bypassTSSIAverage:1; // 0: the average TSSI (the average of the 16 samples), 1: the current TSSI
		UCHAR	adc5_in_sel:1; // 0: TSSI (from the external components, old version), 1: PSI (internal components, new version - RT3390)
	} field;
	
	UCHAR		byte;
} BBP_R49_STRUC, *PBBP_R49_STRUC;
#endif

//
// BBP R105 (FEQ control, MLD control and SIG remodulation)
//
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	Reserve1:4; // Reserved field
		UCHAR	EnableSIGRemodulation:1; // Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
		UCHAR	MLDFor2Stream:1; // Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
		UCHAR	IndependentFeedForwardCompensation:1; // Apply independent feed-forward compensation for independent stream.
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; // Under 40 MHz band, detect SIG on primary channel only.
	} field;

	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#else
typedef union _BBP_R105_STRUC {
	struct
	{
		UCHAR	DetectSIGOnPrimaryChannelOnly:1; // Under 40 MHz band, detect SIG on primary channel only.
		UCHAR	IndependentFeedForwardCompensation:1; // Apply independent feed-forward compensation for independent stream.
		UCHAR	MLDFor2Stream:1; // Apply Maximum Likelihood Detection (MLD) for 2 stream case (reserved field if single RX)
		UCHAR	EnableSIGRemodulation:1; // Enable the channel estimation updates based on remodulation of L-SIG and HT-SIG symbols.
		UCHAR	Reserve1:4; // Reserved field
	} field;
	
	UCHAR		byte;
} BBP_R105_STRUC, *PBBP_R105_STRUC;
#endif

//
// BBP R106 (GI remover)
//
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R106_STRUC {
	struct
	{
		UCHAR	EnableLowPowerFSD:1; // enable/disable the low power FSD
		UCHAR	ShortGI_Offset40:4; // Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate)
		UCHAR	ShortGI_Offset20:3; // Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate)
	} field;

	UCHAR		byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#else
typedef union _BBP_R106_STRUC {
	struct
	{
		UCHAR	ShortGI_Offset20:3; // Delay GI remover when the short GI is detected in 20MHz band (20M sampling rate)
		UCHAR	ShortGI_Offset40:4; // Delay GI remover when the short GI is detected in 40MHz band (40M sampling rate)
		UCHAR	EnableLowPowerFSD:1; // enable/disable the low power FSD
	} field;
	
	UCHAR		byte;
} BBP_R106_STRUC, *PBBP_R106_STRUC;
#endif

//
// BBP R109 (Tx power control in 0.1dB step)
//
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R109_STRUC {
	struct
	{
		UCHAR	Tx1PowerCtrl:4; // Tx1 power control in 0.1dB step (valid: 0~10)
		UCHAR	Tx0PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
	} field;

	UCHAR		byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#else
typedef union _BBP_R109_STRUC {
	struct
	{
		UCHAR	Tx0PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
		UCHAR	Tx1PowerCtrl:4; // Tx0 power control in 0.1dB step (valid: 0~10)
	} field;
	
	UCHAR		byte;
} BBP_R109_STRUC, *PBBP_R109_STRUC;
#endif

//
// BBP R110 (Tx power control in 0.1dB step)
//
#ifdef RT_BIG_ENDIAN
typedef union _BBP_R110_STRUC {
	struct
	{
		UCHAR	Tx2PowerCtrl:4; // Tx2 power control in 0.1dB step (valid: 0~10)
		UCHAR	AllTxPowerCtrl:4; // All transmitters' fine power control in 0.1dB (valid: 0~10)
	} field;

	UCHAR		byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#else
typedef union _BBP_R110_STRUC {
	struct
	{
		UCHAR	AllTxPowerCtrl:4; // All transmitters' fine power control in 0.1dB (valid: 0~10)
		UCHAR	Tx2PowerCtrl:4; // Tx2 power control in 0.1dB step (valid: 0~10)
	} field;
	
	UCHAR		byte;
} BBP_R110_STRUC, *PBBP_R110_STRUC;
#endif


#ifdef RTMP_RBUS_SUPPORT
// TODO: for this definition, need to modify it!!
	//#define MAX_BBP_ID	255
	#define MAX_BBP_ID	200
	#define MAX_BBP_MSG_SIZE	4096
#else
	#define MAX_BBP_ID	136
	#define MAX_BBP_MSG_SIZE	2048
#endif // RTMP_RBUS_SUPPORT //


//
// BBP & RF are using indirect access. Before write any value into it.
// We have to make sure there is no outstanding command pending via checking busy bit.
//
#define MAX_BUSY_COUNT  100         // Number of retry before failing access BBP & RF indirect register

//#define PHY_TR_SWITCH_TIME          5  // usec

//#define BBP_R17_LOW_SENSIBILITY     0x50
//#define BBP_R17_MID_SENSIBILITY     0x41
//#define BBP_R17_DYNAMIC_UP_BOUND    0x40

#define RSSI_FOR_VERY_LOW_SENSIBILITY   -35
#define RSSI_FOR_LOW_SENSIBILITY		-58
#define RSSI_FOR_MID_LOW_SENSIBILITY	-80
#define RSSI_FOR_MID_SENSIBILITY		-90

/*****************************************************************************
	RF register Read/Write marco definition
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
#define RTMP_RF_IO_WRITE32(_A, _V)                  \
{                                                   					\
	if ((_A)->bPCIclkOff == FALSE) 	                \
	{												\
		PHY_CSR4_STRUC  _value;                          \
		ULONG           _busyCnt = 0;                    \
											\
		do {                                            \
			RTMP_IO_READ32((_A), RF_CSR_CFG0, &_value.word);  \
			if (_value.field.Busy == IDLE)               \
				break;                                  \
			_busyCnt++;                                  \
		}while (_busyCnt < MAX_BUSY_COUNT);			\
		if(_busyCnt < MAX_BUSY_COUNT)                   \
		{                                               \
			RTMP_IO_WRITE32((_A), RF_CSR_CFG0, (_V));          \
    		}                                               \
    	}								\
    if ((_A)->ShowRf)					\
    {									\
    	printk("RF:%x\n", _V);			\
	}									\
}
#endif // RTMP_MAC_PCI //





/*****************************************************************************
	BBP register Read/Write marco definitions.
	we read/write the bbp value by register's ID. 
	Generate PER to test BA
 *****************************************************************************/
#ifdef RTMP_MAC_PCI
/*
	basic marco for BBP read operation. 
	_pAd: the data structure pointer of RTMP_ADAPTER
	_bbpID : the bbp register ID
	_pV: data pointer used to save the value of queried bbp register.
	_bViaMCU: if we need access the bbp via the MCU.
*/
#ifdef RELASE_INCLUDE
/*
	The RTMP_PCIE_PS_L3_BBP_IO_READ8 is used to support PCIE power-saving solution3.
	"brc =AsicSendCommandToMcu" is used to avoid any muc command is executed during
	RF_OFF command.
*/
#endif // RELASE_INCLUDE //

#ifdef CONFIG_STA_SUPPORT
#define IS_SUPPORT_PCIE_PS_L3(_pAd) (((_pAd)->OpMode == OPMODE_STA) &&\
	(IS_RT3090((_pAd)) || IS_RT3572((_pAd)) || IS_RT3390((_pAd)) || IS_RT3593((_pAd))) && \
	((_pAd)->StaCfg.PSControl.field.rt30xxPowerMode == 3)&& \
	((_pAd)->StaCfg.PSControl.field.EnableNewPS == TRUE)) 
	
#define RTMP_PCIE_PS_L3_BBP_IO_READ8(_pAd, _bbpID, _pV, _bViaMCU)			\
	do{															\
	BBP_CSR_CFG_STRUC	BbpCsr;									\
	int					_busyCnt, _secCnt, _regID;					\
	BOOLEAN					brc;									\
	_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);	\
	BbpCsr.field.Busy = IDLE;										\
	if (((_pAd)->bPCIclkOff == FALSE)								\
		&& (((_pAd)->LastMCUCmd== WAKE_MCU_CMD) || ((_pAd)->LastMCUCmd==0x72))\
		&& ((_pAd)->brt30xxBanMcuCmd == FALSE))					\
	{															\
		for (_busyCnt=0; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)	\
		{														\
			RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word);			\
			if (BbpCsr.field.Busy == BUSY)                 					\
				continue;                                               				\
			BbpCsr.word = 0;										\
			BbpCsr.field.fRead = 1;									\
			BbpCsr.field.BBP_RW_MODE = 1;							\
			BbpCsr.field.Busy = 1;									\
			BbpCsr.field.RegNum = _bbpID;                       			\
			RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);			\
			if ((_bViaMCU) == TRUE)								\
			{													\
				brc =AsicSendCommandToMcu(_pAd, 0x80, 0xff, 0x0, 0x0); \
				RTMPusecDelay(1000);							\
			}							\
	               if (brc == TRUE) 										\
			{                                                 								\
				for (_secCnt=0; _secCnt<MAX_BUSY_COUNT; _secCnt++)       	\
				{														\
					RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word); 	\
					if (BbpCsr.field.Busy == IDLE)							\
						break;											\
				}														\
				if ((BbpCsr.field.Busy == IDLE) &&							\
				(BbpCsr.field.RegNum == _bbpID))                					\
				{																\
					*(_pV) = (UCHAR)BbpCsr.field.Value;							\
					break;														\
				}																\
			}																\
			else 																\
			{																\
				BbpCsr.field.Busy = 0;											\
				RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);				\
			}																\
		}																	\
	}	\
	if ((BbpCsr.field.Busy == BUSY) || ((_pAd)->bPCIclkOff == TRUE))				\
	{																	\
	                DBGPRINT_ERR(("RTMP_PCIE_PS_L3_BBP_IO_READ8(viaMCU=%d) read R%d fail(reason:clk=%d,busy=%x)\n", (_bViaMCU), _bbpID,(_pAd)->bPCIclkOff ,BbpCsr.field.Busy));      \
			*(_pV) = (_pAd)->BbpWriteLatch[_bbpID];               \
	}																	\
}while(0)
#else
#define IS_SUPPORT_PCIE_PS_L3(_pAd) FALSE
#define RTMP_PCIE_PS_L3_BBP_IO_READ8(_pAd, _bbpID, _pV, _bViaMCU)
#endif // CONFIG_STA_SUPPORT //

#define _RTMP_BBP_IO_READ8(_pAd, _bbpID, _pV, _bViaMCU)			\
	do{															\
		BBP_CSR_CFG_STRUC  BbpCsr;								\
		int   _busyCnt, _secCnt, _regID;                               			\
																\
		_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);	\
		for (_busyCnt=0; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)      \
		{                                                   							\
			RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word);     	\
			if (BbpCsr.field.Busy == BUSY)                  \
				continue;                                               \
			BbpCsr.word = 0;                                \
			BbpCsr.field.fRead = 1;                         \
			BbpCsr.field.BBP_RW_MODE = 1;                         \
			BbpCsr.field.Busy = 1;                          \
			BbpCsr.field.RegNum = _bbpID;                       \
			RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);     \
			if ((_bViaMCU) == TRUE)							\
			{													\
				AsicSendCommandToMcu(_pAd, 0x80, 0xff, 0x0, 0x0); \
				RTMPusecDelay(1000);	\
			}							\
			for (_secCnt=0; _secCnt<MAX_BUSY_COUNT; _secCnt++)       \
			{                                               \
				RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word); \
				if (BbpCsr.field.Busy == IDLE)              \
					break;                                  \
			}                                               \
			if ((BbpCsr.field.Busy == IDLE) &&              \
				(BbpCsr.field.RegNum == _bbpID))                \
			{                                               \
				*(_pV) = (UCHAR)BbpCsr.field.Value;         \
				break;                                      \
			}                                               \
		}                                                   \
		if (BbpCsr.field.Busy == BUSY)                      \
		{                                                   \
			DBGPRINT_ERR(("BBP(viaMCU=%d) read R%d fail\n", (_bViaMCU), _bbpID));      \
			*(_pV) = (_pAd)->BbpWriteLatch[_bbpID];               \
			if ((_bViaMCU) == TRUE)				\
			{									\
				RTMP_IO_READ32(_pAd, _regID, &BbpCsr.word);				\
				BbpCsr.field.Busy = 0;                          \
				RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);				\
			}				\
		}													\
	}while(0)

/*
	This marco used for the BBP read operation which didn't need via MCU.
*/
#define BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)			\
	RTMP_BBP_IO_READ8((_A), (_I), (_pV), FALSE)

/*
	This marco used for the BBP read operation which need via MCU.
	But for some chipset which didn't have mcu (e.g., RBUS based chipset), we
	will use this function too and didn't access the bbp register via the MCU.
*/
#define _RTMP_BBP_IO_READ8_BY_REG_ID(_A, _I, _pV)			\
	do{														\
		if ((_A)->bPCIclkOff == FALSE)                     				\
		{													\
			if ((_A)->infType == RTMP_DEV_INF_RBUS)			\
				RTMP_BBP_IO_READ8((_A), (_I), (_pV), FALSE);	\
			else												\
				if(IS_SUPPORT_PCIE_PS_L3((_A)))				\
					RTMP_PCIE_PS_L3_BBP_IO_READ8((_A), (_I), (_pV), TRUE);	\
				else												\
					RTMP_BBP_IO_READ8((_A), (_I), (_pV), TRUE);	\
		}													\
	}while(0)

/*
	basic marco for BBP write operation. 
	_pAd: the data structure pointer of RTMP_ADAPTER
	_bbpID : the bbp register ID
	_pV: data used to save the value of queried bbp register.
	_bViaMCU: if we need access the bbp via the MCU.
*/

#ifdef RALINK_ATE
#define RTMP_BBP_CAN_WRITE(_pAd, _bbpId)	(!((ATE_ON(_pAd)) && ((_pAd)->ate.forceBBPReg == (_bbpId))))
#else
#define RTMP_BBP_CAN_WRITE(_pAd, _bbpId)	1
#endif // RALINK_ATE //


#ifdef CONFIG_STA_SUPPORT
#define RTMP_PCIE_PS_L3_BBP_IO_WRITE8(_pAd, _bbpID, _pV, _bViaMCU)				\
	do{											\
		BBP_CSR_CFG_STRUC  BbpCsr;							\
		int             k, _busyCnt=0, _regID;						\
		BOOLEAN					brc;					\
														\
		if (RTMP_BBP_CAN_WRITE(_pAd, _bbpID) == FALSE) \
			break;	\
		_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);			\
			if (((_pAd)->bPCIclkOff == FALSE)					\
			&& ((_pAd)->brt30xxBanMcuCmd == FALSE))								\
			{																	\
				if (_pAd->AccessBBPFailCount > 20)								\
				{																\
					AsicResetBBPAgent(_pAd);									\
					_pAd->AccessBBPFailCount = 0;								\
				}																\
				for (_busyCnt=0; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)  \
				{                                                  						 \
					RTMP_IO_READ32((_pAd), _regID, &BbpCsr.word);     \
					if ((BbpCsr.field.Busy == BUSY) || (_busyCnt % 10 == 0))	\
					{															\
						BbpCsr.field.Busy = IDLE;								\
						RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, BbpCsr.word);		\
						continue;                                   \
					}												\
					BbpCsr.word = 0;                                \
					BbpCsr.field.fRead = 0;                         \
					BbpCsr.field.BBP_RW_MODE = 1;                         \
					BbpCsr.field.Busy = 1;                          \
					BbpCsr.field.Value = _pV;                        \
					BbpCsr.field.RegNum = _bbpID;                       \
					RTMP_IO_WRITE32((_pAd), _regID, BbpCsr.word);     \
					if ((_bViaMCU) == TRUE)									\
					{														\
						brc =AsicSendCommandToMcu(_pAd, 0x80, 0xff, 0x0, 0x0);		\
						if ((_pAd)->OpMode == OPMODE_AP)						\
							RTMPusecDelay(1000);							\
					}														\
					if (brc == TRUE) 											\
					{														\
						for (k=0; k<MAX_BUSY_COUNT; k++)								\
						{																\
							BbpCsr.field.Busy = BUSY;											\
							RTMP_IO_READ32(_pAd, H2M_BBP_AGENT, &BbpCsr.word);			\
							if (BbpCsr.field.Busy == IDLE)								\
								break;													\
						}																\
						if ((BbpCsr.field.Busy == BUSY))										\
						{																	\
							DBGPRINT_ERR(("Check BBP write R%d=0x%x fail\n", _bbpID, BbpCsr.word));	\
						}						\
						(_pAd)->BbpWriteLatch[_bbpID] = _pV;                   		\
					}														\
					else 													\
					{														\
						BbpCsr.field.Busy = 0;									\
						RTMP_IO_WRITE32(_pAd, _regID, BbpCsr.word);	\
					}								\
					break;													\
				}  	\
			}		\
			else 										\
			{																	\
				DBGPRINT_ERR(("  brt30xxBanMcuCmd = %d. Write BBP %d \n",  (_pAd)->brt30xxBanMcuCmd, (_regID)));	\
			}																	\
		if ((_busyCnt == MAX_BUSY_COUNT) || ((_pAd)->bPCIclkOff == TRUE))			\
			{																	\
				if (_busyCnt == MAX_BUSY_COUNT)					\
				(_pAd)->AccessBBPFailCount++;					\
				DBGPRINT_ERR(("BBP write R%d=0x%x fail. BusyCnt= %d.bPCIclkOff = %d. \n", _regID, BbpCsr.word, _busyCnt, (_pAd)->bPCIclkOff ));	\
			}																	\
	}while(0)
#else
#define RTMP_PCIE_PS_L3_BBP_IO_WRITE8(_pAd, _bbpID, _pV, _bViaMCU)
#endif // CONFIG_STA_SUPPORT //

#define _RTMP_BBP_IO_WRITE8(_pAd, _bbpID, _pV, _bViaMCU)			\
	do{															\
		BBP_CSR_CFG_STRUC  BbpCsr;                             \
		int             _busyCnt=0, _regID;                               			\
		BOOLEAN					brc;			\
																\
		if (RTMP_BBP_CAN_WRITE(_pAd, _bbpID) == FALSE) \
			break;	\
		_regID = ((_bViaMCU) == TRUE ? H2M_BBP_AGENT : BBP_CSR_CFG);	\
		for (_busyCnt=0; _busyCnt<MAX_BUSY_COUNT; _busyCnt++)  \
		{                                                   \
			RTMP_IO_READ32((_pAd), _regID, &BbpCsr.word);     \
			if (BbpCsr.field.Busy == BUSY)                  \
			{\
					if ( ((_bViaMCU) == TRUE) && ((_busyCnt % 20) == 0)) \
					{\
						BbpCsr.field.Busy = IDLE;\
						RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, BbpCsr.word);\
					}\
				continue;                                   \
			}\
			BbpCsr.word = 0;                                \
			BbpCsr.field.fRead = 0;                         \
			BbpCsr.field.BBP_RW_MODE = 1;                         \
			BbpCsr.field.Busy = 1;                          \
			BbpCsr.field.Value = _pV;                        \
			BbpCsr.field.RegNum = _bbpID;                       \
			RTMP_IO_WRITE32((_pAd), _regID, BbpCsr.word);     \
			if ((_bViaMCU) == TRUE)									\
			{														\
				brc = AsicSendCommandToMcu(_pAd, 0x80, 0xff, 0x0, 0x0);		\
				if ((_pAd)->OpMode == OPMODE_AP)						\
					RTMPusecDelay(1000);							\
				if (brc == FALSE) \
				{ \
					BbpCsr.field.Busy = IDLE;											\
					RTMP_IO_WRITE32((_pAd), H2M_BBP_AGENT, BbpCsr.word);				\
				} \
			}														\
			(_pAd)->BbpWriteLatch[_bbpID] = _pV;                   			\
			break;													\
		}                                                   								\
		if (_busyCnt == MAX_BUSY_COUNT)                      					\
		{                                                   								\
			DBGPRINT_ERR(("BBP write R%d fail\n", _bbpID));     			\
			if((_bViaMCU) == TRUE)									\
			{														\
				RTMP_IO_READ32(_pAd, H2M_BBP_AGENT, &BbpCsr.word);	\
				BbpCsr.field.Busy = 0;                          					\
				RTMP_IO_WRITE32(_pAd, H2M_BBP_AGENT, BbpCsr.word);	\
			}														\
		}                                                   								\
	}while(0)


/*
	This marco used for the BBP write operation which didn't need via MCU.
*/
#define BBP_IO_WRITE8_BY_REG_ID(_A, _I, _pV)			\
	RTMP_BBP_IO_WRITE8((_A), (_I), (_pV), FALSE)

/*
	This marco used for the BBP write operation which need via MCU.
	But for some chipset which didn't have mcu (e.g., RBUS based chipset), we
	will use this function too and didn't access the bbp register via the MCU.
*/
#define _RTMP_BBP_IO_WRITE8_BY_REG_ID(_A, _I, _pV)			\
	do{														\
		if ((_A)->bPCIclkOff == FALSE)                     				\
		{													\
			if ((_A)->infType == RTMP_DEV_INF_RBUS)			\
				RTMP_BBP_IO_WRITE8((_A), (_I), (_pV), FALSE);	\
			else												\
				if(IS_SUPPORT_PCIE_PS_L3((_A)))				\
					RTMP_PCIE_PS_L3_BBP_IO_WRITE8((_A), (_I), (_pV), TRUE);	\
				else												\
				RTMP_BBP_IO_WRITE8((_A), (_I), (_pV), TRUE);	\
		}													\
	}while(0)

#if 0
#ifndef VENDOR_FEATURE3_SUPPORT
#define RTMP_BBP_IO_READ8				_RTMP_BBP_IO_READ8
#define RTMP_BBP_IO_READ8_BY_REG_ID		_RTMP_BBP_IO_READ8_BY_REG_ID
#define RTMP_BBP_IO_WRITE8				_RTMP_BBP_IO_WRITE8
#define RTMP_BBP_IO_WRITE8_BY_REG_ID	_RTMP_BBP_IO_WRITE8_BY_REG_ID
#endif // VENDOR_FEATURE3_SUPPORT //
#endif

#if defined(RT2883) || defined(RT3883) || defined(DFS_HARDWARE_SUPPORT)

#define RTMP_DFS_IO_READ8(_A, _I, _V)                   \
{                                                       \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R140, _I);          \
	BBP_IO_READ8_BY_REG_ID(_A, BBP_R141, _V);           \
}

#define RTMP_DFS_IO_WRITE8(_A, _I, _V)                  \
{                                                       \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R140, _I);          \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R141, _V);          \
}

#define RTMP_CARRIER_IO_READ8(_A, _I, _V)               \
{                                                       \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R184, _I);          \
	BBP_IO_READ8_BY_REG_ID(_A, BBP_R185, _V);           \
}
#define RTMP_CARRIER_IO_WRITE8(_A, _I, _V)              \
{                                                       \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R184, _I);          \
	BBP_IO_WRITE8_BY_REG_ID(_A, BBP_R185, _V);          \
}

#endif // defined(RT2883) || defined(RT3883) || defined(DFS_HARDWARE_SUPPORT) //
	
#endif // RTMP_MAC_PCI //




#endif // __RTMP_PHY_H__ //

