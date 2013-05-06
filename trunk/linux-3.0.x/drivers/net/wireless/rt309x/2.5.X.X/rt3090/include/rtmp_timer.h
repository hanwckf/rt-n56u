/*
 ***************************************************************************
 * Ralink Tech Inc.
 * 4F, No. 2 Technology 5th Rd.
 * Science-based Industrial Park
 * Hsin-chu, Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2008, Ralink Technology, Inc.
 *
 * All rights reserved. Ralink's source code is an unpublished work and the
 * use of a copyright notice does not imply otherwise. This source code
 * contains confidential trade secret material of Ralink Tech. Any attemp
 * or participation in deciphering, decoding, reverse engineering or in any
 * way altering the source code is stricitly prohibited, unless the prior
 * written consent of Ralink Technology, Inc. is obtained.
 ***************************************************************************

    Module Name:
	rtmp_timer.h

    Abstract:
	Ralink Wireless Driver timer related data structures and delcarations
	
    Revision History:
	Who           When                What
	--------    ----------      ----------------------------------------------
	Name          Date                 Modification logs
	Shiang Tu    Aug-28-2008	init version
	
*/

#ifndef __RTMP_TIMER_H__
#define  __RTMP_TIMER_H__




#define DECLARE_TIMER_FUNCTION(_func)			\
	void rtmp_timer_##_func(unsigned long data)

#define GET_TIMER_FUNCTION(_func)				\
	(PVOID)rtmp_timer_##_func

typedef struct _LIST_RESOURCE_OBJ_ENTRY
{
	struct _LIST_RESOURCE_OBJ_ENTRY *pNext;
	VOID *pRscObj;
} LIST_RESOURCE_OBJ_ENTRY, *PLIST_RESOURCE_OBJ_ENTRY;


/* ----------------- Timer Related MARCO ---------------*/
// In some os or chipset, we have a lot of timer functions and will read/write register, 
//   it's not allowed in Linux USB sub-system to do it ( because of sleep issue when 
//  submit to ctrl pipe). So we need a wrapper function to take care it.

#ifdef RTMP_TIMER_TASK_SUPPORT
typedef VOID (*RTMP_TIMER_TASK_HANDLE)(
	IN  PVOID   SystemSpecific1, 
	IN  PVOID   FunctionContext, 
	IN  PVOID   SystemSpecific2, 
	IN  PVOID   SystemSpecific3);
#endif // RTMP_TIMER_TASK_SUPPORT //

typedef struct  _RALINK_TIMER_STRUCT    {
	RTMP_OS_TIMER		TimerObj;       // Ndis Timer object
	BOOLEAN				Valid;			// Set to True when call RTMPInitTimer
	BOOLEAN				State;          // True if timer cancelled
	BOOLEAN				PeriodicType;	// True if timer is periodic timer 
	BOOLEAN				Repeat;         // True if periodic timer
	ULONG				TimerValue;     // Timer value in milliseconds
	ULONG				cookie;			// os specific object
	void					*pAd;
#ifdef RTMP_TIMER_TASK_SUPPORT
	RTMP_TIMER_TASK_HANDLE	handle;
#endif // RTMP_TIMER_TASK_SUPPORT //
}RALINK_TIMER_STRUCT, *PRALINK_TIMER_STRUCT;


#ifdef RTMP_TIMER_TASK_SUPPORT
typedef struct _RTMP_TIMER_TASK_ENTRY_
{
	RALINK_TIMER_STRUCT 			*pRaTimer;
	struct _RTMP_TIMER_TASK_ENTRY_ 	*pNext;
}RTMP_TIMER_TASK_ENTRY;


#define TIMER_QUEUE_SIZE_MAX	128
typedef struct _RTMP_TIMER_TASK_QUEUE_
{
	unsigned int				status;
	unsigned char				*pTimerQPoll;
	RTMP_TIMER_TASK_ENTRY	*pQPollFreeList;
	RTMP_TIMER_TASK_ENTRY 	*pQHead;
	RTMP_TIMER_TASK_ENTRY 	*pQTail;
}RTMP_TIMER_TASK_QUEUE;

#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	_pTimer = (PRALINK_TIMER_STRUCT)data;				\
	RTMP_TIMER_TASK_ENTRY	*_pQNode;										\
	RTMP_ADAPTER			*_pAd;											\
																			\
	_pTimer->handle = _func;													\
	_pAd = (RTMP_ADAPTER *)_pTimer->pAd;										\
	_pQNode = RtmpTimerQInsert(_pAd, _pTimer); 								\
	if ((_pQNode == NULL) && (_pAd->TimerQ.status & RTMP_TASK_CAN_DO_INSERT))	\
		RTMP_OS_Add_Timer(&_pTimer->TimerObj, OS_HZ);               					\
}
#else
#define BUILD_TIMER_FUNCTION(_func)										\
void rtmp_timer_##_func(unsigned long data)										\
{																			\
	PRALINK_TIMER_STRUCT	pTimer = (PRALINK_TIMER_STRUCT) data;				\
																			\
	_func(NULL, (PVOID) pTimer->cookie, NULL, pTimer); 							\
	if (pTimer->Repeat)														\
		RTMP_OS_Add_Timer(&pTimer->TimerObj, pTimer->TimerValue);			\
}
#endif // RTMP_TIMER_TASK_SUPPORT //


DECLARE_TIMER_FUNCTION(MlmePeriodicExec);
DECLARE_TIMER_FUNCTION(MlmeRssiReportExec);
DECLARE_TIMER_FUNCTION(AsicRxAntEvalTimeout);
DECLARE_TIMER_FUNCTION(APSDPeriodicExec);
DECLARE_TIMER_FUNCTION(EnqueueStartForPSKExec);

#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(APDetectOverlappingExec);

#ifdef DOT11N_DRAFT3
DECLARE_TIMER_FUNCTION(Bss2040CoexistTimeOut);
#endif // DOT11N_DRAFT3 //

DECLARE_TIMER_FUNCTION(GREKEYPeriodicExec);
DECLARE_TIMER_FUNCTION(CMTimerExec);
DECLARE_TIMER_FUNCTION(WPARetryExec);
#ifdef AP_SCAN_SUPPORT
DECLARE_TIMER_FUNCTION(APScanTimeout);
#endif // AP_SCAN_SUPPORT //
DECLARE_TIMER_FUNCTION(APQuickResponeForRateUpExec);


#ifdef IDS_SUPPORT
DECLARE_TIMER_FUNCTION(RTMPIdsPeriodicExec);
#endif // IDS_SUPPORT //

#endif // CONFIG_AP_SUPPORT //



#ifdef WSC_INCLUDED
DECLARE_TIMER_FUNCTION(WscEAPOLTimeOutAction);
DECLARE_TIMER_FUNCTION(Wsc2MinsTimeOutAction);
DECLARE_TIMER_FUNCTION(WscUPnPMsgTimeOutAction);
DECLARE_TIMER_FUNCTION(WscM2DTimeOutAction);
DECLARE_TIMER_FUNCTION(WscPBCTimeOutAction);
DECLARE_TIMER_FUNCTION(WscScanTimeOutAction);
DECLARE_TIMER_FUNCTION(WscProfileRetryTimeout);
#ifdef CONFIG_AP_SUPPORT
DECLARE_TIMER_FUNCTION(WscUpdatePortCfgTimeout);
DECLARE_TIMER_FUNCTION(WscSetupLockTimeout);
DECLARE_TIMER_FUNCTION(WscPinAttackCountCheckTimeout);
#endif // CONFIG_AP_SUPPORT /
#ifdef WSC_LED_SUPPORT
DECLARE_TIMER_FUNCTION(WscLEDTimer);
DECLARE_TIMER_FUNCTION(WscSkipTurnOffLEDTimer);
#endif // WSC_LED_SUPPORT //
#endif // WSC_INCLUDED //


#if defined (WLAN_LED)
DECLARE_TIMER_FUNCTION(LedCtrlMain);
#endif // WLAN_LED //

#ifdef WMM_ACM_SUPPORT
DECLARE_TIMER_FUNCTION(ACMP_TR_TC_ReqCheck);
DECLARE_TIMER_FUNCTION(ACMP_TR_STM_Check);
DECLARE_TIMER_FUNCTION(ACMP_TR_TC_General);
DECLARE_TIMER_FUNCTION(ACMP_CMD_Timer_Data_Simulation);
#endif // WMM_ACM_SUPPORT //

#endif // __RTMP_TIMER_H__ //

