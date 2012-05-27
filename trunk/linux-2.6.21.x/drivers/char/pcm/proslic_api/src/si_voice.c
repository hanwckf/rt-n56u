/*
** Copyright (c) 2007-2010 by Silicon Laboratories
**
** $Id: si_voice.c,v 1.3 2012-01-09 12:56:42 qwert Exp $
**
** si_voice.c
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
#include "si_voice_datatypes.h"
#include "si_voice.h"

#ifndef DISABLE_MALLOC
//#include "stdlib.h" /* for malloc */
//#include "string.h" /* memset */
#include <linux/slab.h>
#endif

extern void pcm_reset_slic (void);
/* 
** Control object constructor/descructor
*/
int SiVoice_createControlInterface (SiVoiceControlInterfaceType **pCtrlIntf){

#ifndef DISABLE_MALLOC
  	*pCtrlIntf = kmalloc(sizeof(SiVoiceControlInterfaceType), GFP_KERNEL);
	memset (*pCtrlIntf,0,sizeof(SiVoiceControlInterfaceType));
    return 0;
#else
	return -1;
#endif
}

int SiVoice_destroyControlInterface (SiVoiceControlInterfaceType **pCtrlIntf){

#ifndef DISABLE_MALLOC
	kfree ((SiVoiceControlInterfaceType*)*pCtrlIntf);
	return 0;
#else
	return -1;
#endif
}

/* 
** Device object constructor/destructor
*/
int SiVoice_createDevice (SiVoiceDeviceType **pDevice){

#ifndef DISABLE_MALLOC
	*pDevice = kmalloc (sizeof(SiVoiceDeviceType), GFP_KERNEL);
	memset(*pDevice,0,sizeof(SiVoiceDeviceType));
	return 0;
#else
	return -1;
#endif
}
int SiVoice_destroyDevice (SiVoiceDeviceType **pDevice){

#ifndef DISABLE_MALLOC
	kfree ((SiVoiceDeviceType*)*pDevice);
	return 0;
#else
	return -1;
#endif
}

/*
** Channel object constructor/destructor
*/
int SiVoice_createChannel (SiVoiceChanType_ptr *pChan){

#ifndef DISABLE_MALLOC
	*pChan = kmalloc(sizeof(SiVoiceChanType), GFP_KERNEL);
	memset(*pChan,0,sizeof(SiVoiceChanType));
	return 0;
#else
	return -1; 
#endif
}
int SiVoice_destroyChannel (SiVoiceChanType_ptr *hProslic){

#ifndef DISABLE_MALLOC
	kfree ((SiVoiceChanType_ptr)*hProslic);
	return 0;
#else
	return -1;
#endif
}

/*
** Host control linkage
*/
int SiVoice_setControlInterfaceCtrlObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hCtrl){
	pCtrlIntf->hCtrl = hCtrl;
	return 0;
}

/*
** Host reset linkage
*/
int SiVoice_setControlInterfaceReset (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr){
	pCtrlIntf->Reset_fptr = Reset_fptr;
	return 0;
}

/*
** Host register/RAM read/write linkage
*/
int SiVoice_setControlInterfaceWriteRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr){
	pCtrlIntf->WriteRegister_fptr = WriteRegister_fptr;
	return 0;
}
int SiVoice_setControlInterfaceReadRegister (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr){
	pCtrlIntf->ReadRegister_fptr = ReadRegister_fptr;
	return 0;
}
int SiVoice_setControlInterfaceWriteRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr){
	pCtrlIntf->WriteRAM_fptr = WriteRAM_fptr;
	return 0;
}
int SiVoice_setControlInterfaceReadRAM (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr){
	pCtrlIntf->ReadRAM_fptr = ReadRAM_fptr; 
	return 0;
}
/*
** Host timer linkage
*/
int SiVoice_setControlInterfaceTimerObj (SiVoiceControlInterfaceType *pCtrlIntf, void *hTimer){
	pCtrlIntf->hTimer = hTimer;
	return 0;
}
int SiVoice_setControlInterfaceDelay (SiVoiceControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr){
	pCtrlIntf->Delay_fptr = Delay_fptr;
	return 0;
}
int SiVoice_setControlInterfaceSemaphore (SiVoiceControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr){
	pCtrlIntf->Semaphore_fptr = semaphore_fptr;
	return 0;
}
int SiVoice_setControlInterfaceTimeElapsed (SiVoiceControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr){
	pCtrlIntf->timeElapsed_fptr = timeElapsed_fptr;
	return 0;
}
int SiVoice_setControlInterfaceGetTime (SiVoiceControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr){
	pCtrlIntf->getTime_fptr = getTime_fptr;
	return 0;
}

/*
** Channel object initialization
*/
int SiVoice_SWInitChan (SiVoiceChanType_ptr pChan,int channel,int chipType, SiVoiceDeviceType*pDeviceObj, SiVoiceControlInterfaceType *pCtrlIntf){
	pChan->channel = (uInt8)channel;
	pChan->deviceId = pDeviceObj;
	pChan->deviceId->ctrlInterface = pCtrlIntf;
	pChan->channelEnable=1;
	pChan->deviceId->usermodeStatus = 0;
	pChan->deviceId->ctrlInterface->usermodeStatus = 0;
	pChan->error = RC_NONE;
#ifdef PROSLIC_BOM_DEFAULT
	pChan->bomOption = PROSLIC_BOM_DEFAULT;
#else
	pChan->bomOption = 0;
#endif
	switch (chipType){
		case SI321X_TYPE:
			pChan->deviceId->chipType = SI3210;
			break;
		case SI324X_TYPE:
			pChan->deviceId->chipType = SI3240;
			break;
		case SI3220_TYPE:
			pChan->deviceId->chipType = SI3220;
			break;
		case SI3226_TYPE:
			pChan->deviceId->chipType = SI3226;
			break;
		case SI3217X_TYPE:
			pChan->deviceId->chipType = SI32171;
			break;
		case SI3226X_TYPE:
			pChan->deviceId->chipType = SI32260;
			break;
        case SI3050_TYPE:
            pChan->deviceId->chipType = SI3050;
            break;
	}
	return 0;
}

/*
** Reset control
*/
int SiVoice_Reset (SiVoiceChanType_ptr pChan){
    /*
    ** assert reset, wait 250ms, release reset, wait 250ms
    */
    //pChan->deviceId->ctrlInterface->Reset_fptr(pChan->deviceId->ctrlInterface->hCtrl,1);
    //pChan->deviceId->ctrlInterface->Delay_fptr(pChan->deviceId->ctrlInterface->hTimer,250);
    //pChan->deviceId->ctrlInterface->Reset_fptr(pChan->deviceId->ctrlInterface->hCtrl,0);
    //pChan->deviceId->ctrlInterface->Delay_fptr(pChan->deviceId->ctrlInterface->hTimer,250);
    
    pcm_reset_slic();
    return 0;
}

/*
** Debug Mode Control
*/
int SiVoice_setSWDebugMode (SiVoiceChanType_ptr pChan, int debugEn){
	pChan->debugMode = debugEn;
	return 0;
}

/*
** Error status
*/
int SiVoice_getErrorFlag (SiVoiceChanType_ptr pChan, int*error){
	*error = pChan->error;
	return 0;
}
int SiVoice_clearErrorFlag (SiVoiceChanType_ptr pChan){
	pChan->error = RC_NONE;
	return 0;
}

/* 
** Channel status
*/
int SiVoice_setChannelEnable (SiVoiceChanType_ptr pChan, int chanEn){
	pChan->channelEnable = chanEn;
	return 0;
}
int SiVoice_getChannelEnable (SiVoiceChanType_ptr pChan, int* chanEn){
	*chanEn = pChan->channelEnable;
	return 0;
}