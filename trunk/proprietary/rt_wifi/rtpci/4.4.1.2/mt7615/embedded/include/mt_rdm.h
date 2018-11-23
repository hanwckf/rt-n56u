#ifdef MTK_LICENSE
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
    mt_rdm.h//Jelly20150123
*/
#endif /* MTK_LICENSE */

#ifndef _MT_RDM_H_
#define _MT_RDM_H_

#ifdef MT_DFS_SUPPORT

#include "rt_config.h"

//Remember add a RDM compile flag -- Shihwei 20141104
/*******************************************************************************
*                              C O N S T A N T S
********************************************************************************
*/

#define RDD_STOP     0
#define RDD_START    1
#define RDD_NODETSTOP    2	
#define RDD_DETSTOP   3
#define RDD_CACSTART    4
#define RDD_CACEND    5
#define RDD_NORMALSTART 6
#define RDD_DFSCAL 7
#define RDD_PULSEDBG 8
#define RDD_READPULSE 9
#define RDD_RESUME_BF 10
#define RDD_FALSE_ALARM_PREVENT 11 

#define MAC_DFS_TXSTART 2

#define HW_RDD0      0
#define HW_RDD1      1
#define HW_RDD_RX0      0
#define HW_RDD_RX1      1
#define RDD_BAND0        0
#define RDD_BAND1        1

#define RESTRICTION_BAND_LOW	116
#define RESTRICTION_BAND_HIGH	128
#define CHAN_SWITCH_PERIOD 10
#define CHAN_NON_OCCUPANCY 1800
#define RADAR_DETECT_NOP_TH 1790
#define GROUP1_LOWER 36
#define GROUP1_UPPER 48
#define GROUP2_LOWER 52
#define GROUP2_UPPER 64

#define GROUP3_LOWER 100
#define GROUP3_UPPER 112

#define RDD_STOP_TXTP_BW20	156
#define RDD_STOP_TXTP_BW40	360
#define RDD_STOP_TXTP_BW80	600
#define RDD_STOP_TXTP_BW160	600

#define DFS_MACHINE_BASE	0

#define DFS_BEFORE_SWITCH    0
#define DFS_INSERVICE_MONITOR    1
#define DFS_RDD_SUSPEND    2
#define DFS_MAX_STATE      	3

#define DFS_CAC_END 0
#define DFS_CHAN_SWITCH_TIMEOUT 1
#define DFS_TP_LOW_HIGH 2
#define DFS_TP_HIGH_LOW 3
#define DFS_MAX_MSG			4

#define DFS_FUNC_SIZE (DFS_MAX_STATE * DFS_MAX_MSG)

/* DFS zero wait */
#define ZeroWaitCacApplyDefault      0xFF  //Apply default setting  
#define BgnScanCacUnit               60000 //unit is 1ms
#define DEFAULT_OFF_CHNL_CAC_TIME    1*BgnScanCacUnit+3000 //6*BgnScanCacUnit //6 mins, unit is 1minute for non-weather band channel
#define WEATHER_OFF_CHNL_CAC_TIME    10*BgnScanCacUnit+3000 //60*BgnScanCacUnit //60 mins, unit is 1minute for weather band channel

#define UPPER_BW_160(_pAd)										\
	(_pAd->CommonCfg.RDDurRegion == FCC ? 64:128)                              

#define CH_NOSUPPORT_BW40(_ch)                                 \
    (_ch == 140 || _ch == 165) 

enum {  
    BW80Group1 = 1, //CH36~48
    BW80Group2,     //CH52~64
    BW80Group3,     //CH100~112 
    BW80Group4,     //CH116~128
    BW80Group5,     //CH132~144
    BW80Group6,     //CH149~161
};


/*******************************************************************************
*                                 M A C R O S
********************************************************************************
*/


/*******************************************************************************
*                             D A T A   T Y P E S
********************************************************************************
*/

typedef struct _DFS_CHANNEL_LIST {
	UCHAR Channel;
	USHORT NonOccupancy;
	UCHAR DfsReq;
	UCHAR Flags;
}DFS_CHANNEL_LIST, *PDFS_CHANNEL_LIST;

typedef enum _ENUM_DFS_STATE_T
{
	DFS_IDLE = 0,      
	DFS_INIT_CAC,        
	DFS_CAC,          //Channel avail check state
	DFS_OFF_CHNL_CAC_TIMEOUT, //Off channel CAC timeout state
	DFS_INSERV_MONI, //In service monitor state
	DFS_RADAR_DETECT,  //Radar detected  state    
	DFS_MBSS_CAC,
} ENUM_DFS_STATE_T;

typedef enum _ENUM_RDD_CTRL_STATE_T
{
	RDD_DEFAULT,
	TP_LOW_HIGH_SUSPEND,
	TP_HIGH_LOW_RESUME,
} ENUM_RDD_CTRL_STATE_T;

typedef enum _ENUM_DET_RATE__MODE_T
{
	NO_CH_SWITCH = 0x0,      
	RDD0_DET_RATE = 0x1,        
	RDD1_DET_RATE = 0x2,
	RDD_BOTH_DET_RATE = 0x3,
} ENUM_DET_RATE__MODE_T;

typedef struct _DFS_PARAM {
	UCHAR Band0Ch;//smaller channel number
	UCHAR Band1Ch;//larger channel number		
	UCHAR PrimCh;
	UCHAR PrimBand;
	UCHAR Bw;
	UCHAR RDDurRegion;
	DFS_CHANNEL_LIST DfsChannelList[MAX_NUM_OF_CHANNELS];
	UCHAR ChannelListNum;
	BOOLEAN bIEEE80211H;
	BOOLEAN DfsChBand[2];	
	BOOLEAN RadarDetected[2];
	DOT11_H Dot11_H;
	UCHAR RegTxSettingBW;
	UCHAR RddCtrlState;
	UCHAR DetRateMode;
	BOOLEAN bDfsCheck;
	BOOLEAN RadarDetectState;
	BOOLEAN IsSetCountryRegion;
	BOOLEAN DisableDfsCal;
	BOOLEAN bShowPulseInfo;
	BOOLEAN bDBDCMode;
	BOOLEAN bDfsEnable;
	BOOLEAN bNoAvailableCh;
	BOOLEAN bDfsIsScanRunning;	
	BOOLEAN bClass2DeauthDisable;
	BOOLEAN bFalseAlarmPrevent;
	/* DFS zero wait */
	BOOLEAN bInitZeroWait;       //TBD
	BOOLEAN bZeroWaitSupport;    //Save the profile setting of DfsZeroWait
	UCHAR   ZeroWaitDfsState;    //for DFS zero wait state machine using  
	UCHAR   DfsZeroWaitCacTime;  //unit is minute and Maximum Off-Channel CAC time is one hour
	BOOLEAN bZeroWaitCacSecondHandle;
	/* MBSS DFS zero wait */
	BOOLEAN bInitMbssZeroWait;

	STATE_MACHINE_FUNC 		DfsStateFunc[DFS_FUNC_SIZE]; 
	STATE_MACHINE 			DfsStatMachine;
} DFS_PARAM, *PDFS_PARAM;

/*******************************************************************************
*                    E X T E R N A L   R E F E R E N C E S
********************************************************************************
*/


/*******************************************************************************
*                   F U N C T I O N   D E C L A R A T I O N S 
********************************************************************************
*/


INT Set_RadarDetectStart_Proc(
    RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_RadarDetectStop_Proc(
    RTMP_ADAPTER *pAd, RTMP_STRING *arg);

INT Set_ByPassCac_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg);

INT Set_RDDReport_Proc(
    RTMP_ADAPTER *pAd, 
    RTMP_STRING *arg);

VOID DfsGetSysParameters(
		IN PRTMP_ADAPTER pAd);

VOID DfsParamUpdateChannelList(//finish
	IN PRTMP_ADAPTER	pAd);

VOID DfsParamInit( // finish
	IN PRTMP_ADAPTER	pAd);

VOID DfsStateMachineInit(
	IN RTMP_ADAPTER *pAd,
	IN STATE_MACHINE *Sm,
	OUT STATE_MACHINE_FUNC Trans[]);

INT Set_DfsChannelShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsBwShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsRDModeShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsRDDRegionShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsNonOccupancyShow_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsNonOccupancyClean_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsPulseInfoMode_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

INT Set_DfsPulseInfoRead_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN RTMP_STRING *arg);

/* DFS Zero Wait */
INT Set_DfsZeroWaitCacTime_Proc(RTMP_ADAPTER *pAd, RTMP_STRING *arg);

BOOLEAN WrapDfsByPassChannel(
	IN PRTMP_ADAPTER pAd, struct wifi_dev *wdev, UCHAR Channel);

VOID DfsTXTPHighToLow(
	IN PRTMP_ADAPTER pAd);

VOID DfsCheckRDDByTXTP(
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsEnterSilence(
    IN PRTMP_ADAPTER pAd);

VOID DfsSetCalibration(
	IN PRTMP_ADAPTER pAd, UINT_32 DisableDfsCal);

VOID DFsSetFalseAlarmPrevent(
	IN PRTMP_ADAPTER pAd, UINT_32 DfsFalseAlarmPrevent);

VOID DfsSetZeroWaitCacSecond(
	    IN PRTMP_ADAPTER pAd);

BOOLEAN DfsRadarChannelCheck(
    IN PRTMP_ADAPTER pAd);

VOID DfsSetCountryRegion(
    IN PRTMP_ADAPTER pAd);

VOID DfsCacEndUpdate(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

VOID DfsChannelSwitchTimeoutAction(
	RTMP_ADAPTER *pAd,
	MLME_QUEUE_ELEM *Elem);

VOID DfsCacNormalStart(
    IN PRTMP_ADAPTER pAd);

BOOLEAN DfsCacRestrictBand(
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam);

VOID DfsSaveNonOccupancy(
    IN PRTMP_ADAPTER pAd);

VOID DfsRecoverNonOccupancy(
    IN PRTMP_ADAPTER pAd);

BOOLEAN DfsSwitchCheck(//finish
		IN PRTMP_ADAPTER pAd,
		UCHAR Channel);

BOOLEAN DfsStopWifiCheck(
    IN PRTMP_ADAPTER	pAd);

#ifdef CONFIG_AP_SUPPORT
VOID DfsChannelSwitchParam( /*channel switch count down, finish*/
	IN PRTMP_ADAPTER	pAd);
#endif /* CONFIG_AP_SUPPORT */

VOID DfsNonOccupancyCountDown( /*RemainingTimeForUse --, finish*/
	IN PRTMP_ADAPTER pAd);

BOOLEAN DfsDetRateModeCheck(
	IN PRTMP_ADAPTER pAd, UCHAR RDMode);

VOID WrapDfsRddReportHandle( /*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx);

BOOLEAN DfsRddReportHandle( /*handle the event of EXT_EVENT_ID_RDD_REPORT*/
	IN PDFS_PARAM pDfsParam, UCHAR ucRddIdx);

VOID DfsCacEndHandle( /*handle the event of EXT_EVENT_ID_CAC_END*/
	IN PRTMP_ADAPTER pAd, UCHAR ucRddIdx);

VOID WrapDfsSetNonOccupancy( /*Set Channel non-occupancy time, finish */
	IN PRTMP_ADAPTER pAd);

VOID DfsSetNonOccupancy( /*Set Channel non-occupancy time, finish*/
	IN PRTMP_ADAPTER pAd, IN PDFS_PARAM pDfsParam);

VOID WrapDfsSelectChannel( /*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd);

VOID DfsSelectChannel( /*Select new channel, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

UCHAR WrapDfsRandomSelectChannel( /*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, BOOLEAN bSkipDfsCh);

UCHAR DfsRandomSelectChannel( /*Select new channel using random selection, finish*/
	IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam, BOOLEAN bSkipDfsCh);

VOID WrapDfsRadarDetectStart( /*Start Radar Detection or not, finish*/
   IN PRTMP_ADAPTER pAd);

VOID DfsRadarDetectStart( /*Start Radar Detection or not, finish*/
   IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

VOID WrapDfsRadarDetectStop( /*Start Radar Detection or not*/
	IN PRTMP_ADAPTER pAd);

VOID DfsRadarDetectStop( /*Start Radar Detection or not, finish*/
   IN PRTMP_ADAPTER pAd, PDFS_PARAM pDfsParam);

BOOLEAN DfsIsScanRunning(
	IN PRTMP_ADAPTER pAd);

VOID DfsSetScanRunning(
	IN PRTMP_ADAPTER pAd, BOOLEAN bIsScan);

BOOLEAN DfsIsClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd);

VOID DfsSetClass2DeauthDisable(
	IN PRTMP_ADAPTER pAd, BOOLEAN bEnable);

UCHAR DfsGetCentCh(IN PRTMP_ADAPTER pAd, IN UCHAR Channel,IN UCHAR bw); 

INT mtRddControl(
    IN struct _RTMP_ADAPTER *pAd,
    IN UCHAR ucRddCtrl,
    IN UCHAR ucRddIdex,
    IN UCHAR ucRddInSel);

#ifdef BACKGROUND_SCAN_SUPPORT
/* Mbss Zero Wait */
BOOLEAN MbssZeroWaitStopValidate(PRTMP_ADAPTER pAd, UCHAR MbssCh, INT MbssIdx);
VOID ZeroWaitUpdateForMbss(PRTMP_ADAPTER pAd, BOOLEAN bZeroWaitStop, UCHAR MbssCh, INT MbssIdx);
#endif /* BACKGROUND_SCAN_SUPPORT */

VOID DfsBFSoundingRecovery(IN struct _RTMP_ADAPTER *pAd);
#endif /*MT_DFS_SUPPORT*/
#endif /*_MT_RDM_H_ */
