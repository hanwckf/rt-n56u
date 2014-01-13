
/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
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
    dfs.h

    Abstract:
    Support DFS function.

    Revision History:
    Who       When            What
    --------  ----------      ----------------------------------------------
    Fonchi    03-12-2007      created
*/
#ifndef __DFS_H__
#define __DFS_H__

/*************************************************************************
  *
  *	DFS Radar related definitions.
  *
  ************************************************************************/ 

#ifdef DFS_SUPPORT
#define RADAR_DEBUG_SHOW_RAW_EVENT		0x01  /* Show the 384-bytes raw data of event buffer */
#define RADAR_DEBUG_EVENT					0x02  /* Show effective event reads out from the event buffer */
#define RADAR_DEBUG_SILENCE				0x04
#define RADAR_DEBUG_SW_SILENCE			0x08
#define RADAR_DONT_SWITCH		0x10 /* Don't Switch channel when hit */
#define RADAR_DEBUG_DONT_CHECK_BUSY		0x20
#define RADAR_DEBUG_DONT_CHECK_RSSI		0x40
#define RADAR_SIMULATE						0x80 /* simulate a short pulse hit this channel */

/* McuCmd */
#define DFS_ONOFF_MCU_CMD					0x64

/*#define DFS_SW_RADAR_DECLARE_THRES	3*/
#define DFS_EVENT_SIZE						6    /* Number of bytes of each DFS event */
#define DFS_EVENT_BUFFER_SIZE				384  /* Number of bytes of a DFS event buffer */
#define DFS_SW_RADAR_CHECK_LOOP				50
#define DFS_SW_RADAR_SHIFT          		3
#define DFS_SW_RADAR_CH0_ERR				8
#define DFS_SW_RADAR_PERIOD_ERR				4
#define CE_STAGGERED_RADAR_CH0_H_ERR		(DFS_SW_RADAR_CH0_ERR + 16) // the step is 16 for every 0.1 us different in width
#define CE_STAGGERED_RADAR_DECLARE_THRES	2

#define NEW_DFS_FCC_5_ENT_NUM			5
#define NEW_DFS_DBG_PORT_ENT_NUM_POWER	8
#define NEW_DFS_DBG_PORT_ENT_NUM		(1 << NEW_DFS_DBG_PORT_ENT_NUM_POWER)	/* CE Debug Port entry number, 256 */
#define NEW_DFS_DBG_PORT_MASK			(NEW_DFS_DBG_PORT_ENT_NUM - 1)	/* 0xff */

#define CH_BUSY_SAMPLE_POWER 3
#define CH_BUSY_SAMPLE 		(1 << CH_BUSY_SAMPLE_POWER)
#define CH_BUSY_MASK  		(CH_BUSY_SAMPLE - 1)

#define MAX_FDF_NUMBER 5	/* max false-detection-filter number */

/* Matched Period definition */
#define NEW_DFS_MPERIOD_ENT_NUM_POWER	8
#define NEW_DFS_MPERIOD_ENT_NUM			(1 << NEW_DFS_MPERIOD_ENT_NUM_POWER)	/* CE Period Table entry number, 512 */
#define NEW_DFS_CHANNEL_0				1
#define NEW_DFS_CHANNEL_1				2
#define NEW_DFS_CHANNEL_2				4
#define NEW_DFS_CHANNEL_3				8
#define NEW_DFS_CHANNEL_4				16
#define NEW_DFS_CHANNEL_5				32

#define NEW_DFS_MAX_CHANNEL			5

#define CE_SW_CHECK						3

#define NEW_DFS_WATCH_DOG_TIME		1 /* note that carrier detection also need timer interrupt hook*/

#define NEW_DFS_FCC		0x1 /* include Japan*/
#define NEW_DFS_EU		0x2
#define NEW_DFS_JAP		0x4
#define NEW_DFS_JAP_W53	0x8
#define NEW_DFS_END		0xff
#define MAX_VALID_RADAR_W	5
#define MAX_VALID_RADAR_T	5

#define DFS_SW_RADAR_CH1_SHIFT		3
#define DFS_SW_RADAR_CH2_SHIFT		3

#define CE_STAGGERED_RADAR_PERIOD_MAX		((133333 + 125000 + 117647 + 1000) * 2)
#define FCC_RADAR_PERIOD_MAX				(((28570 << 1) + 1000) * 2)
#define JAP_RADAR_PERIOD_MAX				(((80000 << 1) + 1000) * 2)

#define NEW_DFS_BANDWITH_MONITOR_TIME 	(NEW_DFS_CHECK_TIME / NEW_DFS_CHECK_TIME_TASKLET)
#define NEW_DFS_CHECK_TIME				300
#define NEW_DFS_CHECK_TIME_TASKLET		3

/*#define DFS_SW_RADAR_DECLARE_THRES	3*/

#define DFS_SW_RADAR_SHIFT          3

#define DFS_SW_RADAR_CH0_ERR		8

#define CE_STAGGERED_RADAR_CH0_H_ERR		(DFS_SW_RADAR_CH0_ERR + 16) /* the step is 16 for every 0.1 us different in width*/

#define CE_STAGGERED_RADAR_DECLARE_THRES	2


/* DFS Macros */
#define PERIOD_MATCH(a, b, c)			((a >= b)? ((a-b) <= c):((b-a) <= c))
#define ENTRY_PLUS(a, b, c)				(((a+b) < c)? (a+b) : (a+b-c))
#define ENTRY_MINUS(a, b, c)			((a >= b)? (a - b) : (a+c-b))
#define MAX_PROCESS_ENTRY 				16

#define IS_FCC_RADAR_1(HT_BW, T)			(((HT_BW)? ((T > 57120) && (T < 57160)) : (T > 28560) && (T < 28580)))
#define IS_W53_RADAR_2(HT_BW, T)			(((HT_BW)? ((T > 153820) && (T < 153872)) : (T > 76910) && (T < 76936)))
#define IS_W56_RADAR_3(HT_BW, T)			(((HT_BW)? ((T > 159900) && (T < 160100)) : (T > 79950) && (T < 80050)))

#define DFS_EVENT_SANITY_CHECK(_pAd, _DfsEvent)	\
		!(((_DfsEvent).EngineId >= _pAd->chipCap.DfsEngineNum) ||	\
		 ((_DfsEvent).TimeStamp & 0xffc00000) ||	\
		 ((_DfsEvent).Width & 0xe000))
		 
#define DFS_EVENT_PRINT(_DfsEvent)		\
		DBGPRINT(RT_DEBUG_ERROR, ( "EngineId = %u, Timestamp = %u, Width = %u\n",	\
		_DfsEvent.EngineId, _DfsEvent.TimeStamp, _DfsEvent.Width));


#define DFS_EVENT_BUFF_PRINT(_StarIdx,  _TableIdx, _BufSize)						\
{																				\
	UINT32 k;																	\
	for (k = _StarIdx; k < _BufSize; k++)											\
	{																			\
		DBGPRINT(RT_DEBUG_TRACE, ("0x%02x ", _TableIdx[k]));						\
		if(k%DFS_EVENT_SIZE == ((DFS_EVENT_SIZE-1+_StarIdx)%DFS_EVENT_SIZE)) 	\
			DBGPRINT(RT_DEBUG_TRACE, ("\n"));									\
	}																			\
}

/* check whether we can do DFS detection or not */
#define DFS_CHECK_FLAGS(_pAd, _pRadarDetect)					\
		!((_pAd->Dot11_H.RDMode == RD_SWITCHING_MODE) ||		\
		(_pRadarDetect->bDfsInit == FALSE) ||						\
		(_pRadarDetect->DFSAPRestart == 1))


typedef enum _DFS_VERSION {
	SOFTWARE_DFS = 0,
	HARDWARE_DFS_V1,
	HARDWARE_DFS_V2
} DFS_VERSION;

typedef struct _NewDFSValidRadar
{
	USHORT type;
	USHORT channel; /* bit map*/
	USHORT WLow;
	USHORT WHigh;
	USHORT W;  /* for fixed width radar*/
	USHORT WMargin;
	ULONG TLow;
	ULONG THigh;
	ULONG T;  /* for fixed period radar */
	USHORT TMargin;
}NewDFSValidRadar, *pNewDFSValidRadar;

typedef struct _NewDFSDebugPort {
	ULONG counter;
	ULONG timestamp;
	USHORT width;
	USHORT start_idx;	/* start index to period table */
	USHORT end_idx;		/* end index to period table */
} NewDFSDebugPort, *pNewDFSDebugPort;

/* Matched Period Table */
typedef struct _NewDFSMPeriod {
	USHORT idx;
	USHORT width;
	USHORT idx2;
	USHORT width2;
	ULONG period;
} NewDFSMPeriod, *pNewDFSMPeriod;



typedef struct _NewDFSParam {
	BOOLEAN valid;
	UCHAR mode;
	USHORT avgLen;
	USHORT ELow;
	USHORT EHigh;
	USHORT WLow;
	USHORT WHigh;
	UCHAR EpsilonW;
	ULONG TLow;
	ULONG THigh;
	UCHAR EpsilonT;
	ULONG BLow;
	ULONG BHigh;
} NewDFSParam, *pNewDFSParam;

typedef struct _DFS_PROGRAM_PARAM{
	NewDFSParam NewDFSTableEntry[NEW_DFS_MAX_CHANNEL*4];
	USHORT ChEnable;	/* Enabled Dfs channels (bit wise)*/
	UCHAR DeltaDelay;
	/* Support after dfs_func >= 2 */
	UCHAR Symmetric_Round;
	UCHAR VGA_Mask;
	UCHAR Packet_End_Mask;
	UCHAR Rx_PE_Mask;
	ULONG RadarEventExpire[NEW_DFS_MAX_CHANNEL];
}DFS_PROGRAM_PARAM, *PDFS_PROGRAM_PARAM;

typedef struct _NewDFSTable
{
	USHORT type;
	NewDFSParam entry[NEW_DFS_MAX_CHANNEL];
}NewDFSTable, *pNewDFSTable;

#ifdef DFS_DEBUG
typedef struct _NewDFSDebugResult
{
	char delta_delay_shift;
	char EL_shift;
	char EH_shift;
	char WL_shift;
	char WH_shift;
	ULONG hit_time;
	ULONG false_time;
}NewDFSDebugResult, *pNewDFSDebugResult;
#endif

typedef struct _DFS_EVENT{
	UINT8  EngineId;
	UINT32 TimeStamp;
	UINT16 Width;
}DFS_EVENT, *PDFS_EVENT;

typedef struct _DFS_SW_DETECT_PARAM{
	NewDFSDebugPort FCC_5[NEW_DFS_FCC_5_ENT_NUM];
	UCHAR fcc_5_idx;
	UCHAR fcc_5_last_idx;
	USHORT fcc_5_threshold; /* to check the width of long pulse radar */
	USHORT dfs_width_diff_ch1_Shift;
	USHORT dfs_width_diff_ch2_Shift;
	USHORT dfs_period_err;
	ULONG dfs_max_period;	/* Max possible Period */
	USHORT dfs_width_diff;
	USHORT dfs_width_ch0_err_L;
	USHORT dfs_width_ch0_err_H;
	UCHAR dfs_check_loop;
	UCHAR dfs_declare_thres;	
	ULONG dfs_w_counter;
	DFS_EVENT PreDfsEvent;		/* previous radar event */
	UINT32 EvtDropAdjTime;		/* timing threshold for adjacent event */
	UINT sw_idx[NEW_DFS_MAX_CHANNEL];
	UINT hw_idx[NEW_DFS_MAX_CHANNEL];
	UINT pr_idx[NEW_DFS_MAX_CHANNEL];	
	USHORT dfs_t_idx[NEW_DFS_MAX_CHANNEL];	
	USHORT dfs_w_idx[NEW_DFS_MAX_CHANNEL];
	USHORT dfs_w_last_idx[NEW_DFS_MAX_CHANNEL];
	NewDFSDebugPort DFS_W[NEW_DFS_MAX_CHANNEL][NEW_DFS_DBG_PORT_ENT_NUM];
	NewDFSMPeriod DFS_T[NEW_DFS_MAX_CHANNEL][NEW_DFS_MPERIOD_ENT_NUM];	/* period table */
	/*UCHAR	ce_sw_id_check;*/
	/*USHORT	ce_sw_t_diff;*/
	/*ULONG fcc_5_counter;*/
	/* CE Staggered radar / weather radar */	
#ifdef DFS_DEBUG
	/* Roger debug */
	UCHAR DebugPort[384];
	UCHAR DebugPortPrint;	/* 0 = stop, 1 = log req, 2 = loging, 3 = log done */
	ULONG TotalEntries[4];
	ULONG T_Matched_2;
	ULONG T_Matched_3;
	ULONG T_Matched_4;
	ULONG T_Matched_5;
	UCHAR BBP127Repeat;
	ULONG CounterStored[5];
	ULONG CounterStored2[5];
	ULONG CounterStored3;
	NewDFSDebugPort CE_DebugCh0[NEW_DFS_DBG_PORT_ENT_NUM];
	NewDFSMPeriod CE_TCh0[NEW_DFS_MPERIOD_ENT_NUM];
#endif
}DFS_SW_DETECT_PARAM, *PDFS_SW_DETECT_PARAM;

/***************************************************************************
  *	structure for radar detection and channel switch
  **************************************************************************/
typedef struct _RADAR_DETECT_STRUCT {
	UCHAR	DFSAPRestart;
	ULONG MCURadarRegion;
	CHAR  AvgRssiReq;
	ULONG DfsLowerLimit;
	ULONG DfsUpperLimit;
	ULONG upperlimit;
	ULONG lowerlimit;
	ULONG TimeStamp; /*unit: 1us*/
	UCHAR ChirpCheck; /* anounce on second detection of chirp radar */
	UCHAR bChannelSwitchInProgress; /* RDMode could cover this*/
	BOOLEAN bDfsSwDisable; /* disable sotfwre check */
	BOOLEAN bDfsInit;		/* to indicate if dfs regs has been initialized */
	USHORT PollTime;
	INT DfsRssiHigh;
	INT DfsRssiLow;
	BOOLEAN DfsRssiHighFromCfg;
	BOOLEAN DfsRssiLowFromCfg;
	BOOLEAN DfsRssiHighCfgValid;
	BOOLEAN DfsRssiLowCfgValid;	
	BOOLEAN DFSParamFromConfig;	
	BOOLEAN use_tasklet;	
	DFS_VERSION dfs_func;
	BOOLEAN DFSWatchDogIsRunning;
	UCHAR radarDeclared;
	BOOLEAN SymRoundFromCfg;
	BOOLEAN SymRoundCfgValid;
	ULONG idle_time;
	ULONG busy_time;
	UCHAR ch_busy;
	CHAR	ch_busy_countdown;
	UCHAR	busy_channel;
	UCHAR ch_busy_idle_ratio;
	BOOLEAN BusyIdleFromCfg;
	BOOLEAN BusyIdleCfgValid;
	UCHAR print_ch_busy_sta;
	ULONG ch_busy_sta[CH_BUSY_SAMPLE];
	ULONG ch_idle_sta[CH_BUSY_SAMPLE];
	UCHAR ch_busy_sta_index;
	INT		ch_busy_sum;
	INT		ch_idle_sum;
	UCHAR fdf_num;
	USHORT ch_busy_threshold[MAX_FDF_NUMBER];
	INT		rssi_threshold[MAX_FDF_NUMBER];	
	UCHAR McuRadarDebug;
	USHORT McuRadarTick;
	ULONG RadarTimeStampHigh;
	ULONG RadarTimeStampLow;
	UCHAR EnabledChMask;				/* Bit-wise mask for enabled DFS channels */
	DFS_PROGRAM_PARAM DfsProgramParam;
	DFS_SW_DETECT_PARAM DfsSwParam;
} RADAR_DETECT_STRUCT, *PRADAR_DETECT_STRUCT;

typedef struct _NewDFSProgParam
{
	UCHAR channel;
	UCHAR mode;			/* reg 0x10, Detection Mode[2:0]*/
	USHORT avgLen;		/* reg 0x11~0x12, M[7:0] & M[8]*/
	USHORT ELow;		/* reg 0x13~0x14, Energy Low[7:0] & Energy Low[11:8]*/
	USHORT EHigh;		/* reg 0x15~0x16, Energy High[7:0] & Energy High[11:8]*/
	USHORT WLow;		/* reg 0x28~0x29, Width Low[7:0] & Width Low[11:8]*/
	USHORT WHigh;		/* reg 0x2a~0x2b, Width High[7:0] & Width High[11:8]*/
	UCHAR EpsilonW;		/* reg 0x2c, Width Delta[7:0], (Width Measurement Uncertainty) */
	ULONG TLow;			/* reg 0x17~0x1a, Period Low[7:0] & Period Low[15:8] & Period Low[23:16] & Period Low[31:24]*/
	ULONG THigh;		/* reg 0x1b~0x1e, Period High[7:0] & Period High[15:8] & Period High[23:16] & Period High[31:24]*/
	UCHAR EpsilonT;		/* reg 0x27, Period Delt[7:0], (Period Measurement Uncertainty) */
	ULONG BLow;			/* reg 0x1f~0x22, Burst Low[7:0] & Burst Low[15:8] & Burst Low[23:16] & Burst Low[31:24]*/
	ULONG BHigh;		/* reg 0x23~0x26, Burst High[7:0] & Burst High[15:8] & Burst High[23:16] & Burst High[31:24]		*/
}NewDFSProgParam, *pNewDFSProgParam;

#ifdef CONFIG_AP_SUPPORT
VOID NewRadarDetectionStart(
	IN PRTMP_ADAPTER pAd);

VOID NewRadarDetectionStop(
	IN PRTMP_ADAPTER pAd);

void modify_table1(
	IN PRTMP_ADAPTER pAd, 
	IN ULONG idx, 
	IN ULONG value);

void modify_table2(
	IN PRTMP_ADAPTER pAd, 
	IN ULONG idx, 
	IN ULONG value);
  
void schedule_dfs_task(
	 IN PRTMP_ADAPTER pAd);
 
int SWRadarCheck(
	 IN PRTMP_ADAPTER pAd, USHORT id);

VOID NewRadarDetectionProgram(
	IN PRTMP_ADAPTER pAd,
	IN pNewDFSTable pDFS2Table);

BOOLEAN DfsSwCheckOnHwDetection(
	 IN PRTMP_ADAPTER pAd,
	 IN pNewDFSTable pDFS2Table,
	 IN UINT8 DfsChannel,
	 IN ULONG RadarPeriod,
	 IN ULONG RadarWidth);

INT Set_RfReg_Proc(
    IN  PRTMP_ADAPTER   pAdapter, 
    IN  PSTRING          arg);

INT	Show_BlockCh_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ResetRadarHwDetect_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_DfsSwDisable_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT Set_DfsEnvtDropAdjTime_Proc(
	IN PRTMP_ADAPTER   pAd, 
	IN PSTRING  arg);

INT	Set_RadarStart_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarStop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSetTbl1_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSetTbl2_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Fcc5Thrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_ChBusyThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RssiThrd_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PollTime_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PrintBusyIdle_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_RadarSim_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_BusyIdleRatio_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsRssiHigh_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_DfsRssiLow_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN	PSTRING			arg);

INT	Set_Ch0EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING     arg);

INT	Set_Ch1EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING     arg);

INT	Set_Ch2EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING     arg);

INT	Set_Ch3EventExpire_Proc(
	IN	PRTMP_ADAPTER	pAd,
	IN      PSTRING     arg);

INT	Set_CEPrint_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch0LErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_MaxPeriod_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_PeriodErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch0HErr_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch1Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_Ch2Shift_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DeclareThres_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CheckLoop_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

#ifdef DFS_DEBUG
INT	Set_DfsLowerLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_DfsUpperLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_FixDfsLimit_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_AvgRssiReq_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);

INT	Set_CEPrintDebug_Proc(
	IN	PRTMP_ADAPTER	pAd, 
	IN	PSTRING			arg);
#endif /* DFS_DEBUG */

#ifdef WORKQUEUE_BH
void dfs_workq(struct work_struct *work);
#else
void dfs_tasklet(unsigned long data);
#endif /* WORKQUEUE_BH */

VOID DFSInit(
	IN PRTMP_ADAPTER pAd);

VOID ApRadarDetectPeriodic(
	IN PRTMP_ADAPTER pAd);


#ifdef RTMP_MAC_PCI
VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd);
#endif /* RTMP_MAC_PCI */


#endif /* CONFIG_AP_SUPPORT */
#endif /* DFS_SUPPORT */

#endif /*_DFS_H__*/

