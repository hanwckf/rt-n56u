/*
** Copyright (c) 2009 by Silicon Laboratories
**
** $Id: vdaa.c,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** Vdaa.c
** Vdaa  VoiceDAA interface implementation file
**
** Author(s): 
** naqamar, laj, cdp
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This is the implementation file for the main  VoiceDAA API and is used 
** in the  VoiceDAA demonstration code. 
**
** Dependancies:
** Customer Drivers
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "vdaa.h"
#include "vdaa_registers.h"
#include "vdaa_api_config.h"


#ifdef WIN32
#include "stdlib.h"
#ifndef DISABLE_MALLOC
#include "memory.h" 
#endif 
#endif 




#define WriteReg			pVdaa->deviceId->ctrlInterface->WriteRegister_fptr 
#define ReadReg				pVdaa->deviceId->ctrlInterface->ReadRegister_fptr
#define pVdaaHW				pVdaa->deviceId->ctrlInterface->hCtrl
#define Reset				pVdaa->deviceId->ctrlInterface->Reset_fptr
#define Delay				pVdaa->deviceId->ctrlInterface->Delay_fptr
#define pVdaaTimer			pVdaa->deviceId->ctrlInterface->hTimer

#define WriteRegX			deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX			deviceId->ctrlInterface->ReadRegister_fptr
#define pVdaaHWX			deviceId->ctrlInterface->hCtrl
#define DelayX				deviceId->ctrlInterface->Delay_fptr
#define pVdaaTimerX			deviceId->ctrlInterface->hTimer


/*
** Static VDAA driver functions
*/

/*
** Function: isVerifiedDAA
**
** Description: 
** Verifies addressed channel is DAA
**
** Input Parameters: 
** pVdaa: pointer to SiVoiceChanType or vdaaChanType
**
** Return:
** Verified DAA         
** Not DAA          RC_CHANNEL_TYPE_ERR
** 
*/
static int isVerifiedDAA(vdaaChanType *pVdaa){
	uInt8 data = ReadReg(pVdaaHW,pVdaa->channel,LSIDE_REV);
	if ( (data & 0x40) == 0 ) {/*This bit is always 1 for DAA*/
		return RC_CHANNEL_TYPE_ERR;
	}
	else {
	}
	return RC_NONE;
}

/*
** Function: probeDaisyChain
**
** Description: 
** Identify how many VDAA devices are in daisychain
** Only called if channel 0 has be previously qualified
** as a VDAA.
**
** Input Parameters: 
** pVdaa:       pointer to SiVoiceChanType or vdaaChanType
**
** Return:
** number of channels in daisy chain 
** 
*/
static int probeDaisyChain (vdaaChanType *pVdaa){
	int i=0;
	WriteReg(pVdaaHW,BROADCAST,PCMRX_CNT_LO,0x23);				/* Broadcast */
	while ((ReadReg(pVdaaHW,(uInt8)i++,PCMRX_CNT_LO) == 0x23)&&(i<=16));	/* Count number of channels */
	return i-1;													/* Return number of channels */
}


/*
**
** VDAA wrapper functions calling generic SiVoice
** functions (see si_voice.c for function descriptions)
**
*/

int Vdaa_createControlInterface (vdaaControlInterfaceType **pCtrlIntf){
	return SiVoice_createControlInterface(pCtrlIntf);
}
int Vdaa_destroyControlInterface (vdaaControlInterfaceType **pCtrlIntf){
	return SiVoice_destroyControlInterface (pCtrlIntf);
}
int Vdaa_createDevice (vdaaDeviceType **pDev){
	return SiVoice_createDevice(pDev);
}
int Vdaa_destroyDevice (vdaaDeviceType **pDev){
	return SiVoice_destroyDevice (pDev);
}
int Vdaa_createChannel (vdaaChanType **pVdaa){
    return SiVoice_createChannel(pVdaa);
}
int Vdaa_destroyChannel (vdaaChanType **pVdaa){
    return SiVoice_destroyChannel(pVdaa);
}
int Vdaa_setControlInterfaceCtrlObj (vdaaControlInterfaceType *pCtrlIntf, void *hCtrl){
	return SiVoice_setControlInterfaceCtrlObj(pCtrlIntf,hCtrl);
}
int Vdaa_setControlInterfaceReset (vdaaControlInterfaceType *pCtrlIntf, ctrl_Reset_fptr Reset_fptr){
	return SiVoice_setControlInterfaceReset(pCtrlIntf,Reset_fptr);
}
int Vdaa_setControlInterfaceWriteRegister (vdaaControlInterfaceType *pCtrlIntf, ctrl_WriteRegister_fptr WriteRegister_fptr){
	return SiVoice_setControlInterfaceWriteRegister(pCtrlIntf,WriteRegister_fptr);
}
int Vdaa_setControlInterfaceReadRegister (vdaaControlInterfaceType *pCtrlIntf, ctrl_ReadRegister_fptr ReadRegister_fptr){
	return SiVoice_setControlInterfaceReadRegister (pCtrlIntf,ReadRegister_fptr);
}
int Vdaa_setControlInterfaceWriteRAM (vdaaControlInterfaceType *pCtrlIntf, ctrl_WriteRAM_fptr WriteRAM_fptr){
	return 0;
}
int Vdaa_setControlInterfaceReadRAM (vdaaControlInterfaceType *pCtrlIntf, ctrl_ReadRAM_fptr ReadRAM_fptr){
	return 0;
}
int Vdaa_setControlInterfaceTimerObj (vdaaControlInterfaceType *pCtrlIntf, void *hTimer){
	return SiVoice_setControlInterfaceTimerObj(pCtrlIntf,hTimer);
}
int Vdaa_setControlInterfaceDelay (vdaaControlInterfaceType *pCtrlIntf, system_delay_fptr Delay_fptr){
	return SiVoice_setControlInterfaceDelay(pCtrlIntf,Delay_fptr);
}
int Vdaa_setControlInterfaceSemaphore (vdaaControlInterfaceType *pCtrlIntf, ctrl_Semaphore_fptr semaphore_fptr){
	return SiVoice_setControlInterfaceSemaphore(pCtrlIntf,semaphore_fptr);
}
int Vdaa_setControlInterfaceTimeElapsed (vdaaControlInterfaceType *pCtrlIntf, system_timeElapsed_fptr timeElapsed_fptr){
	return SiVoice_setControlInterfaceTimeElapsed(pCtrlIntf,timeElapsed_fptr);
}
int Vdaa_setControlInterfaceGetTime (vdaaControlInterfaceType *pCtrlIntf, system_getTime_fptr getTime_fptr){
	return SiVoice_setControlInterfaceGetTime(pCtrlIntf,getTime_fptr);
}
int Vdaa_SWInitChan (vdaaChanType_ptr pVdaa,int32 channel,int chipType, SiVoiceDeviceType *pDeviceObj, SiVoiceControlInterfaceType *pCtrlIntf){
    return SiVoice_SWInitChan(pVdaa,channel,chipType,pDeviceObj,pCtrlIntf);
}
int Vdaa_setSWDebugMode (vdaaChanType_ptr pVdaa, int32 debugEn){
	return SiVoice_setSWDebugMode(pVdaa,debugEn);
}
int Vdaa_getErrorFlag (vdaaChanType_ptr pVdaa, int32*error){
	return SiVoice_getErrorFlag (pVdaa,error);
}
int Vdaa_clearErrorFlag (vdaaChanType_ptr pVdaa){
	return SiVoice_clearErrorFlag (pVdaa);
}
int Vdaa_setChannelEnable (vdaaChanType_ptr pVdaa, int32 chanEn){
	return SiVoice_setChannelEnable (pVdaa,chanEn);
}
int Vdaa_getChannelEnable (vdaaChanType_ptr pVdaa, int32* chanEn){
	return SiVoice_getChannelEnable (pVdaa,chanEn);
}


/*
**
**  ------ VDAA CONFIGURATION FUNCTIONS -----
**
*/

/*
** Function:  Vdaa_RingDetectSetup
**
** Description: 
** configure ring detect setup
**
** Returns:
**      
*/
#ifdef DISABLE_VDAA_RING_DETECT_SETUP
#else
extern vdaa_Ring_Detect_Cfg Vdaa_Ring_Detect_Presets[];
int Vdaa_RingDetectSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xfB;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rdi<<2;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xfe;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rt&1;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2) & 0xef;
	regTemp |= ((Vdaa_Ring_Detect_Presets[preset].rt>>1)<<4);
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL3,Vdaa_Ring_Detect_Presets[preset].rfwe<<1);
	
	regTemp = (Vdaa_Ring_Detect_Presets[preset].rdly&0x3) << 6;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rmx ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL1,regTemp);
	
	regTemp = (Vdaa_Ring_Detect_Presets[preset].rdly>>2) << 7;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rto << 3 ;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].rcc ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL2,regTemp);
	
	regTemp = Vdaa_Ring_Detect_Presets[preset].rngv << 7;
	regTemp |= Vdaa_Ring_Detect_Presets[preset].ras ;
	WriteReg(pVdaaHW,pVdaa->channel,RNG_VLD_CTRL3,regTemp);

	regTemp = Vdaa_Ring_Detect_Presets[preset].rpol<<1;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL4,regTemp);
	
	return RC_NONE;
}
#endif

/*
** Function:  Vdaa_TXAudioGainSetup
**
** Description: 
** configure tx audio gain
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_AUDIO_GAIN_SETUP
#else
extern vdaa_audioGain_Cfg Vdaa_audioGain_Presets[];
int Vdaa_TXAudioGainSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;
    
    if(Vdaa_audioGain_Presets[preset].xga2 == XGA_ATTEN) {
	    regTemp = 0x10;
    }
	regTemp |= Vdaa_audioGain_Presets[preset].acgain2 ; 
	WriteReg(pVdaaHW,pVdaa->channel,TX_GN_CTRL2,regTemp);
	
    regTemp = 0;
    if(Vdaa_audioGain_Presets[preset].xga3 == XGA_ATTEN) {
	    regTemp = 0x10 ;
    }
	regTemp |= Vdaa_audioGain_Presets[preset].acgain3 ; 
	WriteReg(pVdaaHW,pVdaa->channel,TX_GN_CTRL3,regTemp);

    if(Vdaa_audioGain_Presets[preset].cpEn) {
	    WriteReg(pVdaaHW,pVdaa->channel,TXCALL_PROG_ATTEN,Vdaa_audioGain_Presets[preset].callProgress);
    }

	return RC_NONE;
}
# endif


/*
** Function:  Vdaa_RXAudioGainSetup
**
** Description: 
** configure rx audio gain
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_AUDIO_GAIN_SETUP
#else
extern vdaa_audioGain_Cfg Vdaa_audioGain_Presets[];

int Vdaa_RXAudioGainSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

    if(Vdaa_audioGain_Presets[preset].xga2 == XGA_ATTEN) {
	    regTemp = 0x10;
    }
	regTemp |= Vdaa_audioGain_Presets[preset].acgain2 ; 
	WriteReg(pVdaaHW,pVdaa->channel,RX_GN_CTRL2,regTemp);
	
    regTemp = 0;
    if(Vdaa_audioGain_Presets[preset].xga3 == XGA_ATTEN) {
	    regTemp = 0x10;
    }
	regTemp |= Vdaa_audioGain_Presets[preset].acgain3 ; 
	WriteReg(pVdaaHW,pVdaa->channel,RX_GN_CTRL3,regTemp);

    if(Vdaa_audioGain_Presets[preset].cpEn) {
	    WriteReg(pVdaaHW,pVdaa->channel,RXCALL_PROG_ATTEN,Vdaa_audioGain_Presets[preset].callProgress);
    }

	return RC_NONE;
}
#endif


/*
** Function:  Vdaa_PCMSetup
**
** Description: 
** configure pcm format, clocking and edge placement
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_PCM_SETUP
#else
extern vdaa_PCM_Cfg Vdaa_PCM_Presets [];
int Vdaa_PCMSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL)&0xe0;
	regTemp |= Vdaa_PCM_Presets[preset].pcm_tri;
	regTemp |= Vdaa_PCM_Presets[preset].pcmHwy << 1;
	regTemp |= Vdaa_PCM_Presets[preset].pcmFormat << 3;
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,regTemp);
	
	return RC_NONE;
}
#endif

/*
** Function:  Vdaa_PCMTimeSlotSetup
**
** Description: 
** configure pcm timeslot
**
** Returns:
**     
*/
int Vdaa_PCMTimeSlotSetup (vdaaChanType *pVdaa, uInt16 rxcount, uInt16 txcount){
	uInt8 data = 0;
	uInt8 pcmStatus;
	
    /* Disable PCM if enabled - restore after updating timeslots */
	pcmStatus = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	if (pcmStatus&0x20){
		WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,pcmStatus&~(0x20));
	}

	/*Storing 10 bit value of Transmit PCM sample in REG 34 and REG 35[0:1]*/
	data = (uInt8)(txcount & 0xff);
	WriteReg(pVdaaHW,pVdaa->channel,PCMTX_CNT_LO,data);
	data = (uInt8)(txcount >> 8) ;
	WriteReg(pVdaaHW,pVdaa->channel,PCMTX_CNT_HI,data);
	
	/*Storing 10 bit value of Receive PCM sample in REG 34 and REG 35[0:1]*/
	data = (uInt8)(rxcount & 0xff);
	WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO,data);
	data = (uInt8)(rxcount >> 8);
	WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_HI,data);
	
	/* Enable back the PCM after storing the values*/
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,pcmStatus);

	return RC_NONE;
}

/*
** Function:  Vdaa_CountrySetup
**
** Description: 
** configure country specific settings  
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_COUNTRY_SETUP
#else
extern vdaa_Country_Cfg Vdaa_Country_Presets [];
int Vdaa_CountrySetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp); /* disable hybrid */

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xFD;
	regTemp |= Vdaa_Country_Presets[preset].rz << 1 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);
	
	regTemp = Vdaa_Country_Presets[preset].dcr;
	regTemp |= Vdaa_Country_Presets[preset].ilim<<1;
	regTemp |= Vdaa_Country_Presets[preset].mini<<4;
	regTemp |= Vdaa_Country_Presets[preset].dcv<<6;
	WriteReg(pVdaaHW,pVdaa->channel,DC_TERM_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL) & 0xF0;
	regTemp |= Vdaa_Country_Presets[preset].acim;
	WriteReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL) & 0xAF;
	regTemp |= ((Vdaa_Country_Presets[preset].ohs_sq >> 2)&1)<<4 ;
	regTemp |= ((Vdaa_Country_Presets[preset].ohs_sq >> 3)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xBF;
	regTemp |= ((Vdaa_Country_Presets[preset].ohs_sq >> 1)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL5) & 0xE7;
	regTemp |= (Vdaa_Country_Presets[preset].ohs_sq&1)<<3 ;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL5,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	regTemp |= (Vdaa_Country_Presets[preset].hbe)<<1 ;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp);
	
	return RC_NONE;
}
#endif

/*
** Function:  Vdaa_HybridSetup
**
** Description: 
** configure hybrid 
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_HYBRID_SETUP
#else
extern vdaa_Hybrid_Cfg Vdaa_Hybrid_Presets [];

int Vdaa_HybridSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regSave = 0;

	regSave = ReadReg(pVdaaHW,pVdaa->channel,CTRL2);
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regSave&0xFD); /* disable hybrid */

	WriteReg(pVdaaHW,pVdaa->channel,HYB1,Vdaa_Hybrid_Presets[preset].hyb1);
	WriteReg(pVdaaHW,pVdaa->channel,HYB2,Vdaa_Hybrid_Presets[preset].hyb2);
	WriteReg(pVdaaHW,pVdaa->channel,HYB3,Vdaa_Hybrid_Presets[preset].hyb3);
	WriteReg(pVdaaHW,pVdaa->channel,HYB4,Vdaa_Hybrid_Presets[preset].hyb4);
	WriteReg(pVdaaHW,pVdaa->channel,HYB5,Vdaa_Hybrid_Presets[preset].hyb5);
	WriteReg(pVdaaHW,pVdaa->channel,HYB6,Vdaa_Hybrid_Presets[preset].hyb6);
	WriteReg(pVdaaHW,pVdaa->channel,HYB7,Vdaa_Hybrid_Presets[preset].hyb7);
	WriteReg(pVdaaHW,pVdaa->channel,HYB8,Vdaa_Hybrid_Presets[preset].hyb8);

	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regSave); /* Restore hybrid enable state at entry */
	
	return RC_NONE;
}
#endif


/*
** Function:  Vdaa_ImpedanceSetup (obsoleted in 5.2.0)
**
** Description: 
** configure impedence synthesis
**
** Note:  This function is redundant and will be replaced by
** Vdaa_CountrySetup since more than the terminating impedances
** are configured.  
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_IMPEDANCE_SETUP
#else
extern vdaa_Impedance_Cfg Vdaa_Impedance_Presets [];

int Vdaa_ImpedanceSetup (vdaaChanType *pVdaa,int32 preset){
	uInt8 regTemp = 0;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp); /* disable hybrid */

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xFD;
	regTemp |= Vdaa_Impedance_Presets[preset].rz << 1 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);
	
	regTemp = Vdaa_Impedance_Presets[preset].dcr;
	regTemp |= Vdaa_Impedance_Presets[preset].ilim<<1;
	regTemp |= Vdaa_Impedance_Presets[preset].mini<<4;
	regTemp |= Vdaa_Impedance_Presets[preset].dcv<<6;
	WriteReg(pVdaaHW,pVdaa->channel,DC_TERM_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL) & 0xF0;
	regTemp |= Vdaa_Impedance_Presets[preset].acim;
	WriteReg(pVdaaHW,pVdaa->channel,AC_TERM_CTRL,regTemp);

	WriteReg(pVdaaHW,pVdaa->channel,HYB1,Vdaa_Impedance_Presets[preset].hyb1);
	WriteReg(pVdaaHW,pVdaa->channel,HYB2,Vdaa_Impedance_Presets[preset].hyb2);
	WriteReg(pVdaaHW,pVdaa->channel,HYB3,Vdaa_Impedance_Presets[preset].hyb3);
	WriteReg(pVdaaHW,pVdaa->channel,HYB4,Vdaa_Impedance_Presets[preset].hyb4);
	WriteReg(pVdaaHW,pVdaa->channel,HYB5,Vdaa_Impedance_Presets[preset].hyb5);
	WriteReg(pVdaaHW,pVdaa->channel,HYB6,Vdaa_Impedance_Presets[preset].hyb6);
	WriteReg(pVdaaHW,pVdaa->channel,HYB7,Vdaa_Impedance_Presets[preset].hyb7);
	WriteReg(pVdaaHW,pVdaa->channel,HYB8,Vdaa_Impedance_Presets[preset].hyb8);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL) & 0xAF;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 2)&1)<<4 ;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 3)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,SPRK_QNCH_CTRL,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL1) & 0xBF;
	regTemp |= ((Vdaa_Impedance_Presets[preset].ohs_sq >> 1)&1)<<6 ;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL1,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL5) & 0xF7;
	regTemp |= (Vdaa_Impedance_Presets[preset].ohs_sq&1)<<3 ;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL5,regTemp);

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2) & 0xFD;
	regTemp |= (Vdaa_Impedance_Presets[preset].hbe)<<1 ;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp);
	
	return RC_NONE;
}
#endif

/*
** Function:  Vdaa_LoopbackSetup (obsoleted 5.2.0)
**
** Description: 
** Apply loopback preset
**
** Returns:
**     
**
*/
#ifdef DISABLE_VDAA_LOOPBACK_SETUP
#else
extern vdaa_Loopback_Cfg Vdaa_Loopback_Presets [];

int Vdaa_LoopbackSetup (vdaaChanType *pVdaa, int32 preset){
	uInt8 regTemp;
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL1) & 0xfD;
	WriteReg(pVdaaHW,pVdaa->channel,CTRL1,regTemp | (Vdaa_Loopback_Presets[preset].isoDigLB<<1));
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL3) & 0xfe;
	WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL3,regTemp | (Vdaa_Loopback_Presets[preset].digDataLB));
	return RC_NONE;	
}
#endif


/*
** -----------------------------------
** ------ VDAA CONTROL FUNCTIONS -----
** -----------------------------------
*/

/*
** Function: Vdaa_VerifyControlInterface (obsoleted 5.2.0)
**
** Description: 
** Verify control interface functionality by writing and
** reading back from benign register address.
**
** Input Parameters: 
** pVdaa:       pointer to SiVoiceChanType or vdaaChanType
**
** Return:
** error code
** 
*/
int Vdaa_VerifyControlInterface (vdaaChanType *pVdaa)
{
	int i;
	int numOfChan;
	vdaaChanType vdaa2;   /* Local channel obj to reference chan 0 */
	vdaa2.channel = 0;
	vdaa2.deviceId = pVdaa->deviceId;
	if (isVerifiedDAA(&vdaa2) == RC_CHANNEL_TYPE_ERR) { /*check chan 0 is daa*/
		if (isVerifiedDAA(pVdaa) == RC_CHANNEL_TYPE_ERR) {
			pVdaa->error = RC_CHANNEL_TYPE_ERR;
			return RC_CHANNEL_TYPE_ERR; /*this channel is not daa*/
		}
		else {
			/*we have a daa but can't broadcast*/
			WriteReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO,0x5A);				
			if (ReadReg(pVdaaHW,pVdaa->channel,PCMRX_CNT_LO) != 0x5A)
				return RC_SPI_FAIL;
			return RC_NONE;
		}
	}

	numOfChan = probeDaisyChain(pVdaa);
	if (numOfChan == 0)
		return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
	if (pVdaa->debugMode)
		LOGPRINT ("vdaa: Found %d channels\n",numOfChan);
#endif
	
	/*Try to write innocuous register to test SPI is working*/
	WriteReg(pVdaaHW,BROADCAST,PCMRX_CNT_LO,0x5A);
		
	for (i=0;i<numOfChan;i++){
		
		if (ReadReg(pVdaa,i,PCMRX_CNT_LO) != 0x5A){
			return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
			if (pVdaa->debugMode)
				LOGPRINT("vdaa: %d not communicating. Register access fail.\n",i);
#endif
			return RC_SPI_FAIL;
		}	
		
	}
	return RC_NONE;
}


/*
** Function:  Vdaa_SetAudioMute
**
** Description: 
** Control RX and TX mute
**
** Returns:
**      
*/
int Vdaa_SetAudioMute(vdaaChanType *pVdaa, tMUTE mute)
{
uInt8 regData;

    regData = ReadReg(pVdaaHW,pVdaa->channel,TXRX_GN_CTRL);

    switch(mute) {
        case MUTE_DISABLE_ALL:
            regData = 0;
            break;
        case MUTE_DISABLE_RX:
            regData &= 0x80;
            break;
        case MUTE_DISABLE_TX:
            regData &= 0x08;
            break;
        case MUTE_ENABLE_RX:
            regData |= 0x08;
            break;
        case MUTE_ENABLE_TX:
            regData |= 0x80;
            break;
        case MUTE_ENABLE_ALL:
            regData = 0x88;
            break;
    }
    WriteReg(pVdaaHW,pVdaa->channel,TXRX_GN_CTRL,regData);

    return RC_NONE;
}


/*
** Function:  Vdaa_PCMStart
**
** Description: 
** Enables PCM bus
**
** Returns:
**     
*/
 int Vdaa_PCMStart (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	/*Enable PCM transfers by setting REG 33[5]=1 */
	data = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	data |= 0x20;
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,data);
	return RC_NONE;
}

 /*
** Function:  Vdaa_PCMStop
**
** Description: 
** Disables PCM bus
**
** Returns:
**     
**
*/
int Vdaa_PCMStop (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	/*disable PCM transfers by setting REG 33[5]=0 */
	data = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
	data &= ~(0x20);
	WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL,data);
	return RC_NONE;
}


/*
** Function:  Vdaa_EnableInterrupts
**
** Description: 
** Enables ALL interrupts
** Note:  Use deprecated.  Replaced by Vdaa_SetInterrupts
** which allows individual control. 
**
** Returns:
**     
*/
int Vdaa_EnableInterrupts (vdaaChanType *pVdaa){
	uInt8 data = 0;
	
	WriteReg (pVdaaHW,pVdaa->channel,INTE_MSK,0xff);
		
	/*Current/Voltage Interrupt REG 44*/
	data = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
	data |= 0x02;
	WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data);
	return RC_NONE;
}


/*
** Function:  Vdaa_SetInterruptMask
**
** Description: 
** Enables interrupts based on passed 9-bit bitmask.  Bit
** values defined by vdaaIntMask enum.
**
** Returns:
**     
*/
int Vdaa_SetInterruptMask(vdaaChanType *pVdaa, vdaaIntMask bitmask)
{
uInt8 intMaskReg = 0;
uInt8 cviReg = 0;

    /* Channel validation */
    if(pVdaa->channelType != DAA) 
        return RC_CHANNEL_TYPE_ERR;

    intMaskReg = (uInt8)(bitmask & 0x00ff);
    cviReg = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
    cviReg |= (uInt8) ((bitmask >> 7) & 0x0002);

	WriteReg (pVdaaHW,pVdaa->channel,INTE_MSK,intMaskReg);
	WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,cviReg);

    return RC_NONE;
}

/*
** Function:  Vdaa_ReadRingDetectStatus
**
** Description: 
** Reads ring detect/hook status
**
** Returns:
**     
**
*/
int Vdaa_ReadRingDetectStatus (vdaaChanType *pVdaa,vdaaRingDetectStatusType *pStatus){
uInt8 reg;

    reg = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);
	pStatus->offhook = reg & 0x01;
	pStatus->ringDetected = (reg & 0x4)>>2;
	pStatus->onhookLineMonitor = (reg & 0x8)>>3;
	pStatus->ringDetectedPos = (reg & 0x20)>>5;
	pStatus->ringDetectedNeg = (reg & 0x40)>>6;
	return RC_NONE;
}

/*
** Function:  Vdaa_Init
**
** Description: 
** Initialize VDAA, load general config parameters
**
** Returns:
**     
**
*/
extern vdaa_General_Cfg Vdaa_General_Configuration;
int Vdaa_Init (vdaaChanType_ptr *pVdaa,int size){
	uInt8 data; 
	int16 k;

	for (k=0;k<size;k++) {
        if(pVdaa[k]->channelType != DAA) continue;   /* Skip PROSLIC or UNDEFINED ports */
		/* Read Device ID and verify if SPI is working or not*/
		data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,SYS_LINE_DEV_REV);
		pVdaa[k]->deviceId->chipRev = data&0xF;
		pVdaa[k]->deviceId->lsType= ((data&~(0xF))>>4);
	
		data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,LSIDE_REV);
		pVdaa[k]->deviceId->lsRev= ((data&0x3C)>>2);
		if (isVerifiedDAA(pVdaa[k]) == RC_CHANNEL_TYPE_ERR) {
			pVdaa[k]->channelEnable = FALSE;
			pVdaa[k]->error = RC_CHANNEL_TYPE_ERR;
#ifdef ENABLE_DEBUG
			if (pVdaa[k]->debugMode)
				LOGPRINT("VDAA not supported on this device\n");
#endif
		}
		if (pVdaa[k]->channelEnable) {
			/*Try to write innocuous register to test SPI is working*/
			pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,PCMRX_CNT_LO,0x5A);
            data = pVdaa[k]->ReadRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,PCMRX_CNT_LO);				
			if (data != 0x5A){
				pVdaa[k]->error = RC_SPI_FAIL;
				pVdaa[k]->channelEnable = FALSE;
#ifdef ENABLE_DEBUG
				if (pVdaa[k]->debugMode)
					LOGPRINT("VDAA %d not communicating\n",pVdaa[k]->channel);
#endif
			}
		}
	}
	for (k=0;k<size;k++) {
        if(pVdaa[k]->channelType != DAA) continue;   /* Skip PROSLIC or UNDEFINED ports */
		if (pVdaa[k]->channelEnable) {

        /* Apply General Configuration parameters */

            /* No need to read-modify-write here since registers are in their reset state */
            data = (Vdaa_General_Configuration.pwmm << 4) | (Vdaa_General_Configuration.pwmEnable << 3);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL1, data);  

            data = (Vdaa_General_Configuration.inte << 7) | (Vdaa_General_Configuration.intp << 6) | 0x03;
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,CTRL2, data);  

            data = (Vdaa_General_Configuration.hssm << 3);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,SMPL_CTRL, data);

            data = (Vdaa_General_Configuration.iire << 4);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,INTL_CTRL1, data);
		
            data = (Vdaa_General_Configuration.rcald << 4);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,RES_CALIB, data);

            data = (Vdaa_General_Configuration.full2 << 4);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,AC_TERM_CTRL, data);

            data = (Vdaa_General_Configuration.lvfd) | (Vdaa_General_Configuration.filt << 1) | 
                    (Vdaa_General_Configuration.foh << 5) | (Vdaa_General_Configuration.full << 7);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,DAA_CTRL5, data);
       
            data = (Vdaa_General_Configuration.spim << 6);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,PCM_SPI_CTRL, data); 

            data = (Vdaa_General_Configuration.cvp) | (Vdaa_General_Configuration.cvs << 2);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,LN_VI_THRESH_INTE_CTRL, data);   

            data = (Vdaa_General_Configuration.gce << 1) | (Vdaa_General_Configuration.rg1 << 2);
            pVdaa[k]->WriteRegX(pVdaa[k]->pVdaaHWX,pVdaa[k]->channel,SPRK_QNCH_CTRL, data);  

            /* Enable Lineside Device */
            Vdaa_PowerupLineside(pVdaa[k]);
		}
	}
	return RC_NONE;
}


/*
** Function:  Vdaa_InitBroadcast (currently unsupported)
**
** Description: 
** Initialize VDAA, load general config parameters
** Only valid for chain of SI3050s
**
** Returns:
**     
**
*/
int Vdaa_InitBroadcast (vdaaChanType_ptr pVdaa){

	return RC_UNSUPPORTED_FEATURE;
}

/*
** Function:  Vdaa_Reset
**
** Description: 
** Reset VDAA - calls user's Reset function
**
** Returns:
**     
*/
int Vdaa_Reset (vdaaChanType *pVdaa){
    return SiVoice_Reset(pVdaa);
}

/*
** Function:  Vdaa_ReadLinefeedStatus
**
** Description: 
** Read Status of Line Feed
**
** Returns:
** RC_VDAA_ILOOP_OVLD if LCS >= 0x1F
**      - no overload
**
*/
int Vdaa_ReadLinefeedStatus (vdaaChanType *pVdaa,int8 *vloop, int16 *iloop){
	
	int16 regTemp = 0x1F;	
	uInt8 iloop_reg; /* REG 12[4:0] = Loop current*/
	regTemp &= ReadReg(pVdaaHW,pVdaa->channel,LSIDE_STAT);
	iloop_reg = (uInt8)regTemp;
	*iloop = (regTemp*LCS_SCALE_NUM) / LCS_SCALE_DEN;					/* Multiply the read result by 3.3mA/bit*/
	*vloop = (int8) ReadReg(pVdaaHW,pVdaa->channel,LINE_VOLT_STAT);
    if(*vloop & 0x80) {
        *vloop = ~(*vloop - 1)*(-1);
    }
	if (iloop_reg == 0x1F)
		return RC_VDAA_ILOOP_OVLD;
	return RC_NONE;
}

/*
** Function:  Vdaa_GetInterrupts
**
** Description: 
** Get Interrupts
**
** Returns:
** number of interrupts
**
*/
int Vdaa_GetInterrupts (vdaaChanType *pVdaa,vdaaIntType *pIntData){
	uInt8 data = 0;
	int j;
	pIntData->number = 0;
	
    if(pVdaa->channelType != DAA)
        return RC_IGNORE;
	
	data = ReadReg(pVdaaHW,pVdaa->channel,INTE_SRC);			/*Snapshot Interrupts*/
	WriteReg(pVdaaHW,pVdaa->channel,INTE_SRC,~(data));			/*Clear interrupts*/
	for (j=0;j<8;j++){
			if (data &(1<<j)){
				
				pIntData->irqs[pIntData->number] = 	j;	
				pIntData->number++;
			
			}
	}
		data = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
		
			if (data &(0x08)){									/*to determine if CVI Interrupt is set*/
				pIntData->irqs[pIntData->number] = 	CVI;		
				pIntData->number++;
				data &= ~(0x08);
				WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data);

			}

	return pIntData->number;
}

/*
** Function:  Vdaa_ClearInterrupts
**
** Description: 
** Clear Interrupts
**
** Returns:
**     
**
*/
int Vdaa_ClearInterrupts (vdaaChanType *pVdaa){
	uInt8 data = 0;

    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

	WriteReg(pVdaaHW,pVdaa->channel,INTE_SRC,0x00);		/* Clear interrupts in REG 4 by writing 0's*/
	
	/*Clear CVI interrupt by writing 0 at REG 44[3]*/
	data = ReadReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL);
	WriteReg(pVdaaHW,pVdaa->channel,LN_VI_THRESH_INTE_CTRL,data&0xF7);

	return RC_NONE;
}

/*
** Function:  Vdaa_GetHookStatus
**
** Description: 
** Read VDAA Hook Status
**
** Return Values - 
**      VDAA_ONHOOK
**      VDAA_OFFHOOK
**      VDAA_ONHOOK_MONITOR
**      RC_INVALID_HOOK_STATUS
*/
uInt8 Vdaa_GetHookStatus (vdaaChanType *pVdaa){
	uInt8 data;

    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);
    data &= 0x09;  /* Look at only ONHM and OH */
    if((data & 0x80)&&(data & 0x01)) {
        return VDAA_ONHOOK_MONITOR;
    } else if (data & 0x01) {
        return VDAA_OFFHOOK;
    } else {
        return VDAA_ONHOOK;
    }
}


/*
** Function:  Vdaa_SetHookStatus
**
** Description: 
** Set VDAA Hook switch to ONHOOK, OFFHOOK,
** or ONHOOK_MONITOR
**
** Returns:
**     
*/
int Vdaa_SetHookStatus (vdaaChanType *pVdaa,uInt8 newHookStatus){
	uInt8 data= 0 ;


    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

	switch (newHookStatus){
	    case VDAA_DIG_LOOPBACK:
		    /*Setting REG6[4]=1,REG5[0]=0,REG5[3]=0*/
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);	
		    data |= 0x10;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);
		    data &= ~(0x09);
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		    break;
	    case VDAA_ONHOOK:
		    /*Setting REG6[4]=0,REG5[0]=0,REG5[3]=0*/
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		    data &= 0xF6;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		    data &= 0xEF;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		    break;
	    case VDAA_OFFHOOK:
		    /*Setting REG6[4]=0,REG5[0]=1,REG5[3]=0*/
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		    data &= 0xF7;
		    data |= 0x01;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		    data &= 0xEF;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		    break;
	    case VDAA_ONHOOK_MONITOR:
		    /*Setting REG6[4]=0,REG5[0]=0,REG5[3]=1*/
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL1);	
		    data &= 0xFE;
		    data |= 0x08;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL1,data);
		    data = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL2);
		    data &= 0xEF;
		    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,data);
		    break;
	    default:
		    return RC_UNSUPPORTED_OPTION;
	}

	return RC_NONE;
}

/*
** Function:  Vdaa_SetLoopbackMode
**
** Description: 
** Loopback mode control
**
** Returns:
**     
**
*/
int Vdaa_SetLoopbackMode(vdaaChanType_ptr pVdaa, tLpbkMode lpbk_mode, tLpbkStatus lpbk_status)
{
uInt8 regData;

    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

    switch(lpbk_mode) {
        case LPBK_NONE:
            /* Disable all loopback types, regardless of lpbk_status */
            regData = ReadReg(pVdaaHW,pVdaa->channel,CTRL1); 
            if(regData & 0x02) {
                WriteReg(pVdaaHW,pVdaa->channel,CTRL1, regData & ~(0x02));
            }
            regData = ReadReg(pVdaaHW,pVdaa->channel,DAA_CTRL3); 
            if(regData & 0x01) {
                WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL3,0);
            }
            regData = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL); 
            if(regData & 0x80) {
                WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL, regData & ~(0x80));
            }
        break;
        case LPBK_IDL:
            if(lpbk_status) {
                regData = ReadReg(pVdaaHW,pVdaa->channel,CTRL1);
                WriteReg(pVdaaHW,pVdaa->channel,CTRL1, regData | 0x02);
            }
            else {
                regData = ReadReg(pVdaaHW,pVdaa->channel,CTRL1);
                WriteReg(pVdaaHW,pVdaa->channel,CTRL1, regData & ~(0x02));
            }    
        break;
        case LPBK_DDL:
            if(lpbk_status) {
                WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL3, 0x01);
            }
            else {
                WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL3, 0);
            }
        break;
        case LPBK_PCML:
            if(lpbk_status) {
                regData = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
                WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL, regData | 0x80);
            }
            else {
                regData = ReadReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL);
                WriteReg(pVdaaHW,pVdaa->channel,PCM_SPI_CTRL, regData & ~(0x80));
            }
        break;
        default:
            return RC_UNSUPPORTED_OPTION;
        break;
    }

    return RC_NONE;
}

/*
** Function:  Vdaa_ADCCal
**
** Description: 
** This function calibrates the VDAA ADC manually
**
** Returns:
**     
**
*/
int Vdaa_ADCCal (vdaaChanType_ptr pVdaa, int32 size){
	uInt8 regTemp = 0;

    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

    /* Clear the previous ADC Calibration data by toggling CALZ*/
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2); 
	regTemp |= 0x80;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	regTemp &= ~0x80;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	
	regTemp = ReadReg(pVdaaHW,pVdaa->channel,INTL_CTRL2); /*disable auto cal*/
	regTemp |= 0x20;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);

	regTemp |= 0x40;									/*initiate manual cal*/
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	regTemp &= ~0x40;
	WriteReg(pVdaaHW,pVdaa->channel,INTL_CTRL2,regTemp);
	
	return RC_NONE;
}


/*
** Function:  Vdaa_EnableWatchdog
**
** Description: 
** Enables watchdog timer
**
** Returns:
**     
**
*/
int Vdaa_EnableWatchdog(vdaaChanType_ptr pVdaa)
{
uInt8 regTemp;
  
    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

	regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2); 
	WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp | 0x10);

    return RC_NONE;
}

/*
** Function:  Vdaa_SetHybridEnable
**
** Description: 
** Enables watchdog timer
**
** Returns:
**     
**
*/
int Vdaa_SetHybridEnable(vdaaChanType_ptr pVdaa, int enable)
{
uInt8 regTemp;

    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

    regTemp = ReadReg(pVdaaHW,pVdaa->channel,CTRL2);
    if(enable) {
        WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp | 0x02);
    }
    else {
        WriteReg(pVdaaHW,pVdaa->channel,CTRL2,regTemp & ~(0x02));
    }

    return RC_NONE;
}


/*
** Function:  Vdaa_SoftReset
**
** Description: 
** Execute soft reset
**
** Returns:
**     
**
*/
int Vdaa_SoftReset(vdaaChanType_ptr pVdaa)
{
    if(pVdaa->channelType != DAA)
        return RC_IGNORE;

    WriteReg(pVdaaHW,pVdaa->channel,CTRL1,0);
    return RC_NONE;
}


/*
** Function:  Vdaa_ReadFDTStatus
**
** Description: 
** Read FDT bit
**
** Returns:
** 0 - Frame Not Detected
** 1 - Frame Detected
**
*/
int Vdaa_ReadFDTStatus(vdaaChanType_ptr pVdaa)
{
    if(pVdaa->channelType != DAA)
        return RC_IGNORE;
    
    return (ReadReg(pVdaaHW,pVdaa->channel,LSIDE_REV) & 0x40);
}



/*
** Function:  Vdaa_PowerupLineside
**
** Description: 
** Power up lineside device
**
** Returns:
**     
**
*/
int Vdaa_PowerupLineside(vdaaChanType_ptr pVdaa)
{
int timeout = 0;
uInt8 data;

    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,0); /* Powerup lineside device */
 
    return RC_NONE;
}

/*
** Function:  Vdaa_PowerdownLineside
**
** Description: 
** Power down lineside device
**
** Returns:
**     
**
*/
int Vdaa_PowerdownLineside(vdaaChanType_ptr pVdaa)
{
    WriteReg(pVdaaHW,pVdaa->channel,DAA_CTRL2,0x10);
    return RC_NONE;
}


/*
** Function: Vdaa_PrintDebugData
**
** Description: 
** Dump of VDAA register space
**
** Input Parameters: 
** pVdaa:       pointer to SiVoiceChanType or vdaaChanType
**
** Return:
**     
** 
*/
int Vdaa_PrintDebugData (vdaaChanType *pVdaa){
#ifdef ENABLE_DEBUG
		int i;
		for (i=0;i<60;i++)
			LOGPRINT ("vdaa: Register %d = %X\n",i,ReadReg(pVdaaHW,pVdaa->channel,i));
#endif
		return RC_NONE;
}


/*
** Function: Vdaa_Version
**
** Description: 
** Return API version
**
** Returns:
** 0
*/
extern const char *SiVoiceAPIVersion;
char *Vdaa_Version()
{
	return (char *)SiVoiceAPIVersion;
}