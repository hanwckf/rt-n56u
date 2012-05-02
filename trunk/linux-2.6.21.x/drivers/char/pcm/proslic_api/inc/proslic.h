/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: proslic.h,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** Proslic.h
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
** proslic_datatypes.h
**
*/
#ifndef PROSLIC_H
#define PROSLIC_H

/*include all the required headers*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "si_voice.h"
#include "proslic_api_config.h"
/*
** ----------------ProSLIC Generic DataTypes/Function Definitions----
**********************************************************************
*/

#define MAX_PROSLIC_IRQS 32

#define MAX_ILONG_SAMPLES 32

#define ON_HOOK_TIMEOUT 0x7f

#define MAX_PSTN_SAMPLES 16



/*
** Battery Rail option
*/
typedef enum {
    FIXED,
    TRACKING
} batRailType;


/*
** Map Proslic types to SiVoice types
*/
typedef SiVoiceControlInterfaceType controlInterfaceType;
typedef SiVoiceControlInterfaceType proslicControlInterfaceType;
typedef SiVoiceDeviceType ProslicDeviceType;
typedef SiVoiceChanType proslicChanType;

/*
** Define channel and device type pointers
*/
typedef ProslicDeviceType* proslicDeviceType_ptr;
typedef proslicChanType *proslicChanType_ptr;

/*
** This is structure used to store pulse dial information
*/
typedef struct {
	uInt8 currentPulseDigit;
	void *onHookTime;
	void *offHookTime;
} pulseDialType;

/*
** Defines structure for configuring pulse dial detection
*/
typedef struct {
	uInt8 minOnHook;
	uInt8 maxOnHook;
	uInt8 minOffHook;
	uInt8 maxOffHook;
} pulseDial_Cfg;


/*
** Interrupt tags
*/
typedef enum {
IRQ_OSC1_T1,
IRQ_OSC1_T2,
IRQ_OSC2_T1,
IRQ_OSC2_T2,
IRQ_RING_T1,
IRQ_RING_T2,
IRQ_PM_T1,
IRQ_PM_T2,
IRQ_FSKBUF_AVAIL,
IRQ_VBAT,
IRQ_RING_TRIP,
IRQ_LOOP_STATUS,
IRQ_LONG_STAT,
IRQ_VOC_TRACK,
IRQ_DTMF,
IRQ_INDIRECT,
IRQ_TXMDM,
IRQ_RXMDM,
IRQ_PQ1,
IRQ_PQ2,
IRQ_PQ3,
IRQ_PQ4,
IRQ_PQ5,
IRQ_PQ6,
IRQ_RING_FAIL,
IRQ_CM_BAL,
IRQ_USER_0,
IRQ_USER_1,
IRQ_USER_2,
IRQ_USER_3,
IRQ_USER_4,
IRQ_USER_5,
IRQ_USER_6,
IRQ_USER_7,
IRQ_DSP,
IRQ_MADC_FS,
IRQ_P_HVIC,
IRQ_P_THERM,
IRQ_P_OFFLD
}ProslicInt;

/*
** Defines structure of interrupt data
*/
typedef struct {
	ProslicInt *irqs;
	uInt8 number;
} proslicIntType;


/*
** Defines structure for configuring 1 oscillator
*/
typedef struct {
	ramData freq;
	ramData amp;
	ramData phas;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
} Oscillator_Cfg;


/* 
** Hook states 
*/
enum {
ONHOOK,
OFFHOOK
};

/*
** Loopback modes
*/
typedef enum {
	PROSLIC_LOOPBACK_NONE,
	PROSLIC_LOOPBACK_DIG,
	PROSLIC_LOOPBACK_ANA
} ProslicLoopbackModes;

/*
** Mute options
*/
typedef enum {
	PROSLIC_MUTE_NONE = 0,
	PROSLIC_MUTE_RX = 0x1,
	PROSLIC_MUTE_TX = 0x2,
	PROSLIC_MUTE_ALL = 0x3
} ProslicMuteModes;

/*
** enumeration of the Proslic polarity reversal states
*/
enum {
	POLREV_STOP,
	POLREV_START,
	WINK_START,
	WINK_STOP
};

/*
** Defines initialization data structures
*/
typedef struct {
	uInt8 address;
	uInt8 initValue;
} ProslicRegInit;

typedef struct {
	uInt16 address;
	ramData initValue;
} ProslicRAMInit;


/*
** Linefeed states 
*/
enum {
LF_OPEN,
LF_FWD_ACTIVE,
LF_FWD_OHT,
LF_TIP_OPEN,
LF_RINGING,
LF_REV_ACTIVE,
LF_REV_OHT,
LF_RING_OPEN
} ;


/*
** Linefeed part number options
*/
typedef enum {
	SI3208,
	SI3209,
	SI3203,
	SI3206,
	SI3205,
	SI3201,
	SI3210_DISCRETE
} linefeed;

/*
** ProSLIC patch object
*/
typedef struct {
	const ramData *patchData; /*1024 max*/
	const uInt16 *patchEntries; /* 8 max */
	const uInt32 patchSerial;
    const uInt16 *psRamAddr;  /* 128 max */
    const ramData *psRamData; /* 128 max */
} proslicPatch;


/*
** Ringing type options
*/
typedef enum
{
      ProSLIC_RING_TRAP_CF11,
      ProSLIC_RING_TRAP_CF12,
      ProSLIC_RING_TRAP_CF13,
      ProSLIC_RING_TRAP_CF14,
      ProSLIC_RING_TRAP_CF15,
      ProSLIC_RING_TRAP_CF16,
      ProSLIC_RING_SINE
} ProSLIC_RINGTYPE_T;

/*
** Ringing (provisioned) object
*/
typedef struct 
{
      ProSLIC_RINGTYPE_T ringtype;
	  uInt8 freq;
      uInt8 amp;
      uInt8 offset;
} ProSLIC_dbgRingCfg;


/*
** Line Monitor
*/
typedef struct 
{
    int32  vtr;
    int32  vtip;
    int32  vring;
    int32  vbat;
    int32  itr;
    int32  itip;
    int32  iring;
    int32  ilong;
} proslicMonitorType;

/* 
** Powersave
*/
enum {
    PWRSAVE_DISABLE = 0,
    PWRSAVE_ENABLE = 1
};

/*
** Test State
*/
typedef struct 
{
    int32 stage;
    int32 waitIterations;
    int32 sampleIterations;
}proslicTestStateType;

/*
** PSTN check
*/

/*
** FEMF OPEN voltage test enable
*/
enum {
    FEMF_MEAS_DISABLE,
    FEMF_MEAS_ENABLE
};

/* Standard line interfaces */
typedef struct
{
    int32   avgThresh;
    int32   singleThresh;
    int32   ilong[MAX_ILONG_SAMPLES];
    uInt8   count;
    uInt8   samples;
    int32   avgIlong;
    BOOLEAN buffFull;
} proslicPSTNCheckObjType;

typedef proslicPSTNCheckObjType* proslicPSTNCheckObjType_ptr;


/* Re-Injection line interfaces (differential) */
typedef struct {
    proslicTestStateType pState;
    int dcfPreset1;
    int dcfPreset2;
    int entryDCFeedPreset;
    uInt8 lfstate_entry;
    uInt8 enhanceRegSave;
    uInt8 samples;
    int32 vdiff1[MAX_PSTN_SAMPLES];
    int32 vdiff2[MAX_PSTN_SAMPLES];
    int32 iloop1[MAX_PSTN_SAMPLES];
    int32 iloop2[MAX_PSTN_SAMPLES];
    int32 vdiff1_avg;
    int32 vdiff2_avg;
    int32 iloop1_avg;
    int32 iloop2_avg;
    int32 rl1;
    int32 rl2;
    int32 rl_ratio;
    int femf_enable;
    int32 vdiff_open;
    int32 max_femf_vopen;
    int return_status;
}proslicDiffPSTNCheckObjType;

typedef proslicDiffPSTNCheckObjType* proslicDiffPSTNCheckObjType_ptr;

/*
** proslic.c function declarations
*/
int ProSLIC_createControlInterface (controlInterfaceType **pCtrlIntf);
int ProSLIC_createDevice (ProslicDeviceType **pDevice);
int ProSLIC_createChannel (proslicChanType_ptr *hProslic);
int ProSLIC_destroyChannel (proslicChanType_ptr *hProslic);
int ProSLIC_destroyDevice (ProslicDeviceType **pDevice);
int ProSLIC_destroyControlInterface (controlInterfaceType **pCtrlIntf);
int ProSLIC_setControlInterfaceCtrlObj (controlInterfaceType *pCtrlIntf, void *hCtrl);
int ProSLIC_setControlInterfaceReset (controlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr);
int ProSLIC_setControlInterfaceWriteRegister (controlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr);
int ProSLIC_setControlInterfaceReadRegister (controlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr);
int ProSLIC_setControlInterfaceWriteRAM (controlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr);
int ProSLIC_setControlInterfaceReadRAM (controlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr);
int ProSLIC_setControlInterfaceTimerObj (controlInterfaceType *pCtrlIntf, void *hTimer);
int ProSLIC_setControlInterfaceDelay (controlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr);
int ProSLIC_setControlInterfaceSemaphore (controlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr);
int ProSLIC_setControlInterfaceTimeElapsed (controlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr);
int ProSLIC_setControlInterfaceGetTime (controlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr);
int ProSLIC_SWInitChan (proslicChanType_ptr hProslic,int channel,int chipType, ProslicDeviceType*deviceObj,controlInterfaceType *pCtrlIntf);
int ProSLIC_setSWDebugMode (proslicChanType_ptr hProslic, int debugEn);
int ProSLIC_getErrorFlag (proslicChanType_ptr hProslic, int*error);
int ProSLIC_clearErrorFlag (proslicChanType_ptr hProslic);
int ProSLIC_setChannelEnable (proslicChanType_ptr hProslic, int chanEn);
int ProSLIC_getChannelEnable (proslicChanType_ptr hProslic, int* chanEn);
int ProSLIC_InitializeDialPulseDetect(pulseDialType *pPulse,void *offHookTime,void *onHookTime);
int ProSLIC_VerifyControlInterface (proslicChanType_ptr pProslic);
int ProSLIC_PrintDebugData (proslicChanType *pProslic);
int ProSLIC_Reset (proslicChanType_ptr hProslic);
int ProSLIC_ShutdownChannel (proslicChanType_ptr hProslic);
int ProSLIC_Init (proslicChanType_ptr *hProslic, int size);
int ProSLIC_InitBroadcast (proslicChanType_ptr *hProslic);
int ProSLIC_Cal (proslicChanType_ptr *hProslic, int size);
int ProSLIC_LoadRegTables (proslicChanType_ptr *pProslic,ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int size);
int ProSLIC_LoadPatch (proslicChanType_ptr hProslic,proslicPatch *pPatch);
int ProSLIC_VerifyPatch (proslicChanType_ptr hProslic,proslicPatch *pPatch);
int ProSLIC_EnableInterrupts (proslicChanType_ptr hProslic);
int ProSLIC_SetMuteStatus (proslicChanType_ptr hProslic, ProslicMuteModes muteEn);
int ProSLIC_SetLoopbackMode (proslicChanType_ptr hProslic, ProslicLoopbackModes newMode);
int ProSLIC_RingSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_ToneGenSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_FSKSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_DTMFDecodeSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_ZsynthSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_GciCISetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_ModemDetSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_TXAudioGainSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_RXAudioGainSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_AudioGainSetup (proslicChanType_ptr hProslic,int32 rxgain,int32 txgain,int preset);
int ProSLIC_DCFeedSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_GPIOSetup (proslicChanType_ptr hProslic);
int ProSLIC_PulseMeterSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_PCMSetup (proslicChanType_ptr hProslic,int preset);
int ProSLIC_PCMTimeSlotSetup (proslicChanType_ptr hProslic,uInt16 rxcount, uInt16 txcount);
int ProSLIC_FindInterruptChannels (proslicChanType_ptr *hProslic,uInt8 *pChannelNumbers);
int ProSLIC_GetInterrupts (proslicChanType_ptr hProslic,proslicIntType *pIntData);
int ProSLIC_ReadHookStatus (proslicChanType_ptr hProslic,uInt8 *pHookStat);
int ProSLIC_SetLinefeedStatus (proslicChanType_ptr hProslic,uInt8 newLinefeed);
int ProSLIC_SetLinefeedStatusBroadcast (proslicChanType_ptr hProslic,uInt8 newLinefeed);
int ProSLIC_PolRev (proslicChanType_ptr hProslic,uInt8 abrupt,uInt8 newPolRevState);
int ProSLIC_GPIOControl (proslicChanType_ptr hProslic,uInt8 *pGpioData, uInt8 read);
int ProSLIC_MWI (proslicChanType_ptr hProslic,uInt8 lampOn);
int ProSLIC_ToneGenStart (proslicChanType_ptr hProslic,uInt8 timerEn);
int ProSLIC_ToneGenStop (proslicChanType_ptr hProslic);
int ProSLIC_RingStart (proslicChanType_ptr hProslic);
int ProSLIC_RingStop (proslicChanType_ptr hProslic);
int ProSLIC_EnableCID (proslicChanType_ptr hProslic);
int ProSLIC_DisableCID (proslicChanType_ptr hProslic);
int ProSLIC_CheckCIDBuffer (proslicChanType_ptr hProslic, uInt8 *fsk_buf_avail);
int ProSLIC_SendCID (proslicChanType_ptr hProslic, uInt8 *buffer, uInt8 numBytes);
int ProSLIC_PCMStart (proslicChanType_ptr hProslic);
int ProSLIC_PCMStop (proslicChanType_ptr hProslic);
int ProSLIC_DialPulseDetect (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData);
int ProSLIC_DialPulseDetectTimeout (proslicChanType *pProslic, pulseDial_Cfg *pPulsedialCfg, pulseDialType *pPulseDialData);
int ProSLIC_DTMFReadDigit (proslicChanType_ptr hProslic,uInt8 *pDigit);
int ProSLIC_PLLFreeRunStart (proslicChanType_ptr hProslic);
int ProSLIC_PLLFreeRunStop (proslicChanType_ptr hProslic);
int ProSLIC_PulseMeterStart (proslicChanType_ptr hProslic);
int ProSLIC_PulseMeterStop (proslicChanType_ptr hProslic);
int ProSLIC_PowerUpConverter(proslicChanType_ptr hProslic);
int ProSLIC_PowerDownConverter(proslicChanType_ptr hProslic);
int ProSLIC_LBCal (proslicChanType_ptr *pProslic, int size);
int ProSLIC_GetLBCalResult (proslicChanType *pProslic,int32 *result1,int32 *result2,int32 *result3,int32 *result4);
int ProSLIC_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3,int32 result4);
int ProSLIC_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result);
int ProSLIC_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result);
int ProSLIC_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset);
int ProSLIC_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset);
int ProSLIC_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset);
int ProSLIC_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);
int ProSLIC_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset);
int ProSLIC_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor);
int ProSLIC_CreatePSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj);
int ProSLIC_CreateDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj);
int ProSLIC_DestroyPSTNCheckObj(proslicPSTNCheckObjType_ptr *pstnCheckObj);
int ProSLIC_DestroyDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr *pstnCheckObj);
int ProSLIC_InitPSTNCheckObj(proslicPSTNCheckObjType_ptr pstnCheckObj, int32 avgThresh, int32 singleThresh, uInt8 samples);
int ProSLIC_InitDiffPSTNCheckObj(proslicDiffPSTNCheckObjType_ptr pstnDiffCheckObj, int preset1, int preset2, int entry_preset, int femf_enable);
int ProSLIC_PSTNCheck(proslicChanType *pProslic, proslicPSTNCheckObjType *pPSTNCheck);
int ProSLIC_DiffPSTNCheck(proslicChanType *pProslic, proslicDiffPSTNCheckObjType *pPSTNCheck);
int ProSLIC_SetPowersaveMode(proslicChanType *pProslic, int pwrsave);
int ProSLIC_SetDAAEnable(proslicChanType *pProslic, int enable);
char *ProSLIC_Version(void);
#endif /*end ifdef PROSLIC_H*/

