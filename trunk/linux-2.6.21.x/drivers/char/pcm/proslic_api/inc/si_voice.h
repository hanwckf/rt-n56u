/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: si_voice.h,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** si_voice.h
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the header file for the ProSLIC driver.
**
** Dependancies:
** si_voice_datatypes.h
**
*/

#ifndef SI_VOICE_H
#define SI_VOICE_H

#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"

#define SI321X_TYPE 0
#define SI324X_TYPE 1
#define SI3220_TYPE 2
#define SI3226_TYPE 3
#define SI3217X_TYPE 4
#define SI3226X_TYPE 5
#define SI3050_TYPE 20
/*
** This is the main Silabs control interface object. do not access directly!
*/
typedef struct	{
		void *hCtrl;
		ctrl_Reset_fptr Reset_fptr;
		ctrl_WriteRegister_fptr WriteRegister_fptr;
		ctrl_ReadRegister_fptr ReadRegister_fptr;
		ctrl_WriteRAM_fptr WriteRAM_fptr; /*ProSLIC only*/
		ctrl_ReadRAM_fptr ReadRAM_fptr; /*ProSLIC only*/
		ctrl_Semaphore_fptr Semaphore_fptr; /*ProSLIC only*/
		void *hTimer;
		system_delay_fptr Delay_fptr;
		system_timeElapsed_fptr timeElapsed_fptr;
		system_getTime_fptr getTime_fptr;
		int usermodeStatus; /*ProSLIC only*/
} SiVoiceControlInterfaceType;

typedef enum {
	SI3210, 
	SI3215,
	SI3216,
	SI3211, 	
	SI3212,
 	SI3210M,
 	SI3215M,
 	SI3216M,
	SI3240,
	SI3241,
	SI3242,
	SI3243,
	SI3245,
	SI3244,
	SI3246,
	SI3247,
	SI3220,
	SI3225,
	SI3226,
	SI3227,
	SI32171,
	SI32172,
	SI32174,
	SI32175,
	SI32176,
	SI32177,
	SI32178,
	SI32179,
    SI32260,
    SI32261,
    SI32262,
    SI32263,
    SI32264,
    SI32265,
    SI32266,
    SI32267,
    SI32268,
    SI32269,
    SI32360,
    SI32361,
    SI3050 = 100
}partNumberType;

/*
** Chip revision definition for easier readability in the source code
*/
typedef enum {
	A,
	B,
	C,
	D,
	E,
	F,
	G
}revisionType ;


typedef enum {
    UNKNOWN,
	PROSLIC,
	DAA
} channelTypeType;

/*
** These are the error codes for ProSLIC failures
*/

typedef enum {
    RC_IGNORE = 0,
    RC_NONE = 0,
    RC_COMPLETE_NO_ERR,
    RC_POWER_ALARM_Q1,
    RC_POWER_ALARM_Q2,
    RC_POWER_ALARM_Q3,
    RC_POWER_ALARM_Q4,
    RC_POWER_ALARM_Q5,
    RC_POWER_ALARM_Q6,
    RC_SPI_FAIL,
    RC_POWER_LEAK,
    RC_VBAT_UP_TIMEOUT,
    RC_VBAT_OUT_OF_RANGE,
    RC_VBAT_DOWN_TIMEOUT,
    RC_TG_RG_SHORT,
    RC_CM_CAL_ERR,
    RC_RING_FAIL_INT,
    RC_CAL_TIMEOUT,
    RC_PATCH_ERR,
    RC_BROADCAST_FAIL,
    RC_UNSUPPORTED_FEATURE,
    RC_CHANNEL_TYPE_ERR,
    RC_GAIN_DELTA_TOO_LARGE,
    RC_GAIN_OUT_OF_RANGE,
    RC_POWER_ALARM_HVIC,
    RC_POWER_ALARM_OFFLD,
    RC_THERMAL_ALARM_HVIC,
    RC_NO_MEM,
    RC_INVALID_GEN_PARAM,
    RC_LINE_IN_USE,
    RC_RING_V_LIMITED,
    RC_PSTN_CHECK_SINGLE_FAIL,
    RC_PSTN_CHECK_AVG_FAIL,
    RC_VDAA_ILOOP_OVLD,
    RC_UNSUPPORTED_OPTION,
    RC_FDT_TIMEOUT, 
    RC_PSTN_OPEN_FEMF
} errorCodeType;

/*
** Generic BOM option tags
*/
typedef enum {
	DEFAULT,    
	SI321X_HV,
    BO_DCDC_FLYBACK,
    BO_DCDC_QCUK,
    BO_DCDC_BUCK_BOOST
} bomOptionsType;

/*
** This is the main Voice device object
*/
typedef struct	{
		SiVoiceControlInterfaceType *ctrlInterface;
		revisionType chipRev;
		partNumberType chipType;
		uInt8 lsRev;
		uInt8 lsType;
		int usermodeStatus;
} SiVoiceDeviceType;

typedef SiVoiceDeviceType *SiVoiceDeviceType_ptr;


/*
** This is the main ProSLIC channel object
*/
typedef struct	{
		SiVoiceDeviceType_ptr deviceId;
		uInt8 channel;
		channelTypeType channelType;
		errorCodeType error;
		int debugMode;
		int channelEnable;
		bomOptionsType bomOption;
} SiVoiceChanType;


typedef SiVoiceChanType *SiVoiceChanType_ptr;

int SiVoice_createControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);
int SiVoice_destroyControlInterface (SiVoiceControlInterfaceType **pCtrlIntf);
int SiVoice_createDevice (SiVoiceDeviceType **pDev);
int SiVoice_destroyDevice (SiVoiceDeviceType **pDev);
int SiVoice_createChannel (SiVoiceChanType_ptr *pChan);
int SiVoice_destroyChannel (SiVoiceChanType_ptr *pChan);
int SiVoice_setControlInterfaceCtrlObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hCtrl);
int SiVoice_setControlInterfaceReset (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr);
int SiVoice_setControlInterfaceWriteRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr);
int SiVoice_setControlInterfaceReadRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr);
int SiVoice_setControlInterfaceWriteRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr);
int SiVoice_setControlInterfaceReadRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr);
int SiVoice_setControlInterfaceTimerObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hTimer);
int SiVoice_setControlInterfaceDelay (SiVoiceControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr);
int SiVoice_setControlInterfaceSemaphore (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr);
int SiVoice_setControlInterfaceTimeElapsed (SiVoiceControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr);
int SiVoice_setControlInterfaceGetTime (SiVoiceControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr);
int SiVoice_SWInitChan (SiVoiceChanType_ptr hProslic,int channel,int chipType, SiVoiceDeviceType*pDeviceObj, SiVoiceControlInterfaceType *pCtrlIntf);
int SiVoice_Reset (SiVoiceChanType_ptr pChan);
int SiVoice_setSWDebugMode (SiVoiceChanType_ptr pChan, int debugEn);
int SiVoice_getErrorFlag (SiVoiceChanType_ptr pChan, int*error);
int SiVoice_clearErrorFlag (SiVoiceChanType_ptr pChan);
int SiVoice_setChannelEnable (SiVoiceChanType_ptr pChan, int chanEn);
int SiVoice_getChannelEnable (SiVoiceChanType_ptr pChan, int* chanEn);
#endif
