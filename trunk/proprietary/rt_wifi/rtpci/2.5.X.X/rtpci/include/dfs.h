
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

#define RADAR_PULSE 1
#define RADAR_WIDTH 2

#define WIDTH_RD_IDLE 0
#define WIDTH_RD_CHECK 1

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT

#define CARRIER_DETECT_HIGH_BOUNDARY_1	100
#define CARRIER_DETECT_HIGH_BOUNDARY_2	110
#define CARRIER_DETECT_LOW_BOUNDARY		40
#endif // CARRIER_DETECTION_SUPPORT //
#endif // CONFIG_AP_SUPPORT //


/*************************************************************************
  *
  *	DFS Radar related definitions.
  *
  ************************************************************************/  
//#define CARRIER_DETECT_TASK_NUM	6
//#define RADAR_DETECT_TASK_NUM	7

// McuRadarState && McuCarrierState for 2880-SW-MCU
#define FREE_FOR_TX				0
#define WAIT_CTS_BEING_SENT		1
#define DO_DETECTION			2

// McuRadarEvent
#define RADAR_EVENT_CTS_SENT			0x01 // Host signal MCU that CTS has been sent
#define RADAR_EVENT_CTS_CARRIER_SENT	0x02 // Host signal MCU that CTS has been sent (Carrier)
#define RADAR_EVENT_RADAR_DETECTING		0x04 // Radar detection is on going, carrier detection hold back
#define RADAR_EVENT_CARRIER_DETECTING	0x08 // Carrier detection is on going, radar detection hold back
#define RADAR_EVENT_WIDTH_RADAR			0x10 // BBP == 2 radar detected
#define RADAR_EVENT_CTS_KICKED			0x20 // Radar detection need to sent double CTS, first CTS sent

// McuRadarCmd
#define DETECTION_STOP			0
#define RADAR_DETECTION			1
#define CARRIER_DETECTION		2

#if defined(RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
#define RADAR_GPIO_DEBUG	0x01 // GPIO external debug
#define RADAR_SIMULATE		0x02 // simulate a short pulse hit this channel
#define RADAR_SIMULATE2		0x04 // print any hit
#define RADAR_LOG			0x08 // log record and ready for print

// Both Old and New DFS
#define RADAR_DONT_SWITCH		0x10 // Don't Switch channel when hit

#ifdef DFS_HARDWARE_SUPPORT
// New DFS only
#define RADAR_DEBUG_EVENT			0x01 // print long pulse debug event
#define RADAR_DEBUG_FLAG_1			0x02
#define RADAR_DEBUG_FLAG_2			0x04
#define RADAR_DEBUG_FLAG_3			0x08
#define RADAR_DEBUG_SILENCE			0x4
#define RADAR_DEBUG_SW_SILENCE		0x8
#endif // DFS_HARDWARE_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
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


 VOID NewTimerCB_Radar(
 	IN PRTMP_ADAPTER pAd);
 
 void schedule_dfs_task(
 	IN PRTMP_ADAPTER pAd);


#endif // DFS_HARDWARE_SUPPORT //

#endif // defined (RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)  //


void MCURadarDetect(PRTMP_ADAPTER pAd);

#ifdef TONE_RADAR_DETECT_SUPPORT
void RTMPHandleRadarInterrupt(PRTMP_ADAPTER  pAd);
#else

#ifdef DFS_HARDWARE_SUPPORT
#if defined (RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT)
void RTMPHandleRadarInterrupt(PRTMP_ADAPTER  pAd);
#endif // defined (RTMP_RBUS_SUPPORT) || defined(DFS_INTERRUPT_SUPPORT) //
#endif // DFS_HARDWARE_SUPPORT //
#endif // TONE_RADAR_DETECT_SUPPORT //

#ifdef TONE_RADAR_DETECT_SUPPORT
INT Set_CarrierCriteria_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
int Set_CarrierReCheck_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
INT Set_CarrierStopCheck_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);
void NewCarrierDetectionStart(PRTMP_ADAPTER pAd);
#endif // TONE_RADAR_DETECT_SUPPORT //

#ifdef DFS_SOFTWARE_SUPPORT
VOID BbpRadarDetectionStart(
	IN PRTMP_ADAPTER pAd);

VOID BbpRadarDetectionStop(
	IN PRTMP_ADAPTER pAd);

VOID RadarDetectionStart(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN CTS_Protect,
	IN UINT8 CTSPeriod);

VOID RadarDetectionStop(
	IN PRTMP_ADAPTER	pAd);
#endif // DFS_SOFTWARE_SUPPORT //

VOID RadarDetectPeriodic(
	IN PRTMP_ADAPTER	pAd);
	
#ifdef CONFIG_AP_SUPPORT

VOID ApRadarDetectPeriodic(
	IN PRTMP_ADAPTER pAd);
#ifdef DFS_SOFTWARE_SUPPORT
VOID RadarSMDetect(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN RadarType);
#endif // DFS_SOFTWARE_SUPPORT //
#endif // CONFIG_AP_SUPPORT //

BOOLEAN RadarChannelCheck(
	IN PRTMP_ADAPTER	pAd,
	IN UCHAR			Ch);

ULONG JapRadarType(
	IN PRTMP_ADAPTER pAd);

ULONG RTMPBbpReadRadarDuration(
	IN PRTMP_ADAPTER	pAd);

ULONG RTMPReadRadarDuration(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPCleanRadarDuration(
	IN PRTMP_ADAPTER	pAd);

VOID RTMPPrepareRDCTSFrame(
	IN	PRTMP_ADAPTER	pAd,
	IN	PUCHAR			pDA,
	IN	ULONG			Duration,
	IN  UCHAR           RTSRate,
	IN  ULONG           CTSBaseAddr,
	IN  UCHAR			FrameGap);
#ifdef DFS_SOFTWARE_SUPPORT
VOID RTMPPrepareRadarDetectParams(
	IN PRTMP_ADAPTER	pAd);
#endif // DFS_SOFTWARE_SUPPORT //
#ifdef CONFIG_AP_SUPPORT
VOID AdaptRadarDetection(
	IN PRTMP_ADAPTER pAd);

VOID DFSStartTrigger(
	IN PRTMP_ADAPTER pAd);

INT Set_FastDfs_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);
#endif // CONFIG_AP_SUPPORT //

INT Set_ChMovingTime_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);

INT Set_LongPulseRadarTh_Proc(
	IN PRTMP_ADAPTER pAd, 
	IN PSTRING arg);

#ifdef CONFIG_AP_SUPPORT
#ifdef CARRIER_DETECTION_SUPPORT
static inline INT isCarrierDetectExist(
	IN PRTMP_ADAPTER pAd)
{
	if (pAd->CommonCfg.CarrierDetect.CD_State != CD_NORMAL)
		return TRUE;
	else
		return FALSE;
}		

static inline INT CarrierDetectReset(
	IN PRTMP_ADAPTER pAd)
{
	pAd->CommonCfg.CarrierDetect.CD_State = CD_NORMAL;
	return 0;
}

static inline VOID CarrierDetectionStart(
	IN PRTMP_ADAPTER pAd,
	IN BOOLEAN CTS_Protect)
{
	UINT8 CTSSending = 0x01;
	UINT8 ActiveTime;
	UINT8 Period;
	UINT8 Band;

	//pAd->CommonCfg.CarrierDetect.CDPeriod = 250;
	pAd->CommonCfg.CarrierDetect.CDPeriod = 10; /* 10 * 100ms = 1 sec. */
	pAd->CommonCfg.CarrierDetect.CDSessionTime = 25;

	/* CTSSending = 1 mean start CS without CTS protection,
	** CTSSending = 2 mean start CS with one CTS protection.
	** CTSSending = 3 mean start CS with two CTS protection.
	*/
	CTSSending = (CTS_Protect != 0 ? 0x02 : 0x01);

	/* N[4:0] indicates radar/carrier detection period in unit of 1ms.
	** N[6:5] indicates CTS protect, 0: stop detection,	1: detect with one CTS protect, 3: detect with two CTS protect.
	** N[7] indicates A/G band, 0: A Band, 1: G Band.
	** M[7:0] indicates radar detection period.	
	*/
	Period = pAd->CommonCfg.CarrierDetect.CDPeriod;
	ActiveTime = pAd->CommonCfg.CarrierDetect.CDSessionTime & 0x1f;
	Band = (pAd->CommonCfg.Channel > 14) ? 0 : 1;
	//AsicSendCommandToMcu(pAd, 0x61, 0xff, Period, ActiveTime | (CTSSending << 5) | (Band << 7));
	AsicSendCommandToMcu(pAd, FALSE,0x63, 0xff, Period, ActiveTime | (CTSSending << 5) | (Band << 7)); // perform Carrier-Sense very 1 sec.

	mdelay(5);

	return;
}	

static inline VOID CarrierDetectionStop(
    IN PRTMP_ADAPTER	pAd)
{
	CarrierDetectReset(pAd);

	//AsicSendCommandToMcu(pAd, 0x61, 0xff, 0x00, 0x00);
	AsicSendCommandToMcu(pAd, FALSE,0x63, 0xff, 0x00, 0x00);
	mdelay(5);

	return;
}


#ifdef TONE_RADAR_DETECT_SUPPORT
#define CARRIER_DETECT_START(_P, _F)	NewCarrierDetectionStart((_P))
#define CARRIER_DETECT_STOP(_P)
#else // original RT28xx source code //
#define CARRIER_DETECT_START(_P, _F)	CarrierDetectionStart((_P), (_F))
#define CARRIER_DETECT_STOP(_P)			CarrierDetectionStop((_P))
#endif // TONE_RADAR_DETECT_SUPPORT //


VOID CarrierDetectionFsm(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 CurFalseCCA);

VOID CarrierDetectionCheck(
	IN PRTMP_ADAPTER pAd);

VOID CarrierDetectStartTrigger(
	IN PRTMP_ADAPTER pAd);

INT Set_CarrierDetect_Proc(IN PRTMP_ADAPTER pAd, IN PSTRING arg);

#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DFS_SUPPORT
#ifdef DFS_SOFTWARE_SUPPORT
#ifdef WORKQUEUE_BH
void pulse_radar_detect_workq(struct work_struct *work);
void width_radar_detect_workq(struct work_struct *work);
#else
void pulse_radar_detect_tasklet(unsigned long data);
void width_radar_detect_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //
#endif // DFS_SOFTWARE_SUPPORT //
#endif // DFS_SUPPORT //
#ifdef CARRIER_DETECTION_SUPPORT
#ifdef WORKQUEUE_BH
void carrier_sense_workq(struct work_struct *work);
#else
void carrier_sense_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //
#endif // CARRIER_DETECTION_SUPPORT //

#ifdef DFS_HARDWARE_SUPPORT
#ifdef WORKQUEUE_BH
void dfs_workq(struct work_struct *work);
#else
void dfs_tasklet(unsigned long data);
#endif // WORKQUEUE_BH //
int SWRadarCheck(
	IN PRTMP_ADAPTER pAd, USHORT id);
#endif // DFS_HARDWARE_SUPPORT //

#endif // CONFIG_AP_SUPPORT //

