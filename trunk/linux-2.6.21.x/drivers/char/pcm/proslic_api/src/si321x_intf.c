/*
** Copyright (c) 2007-2009 by Silicon Laboratories
**
** $Id: si321x_intf.c,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** Proslic.c
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
** This is the interface file for the ProSLIC drivers.
**
** Dependancies:
** proslic_datatypes.h
**
*/
 
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si321x_intf.h"
#include "si321x_registers.h"
#include "proslic_api_config.h"



#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM	pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr

#define WriteRegX		deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX		deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX			deviceId->ctrlInterface->hCtrl
#define DelayX			deviceId->ctrlInterface->Delay_fptr
#define pProTimerX		deviceId->ctrlInterface->hTimer
#define WriteRAMX		deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX		deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsedX	deviceId->ctrlInterface->timeElapsed_fptr

#ifndef SI321X_RSENSE /* Refer to High Voltage App note */
#define SI321X_RSENSE 200
#endif

#define KSENSE_MUL(X) ((SI321X_RSENSE*(X))/200)
#define KSENSE_DIV(X) (200/(SI321X_RSENSE*(X)))
#define KSENSE_MUL_INV(X) (((X)*200)/SI321X_RSENSE)

/*initial values are defined using Si3210 indirect addresses
the address is adjusted if a Si3215 is detected in the function in the 
SPI driver (...proslic_api/example_drivers/win/proslic_ppt_mb/proslic_spiGci) 
called posssibleAddressCorrect()*/
#define	INIT_IR0		0x55C2	/* DTMF_ROW_0_PEAK	*/
#define	INIT_IR1		0x51E6  /*	DTMF_ROW_1_PEAK,*/	
#define	INIT_IR2		0x4B85	/*	DTMF_ROW2_PEAK,	*/	
#define	INIT_IR3		0x4937	/*	DTMF_ROW3_PEAK,	*/
#define	INIT_IR4		0x3333	/*	DTMF_COL1_PEAK,	*/
#define	INIT_IR5		0x0202	/*	DTMF_FWD_TWIST,*/
#define	INIT_IR6		0x0202	/*	DTMF_RVS_TWIST,	*/
#define	INIT_IR7		0x0198	/*	DTMF_ROW_RATIO,	*/
#define	INIT_IR8		0x0198	/*	DTMF_COL_RATIO,	*/	
#define	INIT_IR9		0x0611	/*	DTMF_ROW_2ND_ARM,*/
#define	INIT_IR10		0x0202	/*	DTMF_COL_2ND_ARM,*/	
#define	INIT_IR11		0x00E5	/*	DTMF_PWR_MIN_,	*/
#define	INIT_IR12		0x0A1C	/*	DTMF_OT_LIM_TRES,*/		
#define	INIT_IR13		0x7b30	/*	OSC1_COEF,	*/
#define	INIT_IR14		0x0063	/*	OSC1X,	*/
#define	INIT_IR15		0x0000	/*	OSC1Y,	*/
#define	INIT_IR16		0x7870	/*	OSC2_COEF,*/	
#define	INIT_IR17		0x007d	/*	OSC2X,	*/
#define	INIT_IR18		0x0000	/*	OSC2Y,	*/
#define	INIT_IR19		0x0000	/*	RING_V_OFF*/
#define	INIT_IR20		0x7EF0	/*	RING_OSC,	*/	
#define	INIT_IR21		0x0160	/*	RING_X,	*/
#define	INIT_IR22		0x0000	/*	RING_Y,	*/
#define	INIT_IR23		0x2000	/*	PULSE_ENVEL,*/	
#define	INIT_IR24		0x2000	/*	PULSE_X,*/
#define	INIT_IR25		0x0000	/*	PULSE_Y,*/	
#define	INIT_IR26		0x4000	/*	RECV_DIGITAL_GAIN,*/	
#define	INIT_IR27		0x4000	/*	XMIT_DIGITAL_GAIN,	*/
#define	INIT_IR28		0x1000	/*	LOOP_CLOSE_TRES,	*/
#define	INIT_IR29		0x3600	/*	RING_TRIP_TRES,	*/
#define	INIT_IR30		0x1000	/*	COMMON_MIN_TRES,*/	
#define	INIT_IR31		0x080	/*	COMMON_MAX_TRES,*/
#define	INIT_IR30_HV		0x900	/*	COMMON_MIN_TRES,*/	
#define	INIT_IR31_HV		0x080	/*	COMMON_MAX_TRES,*/	
	
#define	INIT_IR35		0x8000	/*	LOOP_CLSRE_FlTER,*/
#define	INIT_IR36		0x0320	/*	RING_TRIP_FILTER,*/	
	
#define	INIT_IR40		0x200  	/*	CM_BIAS_RINGING,*/	
#define	INIT_IR41		0xC00	/*	DCDC_MIN_V,	*/
#define	INIT_IR40_HV		0x200  	/*	CM_BIAS_RINGING,*/	
#define	INIT_IR41_HV		0x600	/*	DCDC_MIN_V,	*/
#define	INIT_IR42		0x0	    
#define	INIT_IR43		0xE00	/*	"LOOP_CLOSE_TRES Low*/
#define	INIT_IR99		0x00DA	/* FSK 0 FREQ PARAM */
#define	INIT_IR100		0x6B60	/* FSK 0 AMPL PARAM */
#define	INIT_IR101		0x0074	/* FSK 1 FREQ PARAM */
#define	INIT_IR102		0x79C0	/* FSK 1 AMPl PARAM */
#define	INIT_IR103		0x1120	/* FSK 0to1 SCALER */
#define	INIT_IR104		0x3BE0	/* FSK 1to0 SCALER */
#define	INIT_IR97		0x0000	/* TRASMIT_FILTER */
#define INIT_SI3210M_DR92 0x60  /*  92 0x60 Initialization DC-DC Converter PWM Period (61.035 ns/LSB) */
#define INIT_SI3210M_DR93 0x38  /*  92 0x60 Initialization DC-DC Converter PWM Period (61.035 ns/LSB) */
#define	INIT_DR92	0x7f	/*	92 0x5C  7F Initialization DC-DC Converter PWM Period (61.035 ns/LSB) */
#define	INIT_DR93	0x14	/*	93 0x5D 0x14 0x19 Initialization DC-DC Converter Min. Off Time (61.035 ns/LSB) */
#define	INIT_IR32		0x7c0  	/*	PWR_ALARM_Q1Q2, */		
#define	INIT_IR33		0x2600	/*	PWR_ALARM_Q3Q4, */
#define	INIT_IR34		0x1B80	/*	PWR_ALARM_Q5Q6,	 */
#define	INIT_IR37		0x08c	/*	TERM_LP_POLE_Q1Q2, */	
#define	INIT_IR38		0x0100	/*	TERM_LP_POLE_Q3Q4,	 */
#define	INIT_IR39		0x010	/*	TERM_LP_POLE_Q5Q6, */
#define	INIT_DR71	0X01	/*	71 0x47 0x00 0x01 Initialization Off-Hook Loop Current Limit (20 mA + 3 mA/LSB) */
#define	INIT_DR65	0X60	/*	65 0x41 0x61 Initialization External Bipolar Transistor Settings  */

extern Si321x_General_Cfg Si321x_General_Configuration;

typedef struct
{
    int32   gain;
    int16   scale;
} ProSLIC_GainScaleLookup;


/* Gain tables for Si321x */

static const ProSLIC_GainScaleLookup gainScaleTable_0dB[] =
/*  gain, scale=16384* 10^(gain/20) - sorted order */
{ 
    { -12,0x1013},
    { -11,0x1209},
    { -10,0x143D},
    { -9, 0x16B5},
    { -8, 0x197A},
    { -7, 0x1C96},
    { -6, 0x2013},
    { -5, 0x23FD},
    { -4, 0x2861},
    { -3, 0x2D4E},
    { -2, 0x32D6},
    { -1, 0x390A},
    { 0,  0x4000},
    { 1,  0x47CF},
    { 2,  0x5092},
    { 3,  0x5A67},
    { 4,  0x656E},
    { 5,  0x71CF},
    { 6,  0x7FB2},
    {0xff,0} /* terminator */
};

/* This table represents a 3.5 dB offset (either positive or negative) that one
   can use with conjuction of analog gain offset...
*/
static const ProSLIC_GainScaleLookup gainScaleTable_35dB[] =
{
/*  gain, scale=16384* 10^((gain +|- 3.5)/20) - sorted order */

    { -16, 0x0F2D},
    { -15, 0x1107},
    { -14, 0x131B},
    { -13, 0x1570},
    { -12, 0x180D},
    { -11, 0x1AFD},
    { -10, 0x1E48},
    { -9,  0x21FA},
    { -8,  0x261F},
    { -7,  0x2AC6},
    { -6,  0x2FFE},
    { -5,  0x35D9},
    { -4,  0x3C6B},
    { -3,  0x43CA},
    { -2,  0x4C10},
    { -1,  0x5558},
    { 0,   0x5FC2},
    { 1,   0x6B71},
    { 2,   0x788D},
    { 0xFF,0} /* Terminator */
};

/*
** Local functions are defined first
*/

/*
** Function: setDaisyChainMode
*/
static int setDaisyChainMode(proslicChanType *pProslic){
	WriteReg(pProHW,SI321X_BROADCAST,0,0x80);
	if ((ReadReg(pProHW,0,0) & 0xC0) == 0x80)
		return 0;
	/*if daisy set failed assume we were already in daisy mode and resync*/
	WriteReg(pProHW,0,1,0x80);
	WriteReg(pProHW,SI321X_BROADCAST,0,0x0);
	ReadReg(pProHW,0,0);
	WriteReg(pProHW,SI321X_BROADCAST,0,0x80);
	if ((ReadReg(pProHW,0,0) & 0xC0) == 0x80)
		return 0;
	return -1;
}

static int probeDaisyChain (proslicChanType *pProslic){
	uInt8 temp,i=0;
	WriteReg(pProHW,SI321X_BROADCAST,AUDIO_LOOPBACK,2);
	do {
		temp = ReadReg(pProHW,i,AUDIO_LOOPBACK);
	} while ((temp == 2) && (++i<8));
	return i;
}

static uInt8 chipRev(proslicChanType *pProslic){	
	uInt8 versiont, data;
	data = ReadReg(pProHW, pProslic->channel, SPI_MODE);

	versiont = (0xf & data); 

	return (versiont);
}

static uInt8 chipType(proslicChanType *pProslic){
	/*
	chipType corresponds as follows:
	Si3210............... 0
	Si3215/16............ 1
	Si3211............... 2	
	Si3212............... 3 
	Si3210 M............. 4 
	Si3215 M............. 5 
	Si3216 M............. 6
	*/
	uInt8 temp, data;
	
	data = ReadReg(pProHW, pProslic->channel, SPI_MODE);
	temp = ReadReg(pProHW, pProslic->channel, PCM_MODE);
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode){
		LOGPRINT ("si321x %d reg 0 = %X\treg1 = %X\n",pProslic->channel,data,temp);
	}
#endif
	if (temp & 0x80)
	{
		data = ReadReg(pProHW, pProslic->channel, 6);
		switch (data&0xE0)
		{
			case 0:
				return SI3215;	/*Si3215 */
			break;
			case 0x80:
				return SI3216;	/*Si3216 */
			break;
			case 0x60:
				return SI3215M;	/*Si3215 M */
			break;
			case 0xE0:
				return SI3216M;	/*Si3216 M */
			break;
		}
	}
	else
	{
		switch (data&0x30)
		{
			case 0:
				return SI3210;	/*Si3210 */
			break;
			case 0x10:
				return SI3211;	/*Si3211 */
			break;
			case 0x20:
				return SI3212;	/*Si3212 */
			break;
			case 0x30:
				return SI3210M;	/*Si3210 M */
			break;
		}
	}
	return SI3210;
}

static int powerUp(proslicChanType_ptr *pProslic,int size){ 
	uInt16 vBat, data,data2; 
	uInt16 vBatProg;
	uInt16 vBatTarget;
	int j;
	int i=0;

	j=0;
	do {
			if (pProslic[j]->deviceId->chipType == SI3210M || pProslic[j]->deviceId->chipType == SI3215M || pProslic[j]->deviceId->chipType == SI3216M){  
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 92, INIT_SI3210M_DR92);/* M version */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 93, INIT_SI3210M_DR93);/* M version */
			}
			else{
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 93, 0x12);/* Set the PWM period to 1/64kHz to */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 92, 0xff);/* start out slow (prevent voltage runaway)*/
			}
	} while (++j < size);
	
	for (j=0;j<size;j++){
#ifdef ENABLE_DEBUG
		if (pProslic[j]->debugMode){
			LOGPRINT ("si321x %d checking for foreign volt source\n",pProslic[j]->channel);
		}
#endif
		if (pProslic[j]->channelEnable){
			data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel,80);/*vtip*/
			data2 = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel,81);/*vring*/
			if ((data > SI321X_POWERUP_VOLT_THRESH) || (data2 > SI321X_POWERUP_VOLT_THRESH)){
				pProslic[j]->error = RC_VBAT_OUT_OF_RANGE;
				pProslic[j]->channelEnable = 0;
			}
		}
	}

	if (pProslic[0]->bomOption == SI321X_HV) { /*reprogram battery voltages */
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){ /*make sure channel didn't fail foreign voltage check*/
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, BAT_V_HI, 0x1d);/*vbath */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, BAT_V_LO, 0x9);/*vbatl */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, ON_HOOK_V, 0x12);/*voc */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, COMMON_V, 0x1);/*vcm */
			}
		}
	}

	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable){ /*make sure channel didn't fail foreign voltage check */
			pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, PWR_DOWN1, 0);/* Enable DC-DC converter */
		}
	}
	/* Wait for measured VBAT to be "close" to desired value.  Always
	 check for a voltage at least 6-10v below programmed level
	 within 200ms */
	
	vBatProg = pProslic[0]->ReadRegX(pProslic[0]->pProHWX, pProslic[0]->channel, BAT_V_HI);
	vBatTarget = (vBatProg*3)/2;  /*target in volts */
	vBatTarget = (KSENSE_MUL_INV(vBatTarget-6)*8)/3;
    
	vBat=0;
	i=0;
	while ((vBat < vBatTarget) && (i < 2)){ 
		vBat = vBatTarget;
		for (j=0;j<size;j++){ /* wait for all channels to power up */
			if (pProslic[j]->channelEnable){
#ifdef ENABLE_DEBUG
				if (pProslic[j]->debugMode){
					LOGPRINT ("si321x %d bringing up vbat %d\n",pProslic[j]->channel,i);
				}
#endif
				data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, BAT_V_HI_SENSE);
				if (data < vBatTarget){
					vBat = data;
				}
			}
		
			if ((i > 2) && (vBat < vBatTarget)){
#ifdef ENABLE_DEBUG
				if (pProslic[j]->debugMode){
					LOGPRINT("si321x %d Timed out during POWER UP\n",pProslic[j]->channel);
				}
#endif
				pProslic[j]->error = RC_VBAT_UP_TIMEOUT;
			}
		}
		pProslic[0]->DelayX(pProslic[0]->pProTimerX, 100);
		++i;
		
	}
	
	/* Program desired PWM rate and start calibraton */
	j=0;
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x running cal 1\n");
	}
#endif
	do {
			if (pProslic[j]->deviceId->chipType == SI3210M || pProslic[j]->deviceId->chipType == SI3215M || pProslic[j]->deviceId->chipType == SI3216M){ 
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, DCDC_PWM_OFF, INIT_SI3210M_DR92);/* M version */
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel,DCDC,INIT_SI3210M_DR93);/* M version */
				pProslic[j]->DelayX(pProslic[j]->pProTimerX, 30);
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, DCDC,0x80|INIT_SI3210M_DR93);/* M version */
			}
			else{
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, DCDC_PWM_OFF,INIT_DR92);
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, DCDC, INIT_DR93);
				pProslic[j]->DelayX(pProslic[j]->pProTimerX,30);
				pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, DCDC, 0x80|INIT_DR93);

			}
	} while (++j < size);

	i=0;
	do{
		data2 = 0;
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 93);
				data2 |= data;
				if (data&0x80){
					if (i>10){
						pProslic[j]->error=RC_CAL_TIMEOUT;
#ifdef ENABLE_DEBUG
						if (pProslic[j]->debugMode){
							LOGPRINT ("si321x %d cal timeout\n",pProslic[j]->channel);
						}
#endif
					}
				}
			}
		}
		pProslic[0]->DelayX(pProslic[0]->pProTimerX, 100);
		if (i>10)
			data2 = 0;
		++i;
		
	}while(0x80&data2);  /* Wait for DC-DC Calibration to complete */

	
	return 0;
}

/*
** Function: handleError
**
** Description: 
** Called whenever an error is encountered with the proslic
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** fault: error code
**
** Return:
** none
*/
static void handleError (proslicChanType *pProslic, errorCodeType fault){
#ifdef ENABLE_DEBUG
if (pProslic->debugMode)
	LOGPRINT ("Error encountered\n\n");
#endif

}



/*
** Functions below are defined in header file and can be called by external files
*/

/*
**
** PROSLIC INITIALIZATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_Reset
**
** Description: 
** Resets the ProSLIC
*/
int Si321x_Reset (proslicChanType_ptr pProslic){
	/*
	** resets ProSLIC, wait 250ms, release reset, wait 250ms
	*/
	Reset(pProHW,1);
	Delay(pProTimer,250);
	Reset(pProHW,0);
	Delay(pProTimer,250);
	setDaisyChainMode(pProslic);
	return 0;
}

static int LoadRegTables (proslicChanType_ptr pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable,int broadcast){
	uInt16 i;
	uInt8 channel = pProslic->channel;
	if (broadcast)
		channel = SI321X_BROADCAST;
	if (pProslic->channelEnable == 0)
		return 1;
	if (pRamTable != 0){
		
			
				i=0;
				while (pRamTable[i].address != 0xffff){
					WriteRAM(pProHW,pProslic->channel,pRamTable[i].address,pRamTable[i].initValue);
					i++;
				}
			
		
	}
	if (pRegTable != 0){
		
		
				i=0;
				while (pRegTable[i].address != 0xff){
					WriteReg(pProHW,pProslic->channel,pRegTable[i].address,pRegTable[i].initValue);/* broadcast */
					i++;
				}
		
	}
	return 0;
}

int Si321x_PrintDebugData(proslicChanType_ptr hProslic){
	return 0;
}
int Si321x_VerifyControlInterface (proslicChanType_ptr pProslic)
{
	int i;
	int numOfChan = probeDaisyChain(pProslic);
	if (numOfChan == 0)
		return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Found %d channels\n",numOfChan);
#endif
	WriteReg(pProHW,SI321X_BROADCAST,PCM_RCV_START_COUNT_LSB,0x5a);
	/*WriteRAM(pProHW,SI321X_BROADCAST,448,0x1234);*/

	for (i=0;i<numOfChan;i++){
		/*Try to write innocuous register to test SPI is working*/
		
		if (ReadReg(pProHW,i,PCM_RCV_START_COUNT_LSB) != 0x5A){
			handleError(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
			if (pProslic->debugMode)
				LOGPRINT("Proslic %d not communicating. Register access fail.\n",i);
#endif
			return RC_SPI_FAIL;
		}	
		
	}
	return 0;
}
/*
** Function: PROSLIC_LoadRegTables
**
** Description: 
** Loads registers and ram in the ProSLIC
*/
int Si321x_LoadRegTables (proslicChanType_ptr *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable,int size){

	int j;
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable)	
			LoadRegTables(pProslic[j],pRamTable,pRegTable,0);
	}
	return 0;
}

/*
** Function: PROSLIC_LoadPatch
**
** Description: 
** Loads patch to the ProSLIC
*/
int Si321x_LoadPatch (proslicChanType *pProslic, proslicPatch *pPatch){
	
	return 0; /* we dont load patches for si321x */
}

static void flushAccumulatorBroadcast (proslicChanType_ptr pProslic){
	int i;
	
		if (pProslic->channelEnable){
			for (i=88;i<212;i++){
				WriteRAM(pProHW,SI321X_BROADCAST,i,0);
			}
		}
	
}

static void flushAccumulator (proslicChanType_ptr *pProslic, int size){
	int i,j;
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable){
			for (i=88;i<212;i++){
				pProslic[j]->WriteRAMX(pProslic[j]->pProHWX,pProslic[j]->channel,i,0);
			}
		}
	}
}
/*
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLICs
*/
int Si321x_Init (proslicChanType_ptr *pProslic, int size){
	/*
	** This function will initialize the chipRev and chipType members in pProslic
	** as well as load the initialization structures.
	*/

	uInt8 data;
	controlInterfaceType * tmp;
	int j;
	tmp = pProslic[0]->deviceId->ctrlInterface;

	if (size == 0)
		return 0;
	/* First test SPI is working properly. STEP 8 AN35*/
	for (j=0;j<size;j++){

		data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 8);
		if (data !=2) {
			if (pProslic[j]->debugMode)
				LOGPRINT("si321x %d not communicating. Reg 8 = %X\n",pProslic[j]->channel, data);
			pProslic[j]->error =  RC_SPI_FAIL;
			pProslic[j]->channelEnable = 0;
		}
	}
	for (j=0;j<size;j++){
		data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 11);
		if (data !=0x33) {
#ifdef ENABLE_DEBUG
			if (pProslic[j]->debugMode)
				LOGPRINT("si321x %d not communicating. Reg 11 = %X\n",pProslic[j]->channel, data);
#endif
			pProslic[j]->error = RC_SPI_FAIL;
			pProslic[j]->channelEnable = 0;
		}
	}

	/* Read chip type and revision*/
	for (j=0;j<size;j++){
		pProslic[j]->deviceId->chipRev = chipRev(pProslic[j]); 
		pProslic[j]->deviceId->chipType= chipType(pProslic[j]);
	}
	
	/*STEP 9 AN35*/
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x loading indirect registers\n");
	}
	for (j=0;j<size;j++){
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	0	,	INIT_IR0);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	1	,	INIT_IR1);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	2	,	INIT_IR2);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	3	,	INIT_IR3);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	4	,	INIT_IR4);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	5	,	INIT_IR5);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	6	,	INIT_IR6);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	7	,	INIT_IR7);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	8	,	INIT_IR8);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	9	,	INIT_IR9);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	10	,	INIT_IR10);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	11	,	INIT_IR11);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	12	,	INIT_IR12);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	13	,	INIT_IR13);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	14	,	INIT_IR14);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	15	,	INIT_IR15);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	16	,	INIT_IR16);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	17	,	INIT_IR17);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	18	,	INIT_IR18);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	19	,	INIT_IR19);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	20	,	INIT_IR20);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	21	,	INIT_IR21);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	22	,	INIT_IR22);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	23	,	INIT_IR23);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	24	,	INIT_IR24);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	25	,	INIT_IR25);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	26	,	INIT_IR26);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	27	,	INIT_IR27);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	28	,	INIT_IR28);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	29	,	INIT_IR29);

		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	30	,	INIT_IR30);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	31	,	INIT_IR31);
		if (pProslic[j]->bomOption == SI321X_HV) {
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	30	,	INIT_IR30_HV);
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	31	,	INIT_IR31_HV);
		}
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	32	,	INIT_IR32);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	33	,	INIT_IR33);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	34	,	INIT_IR34);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	35	,	0x8000);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	36	,	0x8000);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	37	,	0x8000);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	38	,	0x8000);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	39	,	0x8000);

		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	40	,	INIT_IR40);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	41	,	INIT_IR41);
		if (pProslic[j]->bomOption == SI321X_HV) {
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	40	,	INIT_IR40_HV);
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	41	,	INIT_IR41_HV);
		}
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	43	,	INIT_IR43);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	99	,	INIT_IR99);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	101	,	INIT_IR101);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	102	,	INIT_IR102);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	103	,	INIT_IR103);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	104	,	INIT_IR104);

	}
	

	for (j=0;j<size;j++) { 
			pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, AUDIO_LOOPBACK, 0); /* STEP 10 AN35 */
			pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 108, 0xEB); /* STEP 10 AN35 */
	}

	powerUp(pProslic,size);         /* STEP 12 AN35.  Turn on the DC-DC converter and verify voltage. */
	
	for (j=0;j<size;j++) {
			pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 64, 0); /* STEP 10 AN35 */
	}
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x running cal 2\n");
	}
#endif
	Si321x_Cal(pProslic,size); /* STEPS 14-20 */
    /*----------------------------
	   STEP 20: Flush Accumulators
	 ----------------------------*/
	
	flushAccumulator(pProslic,size);

	for (j=0;j<size;j++){/* clear pending interupts (step21) */
		data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS1);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS1, data);
		data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS2);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS2, data);
		data = pProslic[j]->ReadRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS3);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, INTRPT_STATUS3, data);

	}
	for (j=0;j<size;j++){
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	35	,	INIT_IR35);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	36	,	INIT_IR36);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	37	,	INIT_IR37);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	38	,	INIT_IR38);
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	39	,	INIT_IR39);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 65, INIT_DR65);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 71, INIT_DR71);
#ifdef INIT_PWR_ALARM_Q1Q2
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	32	,	INIT_PWR_ALARM_Q1Q2);
#endif
#ifdef INIT_PWR_ALARM_Q3Q4
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	33	,	INIT_PWR_ALARM_Q3Q4);
#endif
#ifdef INIT_PWR_ALARM_Q5Q6
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	34	,	INIT_PWR_ALARM_Q5Q6);
#endif
#ifdef INIT_THERM_LP_POLE_Q1Q2
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	37	,	INIT_THERM_LP_POLE_Q1Q2);
#endif
#ifdef INIT_THERM_LP_POLE_Q3Q4
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	38	,	INIT_THERM_LP_POLE_Q3Q4);
#endif
#ifdef INIT_THERM_LP_POLE_Q5Q6
		pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	39	,	INIT_THERM_LP_POLE_Q5Q6);
#endif
		if (pProslic[0]->bomOption == SI321X_HV) {
#ifdef INIT_PWR_ALARM_Q1Q2_HV
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	32	,	INIT_PWR_ALARM_Q1Q2_HV);
#endif
#ifdef INIT_PWR_ALARM_Q3Q4_HV
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	33	,	INIT_PWR_ALARM_Q3Q4_HV);
#endif
#ifdef INIT_PWR_ALARM_Q5Q6_HV
			pProslic[j]->WriteRAMX(pProslic[j]->pProHWX, pProslic[j]->channel,	34	,	INIT_PWR_ALARM_Q5Q6_HV);
#endif
		}
	}

	return 0;
}

int Si321x_InitBroadcast (proslicChanType_ptr *hProslic){
	/*this is temporary. function will be updated to truly broadcast*/
	uInt8 i,size;
	size = probeDaisyChain(hProslic[0]);
	for (i=0;i<size;i++){
        hProslic[i]->channel = i;
        hProslic[i]->channelEnable = 1;
        hProslic[i]->error = RC_NONE;
	}
	Si321x_Init(hProslic,size);
	return 0;	
}
/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
*/
int Si321x_Cal(proslicChanType_ptr *pProslic, int size){
	int i=0;
	uInt8 orData;
	int32 timeOut,nCalComplete;
	uInt32 manCalStat=0;
	int j; 
	if (size == 0)
		return 0;
	/*-----------------------------
	   STEP 14-15: SLIC calibration
	  ----------------------------*/
	/*DISABLE_ALL_DR21*/
	j=0;
	do {
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 21, 0);/*Disable all interupts in DR21 */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 22, 0);/*Disable all interupts in DR22 */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 23, 0);/*Disabel all interupts in DR23 */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 64, 0); /*OPEN_DR64 = 0 */
		/* STANDARD_CAL_DR97 */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 97, 0x1E);/*Calibrations without the ADC and DAC offset and without common mode calibration.*/
		/* STANDARD_CAL_DR96 */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 96, 0x47);/*(0x47)Calibrate common mode and differential DAC mode DAC + ILIM */
	}while (++j < size);
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x %d cmdac and difdac cal start\n",pProslic[0]->channel);
	}
#endif
   	do{
		timeOut = (i++ > 800);/* (800) MS */
		orData = 0;
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				nCalComplete = pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 96);
				if (nCalComplete && (i > 800)){
					pProslic[j]->error=RC_CAL_TIMEOUT;
				}
				orData |= nCalComplete;
			}
		}
        nCalComplete = (orData != 0);/* (0)  When Calibration completes DR 96 will be zero */
	    pProslic[0]->DelayX(pProslic[0]->pProTimerX, 1);
	} while (nCalComplete && (!timeOut));
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x manual cal ring starting\n");
	}   
#endif
    /*----------------------------
	   STEP 16: Manual Calibration
	  ----------------------------*/
	pProslic[0]->DelayX(pProslic[0]->pProTimerX, 10);
	j=0;
	do {
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 99, 0x10);/* Initialize TIP gain cal while RING being calibrated */
	} while (++j < size);
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable)
			manCalStat |= (1<<j);
	}
	/* Ring Gain Calibration */
	for(i=0x1f; i>0; i--){
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				if (manCalStat & (1<<j))
					pProslic[j]->WriteRegX(pProslic[j]->pProHWX,pProslic[j]->channel, 98, i);
			}
		}
		pProslic[0]->DelayX(pProslic[0]->pProTimerX, 40);
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				if (manCalStat & (1<<j)){
					if(pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 88) == 0){
						manCalStat &= ~(1<<j);
					}
				}
			}
		}
		if (manCalStat == 0)
			i = 0;

	} /* for */
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable)
			manCalStat |= (1<<j);
	}
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		LOGPRINT ("si321x manual cal tip starting\n");
	}
#endif
	/* TIP Gain Calibration */
	for(i=0x1f; i>0; i--){
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				if (manCalStat & (1<<j))
					pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 99, i);
			}
		}
		pProslic[0]->DelayX(pProslic[0]->pProTimerX, 40);
		for (j=0;j<size;j++){
			if (pProslic[j]->channelEnable){
				if (manCalStat & (1<<j)){
					if(pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 89) == 0){
						manCalStat &= ~(1<<j);
					}
				}
			}
		}
		if (manCalStat == 0)
			i = 0;
		

	} /* for */
	
	/*------------------------------
	   STEP 17-19:  Long Balance Cal
	  ------------------------------*/
	
	for (j=0;j<size;j++){
		if (pProslic[j]->debugMode){
			LOGPRINT ("si321x %d long balance cal starting\n",pProslic[j]->channel);
		}
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 23, (1<<2)); /* enable interrupt for the balance Cal */
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 64, 1);
	}
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
			LOGPRINT ("si321x %d long balance cal waiting for active line(s) to charge\n",pProslic[0]->channel);
	}
#endif
	pProslic[0]->DelayX(pProslic[0]->pProTimerX,250);
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable){
			if (pProslic[j]->ReadRegX(pProslic[j]->pProHWX,pProslic[j]->channel,LOOP_STAT) & 1){ /* offhook */
				if (pProslic[j]->debugMode)
					LOGPRINT("si321x %d Calibration failure due to off-hook phone\n",pProslic[j]->channel);
				pProslic[j]->error = RC_CM_CAL_ERR;
				pProslic[j]->channelEnable = 0;
			}
		}
	}

	for (j=0;j<size;j++){
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 64, 0);/* this is a singular calibration bit for longitudinal calibration*/
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 97, 1);
		pProslic[j]->WriteRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 96, 0x40);
	}
	i=0;
	do{
		orData = 0;
		timeOut = (i++ > 8);/* (800) MS */
		for (j=0;j<size;j++){
#ifdef ENABLE_DEBUG
			if (pProslic[j]->debugMode){
				LOGPRINT ("si321x %d long balance cal executing\n",pProslic[j]->channel);
			}
#endif
			if (pProslic[j]->channelEnable){
				orData |= pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 96);
			}
		}
		pProslic[0]->DelayX(pProslic[0]->pProTimerX,100);
	} while((orData != 0) && (!timeOut));
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
		if (timeOut)
			LOGPRINT ("si321x %d lb cal timeout\n",pProslic[0]->channel);
	}
#endif
	for (j=0;j<size;j++){
		if (pProslic[j]->channelEnable){
			/* check for cm cal interrupt */
			if (pProslic[j]->ReadRegX(pProslic[j]->pProHWX, pProslic[j]->channel, 20) & 0x4){
				pProslic[j]->error = RC_CM_CAL_ERR;
				pProslic[j]->channelEnable = 0;
			}
		}
	}
#ifdef ENABLE_DEBUG
	if (pProslic[0]->debugMode){
			LOGPRINT ("si321x %d cal sequence finished\n",pProslic[0]->channel);
	}
#endif
	return 0;
}

/*
** Function: PROSLIC_EnableInterrupts
**
** Description: 
** Enables interrupts
*/
int Si321x_EnableInterrupts (proslicChanType_ptr pProslic){
	WriteReg(pProHW,pProslic->channel, INTRPT_MASK1, Si321x_General_Configuration.irqen1);
	WriteReg(pProHW,pProslic->channel, INTRPT_MASK2, Si321x_General_Configuration.irqen2);
	WriteReg(pProHW,pProslic->channel, INTRPT_MASK3, Si321x_General_Configuration.irqen3);
	return 0;
}

int Si321x_SetLoopbackMode (proslicChanType_ptr pProslic, ProslicLoopbackModes newMode){
	uInt8 regTemp;
	regTemp = ReadReg(pProHW,pProslic->channel, AUDIO_LOOPBACK);
	switch (newMode){
		case PROSLIC_LOOPBACK_NONE:
			WriteReg(pProHW,pProslic->channel, AUDIO_LOOPBACK, regTemp&~(0x6));
			break;
		case PROSLIC_LOOPBACK_DIG:
			WriteReg(pProHW,pProslic->channel, AUDIO_LOOPBACK, regTemp|(0x2));
			break;
		case PROSLIC_LOOPBACK_ANA:
			WriteReg(pProHW,pProslic->channel, AUDIO_LOOPBACK, regTemp|(0x4));
			break;
	}
	return 0;
}

int Si321x_SetMuteStatus (proslicChanType_ptr pProslic, ProslicMuteModes muteEn){
	uInt8 regTemp;
    uInt8 newRegValue;

	regTemp = ReadReg (pProHW,pProslic->channel,AUDIO_LOOPBACK);
	
	WriteReg (pProHW,pProslic->channel,AUDIO_LOOPBACK,regTemp&~(0x30));
    newRegValue = regTemp &~(0x30);
	
	if (muteEn & PROSLIC_MUTE_RX){
        newRegValue |= 1<<4;
	}
	if (muteEn & PROSLIC_MUTE_TX){
        newRegValue |= 1<<5;
	}

    if(newRegValue != regTemp)
    {
		WriteReg (pProHW,pProslic->channel,AUDIO_LOOPBACK, newRegValue);
    }
	return 0;
}

/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
*/

/*
** Function: PROSLIC_RingSetup
**
** Description: 
** configure ringing
*/
#ifdef DISABLE_RING_SETUP
#else
extern Si321x_Ring_Cfg Si321x_Ring_Presets [];
int Si321x_RingSetup (proslicChanType *pProslic, int preset){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode){
		/*LOGPRINT("si321x RingSetup %X %X %X %X %X %X %X %X %X\n",Si321x_Ring_Presets[preset].rngx,
			Si321x_Ring_Presets[preset].rngy,Si321x_Ring_Presets[preset].roff,Si321x_Ring_Presets[preset].rco,
			Si321x_Ring_Presets[preset].tahi,Si321x_Ring_Presets[preset].talo,Si321x_Ring_Presets[preset].tihi,
			Si321x_Ring_Presets[preset].tilo,Si321x_Ring_Presets[preset].ringcon);*/
	}
#endif
	WriteRAM(pProHW,pProslic->channel,RING_X,Si321x_Ring_Presets[preset].rngx);/* Ringing Oscillator Amplitude */
	WriteRAM(pProHW,pProslic->channel,RING_Y,Si321x_Ring_Presets[preset].rngy);/*Ringing Oscillator Initial Phase */
	WriteRAM(pProHW,pProslic->channel,RING_V_OFF,Si321x_Ring_Presets[preset].roff);/*Ringing Oscillator DC Offset */
	WriteRAM(pProHW,pProslic->channel,RING_OSC_COEF,Si321x_Ring_Presets[preset].rco); /* Ringing Oscillator Frequency */
	
	/* Active Timer */
	WriteReg(pProHW,pProslic->channel,RING_ON_HI,(Si321x_Ring_Presets[preset].tahi));
	WriteReg(pProHW,pProslic->channel,RING_ON__LO,(Si321x_Ring_Presets[preset].talo));

	/*  Inactive Timer */
	WriteReg(pProHW,pProslic->channel,RING_OFF_HI,(Si321x_Ring_Presets[preset].tihi));
	WriteReg(pProHW,pProslic->channel,RING_OFF_LO,(Si321x_Ring_Presets[preset].tilo));


	WriteReg(pProHW,pProslic->channel,RING_OSC_CTL,(Si321x_Ring_Presets[preset].ringcon));
	WriteReg(pProHW,pProslic->channel,RT_DEBOUCE,Si321x_Ring_Presets[preset].rtdi);
	WriteRAM(pProHW,pProslic->channel,RING_TRIP_FILTER,Si321x_Ring_Presets[preset].nrtp);
	return 0;
}
#endif
/*
** Function: PROSLIC_ToneGenSetup
**
** Description: 
** configure tone generators
*/
#ifdef DISABLE_TONE_SETUP
#else
#ifdef SI3210_TONE
extern Si321x_Tone_Cfg Si321x_Tone_Presets [];
#endif
#ifdef SI3215_TONE
extern Si321x_Tone_Cfg Si3215_Tone_Presets [];
#endif
#ifndef SI3210_TONE
#ifndef SI3215_TONE
#error Either SI3210 or SI3215 (or both) must be defined in proslic_api_config.h 
#endif
#endif
int Si321x_ToneGenSetup (proslicChanType *pProslic, int preset){
	Si321x_Tone_Cfg *pTone;
#ifdef SI3215_TONE
	pTone = Si3215_Tone_Presets;
#endif
#ifdef SI3210_TONE
	pTone = Si321x_Tone_Presets;
#endif
#ifdef SI3215_TONE
	if (pProslic->deviceId->chipType == SI3215 || pProslic->deviceId->chipType == SI3215M || pProslic->deviceId->chipType == SI3216 || pProslic->deviceId->chipType == SI3216M)
		pTone = Si3215_Tone_Presets;
#endif
	WriteRAM(pProHW,pProslic->channel,OSC1X,pTone[preset].osc1.amp);
	WriteRAM(pProHW,pProslic->channel,OSC1Y,pTone[preset].osc1.phas);
	WriteRAM(pProHW,pProslic->channel,OSC1_COEF,pTone[preset].osc1.freq);
	
	WriteReg(pProHW,pProslic->channel,OSC1_ON_HI,(pTone[preset].osc1.tahi));
	WriteReg(pProHW,pProslic->channel,OSC1_ON__LO,(pTone[preset].osc1.talo));
	WriteReg(pProHW,pProslic->channel,OSC1_OFF_HI,(pTone[preset].osc1.tihi));
	WriteReg(pProHW,pProslic->channel,OSC1_OFF_LO,(pTone[preset].osc1.tilo));
	
	WriteRAM(pProHW,pProslic->channel,OSC2X,pTone[preset].osc2.amp);
	WriteRAM(pProHW,pProslic->channel,OSC2Y,pTone[preset].osc2.phas);
	WriteRAM(pProHW,pProslic->channel,OSC2_COEF,pTone[preset].osc2.freq);
	
	WriteReg(pProHW,pProslic->channel,OSC2_ON_HI,(pTone[preset].osc2.tahi));
	WriteReg(pProHW,pProslic->channel,OSC2_ON__LO,(pTone[preset].osc2.talo));
	WriteReg(pProHW,pProslic->channel,OSC2_OFF_HI,(pTone[preset].osc2.tihi));
	WriteReg(pProHW,pProslic->channel,OSC2_OFF_LO,(pTone[preset].osc2.tilo));
	
	WriteReg(pProHW,pProslic->channel,OSC1,(pTone[preset].omode1));
	WriteReg(pProHW,pProslic->channel,OSC2,(pTone[preset].omode2));
	return 0;
}
#endif
/*
** Function: PROSLIC_FSKSetup
**
** Description: 
** configure fsk
*/
#ifdef DISABLE_FSK_SETUP
#else
extern Si321x_FSK_Cfg Si321x_FSK_Presets [];
int Si321x_FSKSetup (proslicChanType *pProslic, int preset){

	WriteRAM(pProHW, pProslic->channel, FSK_X_0, Si321x_FSK_Presets[preset].fsk0); /*fsk frequency 0 location */
	WriteRAM(pProHW, pProslic->channel, FSK_COEFF_0, Si321x_FSK_Presets[preset].fsk0x); /* fsk amplitude 0 location */

	WriteRAM(pProHW, pProslic->channel, FSK_X_1, Si321x_FSK_Presets[preset].fsk1); /* fsk frequency 0 location */
	WriteRAM(pProHW, pProslic->channel, FSK_COEFF_1, Si321x_FSK_Presets[preset].fsk1x); /* fsk amplitude 1 location */

	WriteRAM(pProHW, pProslic->channel, FSK_X_01,Si321x_FSK_Presets[preset].fsk01);
	WriteRAM(pProHW, pProslic->channel, FSK_X_10, Si321x_FSK_Presets[preset].fsk10);

	WriteReg(pProHW, pProslic->channel, 36, 19);

	WriteReg(pProHW, pProslic->channel, 37, 0);
	return 0;
}
#endif
/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
*/
int Si321x_DTMFDecodeSetup (proslicChanType *pProslic, int preset){
	return 0;/* not needed for si321x */
}

/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic
*/
int Si321x_SetProfile (proslicChanType *pProslic, Si321x_country_Cfg *pCountryData){
	return 0;
}

/*
** Function: PROSLIC_ZsynthSetup
**
** Description: 
** configure impedence synthesis
*/
#ifdef DISABLE_ZSYNTH_SETUP
#else
extern Si321x_Zsynth_Cfg Si321x_Zsynth_Presets[];
int Si321x_ZsynthSetup (proslicChanType *pProslic, int preset){
	
	WriteReg(pProHW, pProslic->channel, LINE_IMPEDANCE, Si321x_Zsynth_Presets[preset].tiss | (Si321x_Zsynth_Presets[preset].clc<<4));

	return 0;
}
#endif
/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
*/
int Si321x_GciCISetup (proslicChanType *pProslic, int preset){
	return 0;/* not needed for si321x */
}

/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
*/
int Si321x_ModemDetSetup (proslicChanType *pProslic, int preset){
	return 0;/* not needed for si321x */
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
#ifdef DISABLE_AUDIOGAIN_SETUP
#else
extern Si321x_audioGain_Cfg Si321x_AudioGain_Presets [];
int Si321x_TXAudioGainSetup (proslicChanType *pProslic, int preset){
	uInt8 data;
	data = ReadReg(pProHW, pProslic->channel, AUDIO_GAIN);
	data &= ~(0xC);
	WriteReg(pProHW, pProslic->channel, AUDIO_GAIN, data|(Si321x_AudioGain_Presets[preset].gain<<2));
	WriteRAM(pProHW, pProslic->channel, XMIT_DIGITAL_GAIN, Si321x_AudioGain_Presets[preset].digGain);	
        return RC_NONE;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si321x_RXAudioGainSetup (proslicChanType *pProslic, int preset){
	uInt8 data;
	data = ReadReg(pProHW, pProslic->channel, AUDIO_GAIN);
	data &= ~(0x3);
	WriteReg(pProHW, pProslic->channel, AUDIO_GAIN, data|Si321x_AudioGain_Presets[preset].gain); /* AUDIO_GAIN == 9 */
	WriteRAM(pProHW, pProslic->channel, RECV_DIGITAL_GAIN, Si321x_AudioGain_Presets[preset].digGain);
	return RC_NONE;
}
#endif
/*
** Function: PROSLIC_HybridSetup
**
** Description: 
** configure Proslic hybrid
*/
int Si321x_HybridSetup (proslicChanType *pProslic, int preset){
	return RC_NONE;/* not needed for si321x */
}

/*
** Function: PROSLIC_AudioEQSetup
**
** Description: 
** configure audio equalizers
*/
int Si321x_AudioEQSetup (proslicChanType *pProslic, int preset){
	return RC_NONE;/* not needed for si321x */
}

/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
*/
#ifdef DISABLE_DCFEED_SETUP
#else
extern Si321x_DCfeed_Cfg Si321x_Dcfeed_Presets [];
int Si321x_DCFeedSetup (proslicChanType *pProslic, int preset){
	WriteReg(pProHW, pProslic->channel, LOOP_I_LIMIT, (Si321x_Dcfeed_Presets [preset].ilim));
	WriteReg(pProHW, pProslic->channel, ON_HOOK_V, (Si321x_Dcfeed_Presets [preset].voc));
	WriteReg(pProHW, pProslic->channel, COMMON_V, (Si321x_Dcfeed_Presets [preset].vcm));
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
*/
int Si321x_GPIOSetup (proslicChanType *pProslic, int preset){
	return RC_NONE;/* not needed for si321x */
}

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
#ifdef DISABLE_PULSE_SETUP
#else
extern Si321x_PulseMeter_Cfg Si321x_PulseMeter_Presets [];
int Si321x_PulseMeterSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW, pProslic->channel, PULSE_Y, Si321x_PulseMeter_Presets [preset].plsco);/* pulse metering frequency */
	WriteRAM(pProHW, pProslic->channel, PULSE_X, Si321x_PulseMeter_Presets [preset].plsx);/* pulse metering amplitude */
	WriteRAM(pProHW, pProslic->channel, PULSE_ENVEL, Si321x_PulseMeter_Presets [preset].plsd);/* pulse metering ramp rate */
	
	WriteReg(pProHW, pProslic->channel, PULSE_ON__LO, (uInt8)((Si321x_PulseMeter_Presets [preset].pat)));/* pulse metering active timer */
	WriteReg(pProHW, pProslic->channel, PULSE_ON__LO+1, (uInt8)((Si321x_PulseMeter_Presets [preset].pat)>>8));/* pulse metering active timer */
	WriteReg(pProHW, pProslic->channel, PULSE_OFF_LO, (uInt8)(Si321x_PulseMeter_Presets [preset].pit));/* pulse metering inactive timer */
	WriteReg(pProHW, pProslic->channel, PULSE_OFF_LO+1, (uInt8)((Si321x_PulseMeter_Presets [preset].pit)>>8));/* pulse metering inactive timer */
	return RC_NONE;
}
#endif

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
#ifdef DISABLE_PCM_SETUP
#else
extern Si321x_PCM_Cfg Si321x_Pcm_Presets [];
int Si321x_PCMSetup (proslicChanType *pProslic, int preset){
	uInt8 data=0;
		
	
	data |= Si321x_Pcm_Presets [preset].pcmf<<2;
	data |= Si321x_Pcm_Presets [preset].wbe<<6;
	data |= Si321x_Pcm_Presets [preset].tri;
	WriteReg(pProHW, pProslic->channel, PCM_MODE, data);
	

	return RC_NONE;
}
#endif
int Si321x_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount){
	WriteReg(pProHW,pProslic->channel,PCM_XMIT_START_COUNT_LSB,txcount & 0xff);
	WriteReg(pProHW,pProslic->channel,PCM_XMIT_START_COUNT_MSB,txcount >> 8);

	WriteReg(pProHW,pProslic->channel,PCM_RCV_START_COUNT_LSB,rxcount & 0xff);
	WriteReg(pProHW,pProslic->channel,PCM_RCV_START_COUNT_MSB,rxcount >> 8);
	return RC_NONE;
}
/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/

/*
** Function: PROSLIC_FindInterruptChannels
**
** Description: 
** Finds channels needing serviced
** 
*/
int Si321x_FindInterruptChannels (proslicChanType_ptr *hProslic, uInt8 *pChannelNumbers){
	return RC_NONE;
}

/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Reads interrupt registers status (IRQ1-3)
*/
int Si321x_GetInterrupts (proslicChanType *pProslic,proslicIntType *pIntData){
	/* Multiple interrupts may occur at once so bear that in mind when
	   writing an interrupt handling routine */
	uInt8 data[3];
	uInt8 i,j;
	int k=0;
	pIntData->number = 0;
	data[0] = ReadReg(pProHW,pProslic->channel,INTRPT_STATUS1);
	data[1] = ReadReg(pProHW,pProslic->channel,INTRPT_STATUS2);
	data[2] = ReadReg(pProHW,pProslic->channel,INTRPT_STATUS3);
	WriteReg(pProHW,pProslic->channel,INTRPT_STATUS1,data[0]);/* clear the interrupts */
	WriteReg(pProHW,pProslic->channel,INTRPT_STATUS2,data[1]);
	WriteReg(pProHW,pProslic->channel,INTRPT_STATUS3,data[2]);

	for (i=0;i<3;i++){
		for (j=0;j<8;j++){
			if (data[i]&(1<<j)){
				switch (j + (i*8)){
					case OSC1_T1_SI321X:
						k=IRQ_OSC1_T1;
						break;
					case OSC1_T2_SI321X:
						k=IRQ_OSC1_T2;
						break;
					case OSC2_T1_SI321X:
						k=IRQ_OSC2_T1;
						break;
					case OSC2_T2_SI321X:
						k=IRQ_OSC2_T2;
						break;
					case RING_T1_SI321X:
						k=IRQ_RING_T1;
						break;
					case RING_T2_SI321X:
						k=IRQ_RING_T2;
						break;
					case PM_T1_SI321X:
						k=IRQ_PM_T1;
						break;
					case PM_T2_SI321X:
						k=IRQ_PM_T2;
						break;
					case RING_TRIP_SI321X:
						k=IRQ_RING_TRIP;
						break;
					case LOOP_STAT_SI321X:
						k=IRQ_LOOP_STATUS;
						break;
					case PQ1_SI321X:
						k=IRQ_PQ1;
						break;
					case PQ2_SI321X:
						k=IRQ_PQ2;
						break;
					case PQ3_SI321X:
						k=IRQ_PQ3;
						break;
					case PQ4_SI321X:
						k=IRQ_PQ4;
						break;
					case PQ5_SI321X:
						k=IRQ_PQ5;
						break;
					case PQ6_SI321X:
						k=IRQ_PQ6;
						break;
					case DTMF_SI321X:
						k=IRQ_DTMF;
						break;
					case INDIRECT_SI321X:
						k=IRQ_INDIRECT;
						break;
					case CM_BAL_SI321X:
						k=IRQ_CM_BAL;
						break;
					default:
						k=0xff;
				}
				pIntData->irqs[pIntData->number] = 	k;		
				pIntData->number++;
			
			}
		}	

	}

	return pIntData->number;
}

/*
** Function: PROSLIC_ReadHookStatus
**
** Description: 
** Determine hook status
*/
int Si321x_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat){
	if (ReadReg(pProHW,pProslic->channel,LOOP_STAT) & 1)
		*pHookStat=OFFHOOK;
	else
		*pHookStat=ONHOOK;
	return RC_NONE;
}

/*
** Function: PROSLIC_WriteLinefeed
**
** Description: 
** Sets linefeed state
*/
int Si321x_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed){
	WriteReg (pProHW, pProslic->channel, LINE_STATE,newLinefeed);
	return RC_NONE;
}

int Si321x_SetLinefeedStatusBroadcast (proslicChanType *pProslic,uInt8 newLinefeed){
	WriteReg (pProHW, SI321X_BROADCAST, LINE_STATE,newLinefeed);
	return RC_NONE;
}

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
*/
int Si321x_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState){
	if (newPolRevState)
		WriteReg (pProHW, pProslic->channel, LINE_STATE,5);
	else
		WriteReg (pProHW, pProslic->channel, LINE_STATE,1);
	return RC_NONE;
}

/*
** Function: PROSLIC_GPIOControl
**
** Description: 
** Sets gpio of the proslic
*/
int Si321x_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read){
	return RC_NONE; /* not needed for si321x */
}

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
*/
int Si321x_MWI (proslicChanType *pProslic,uInt8 lampOn){
	uInt8 data;
	do {
	    data = ReadReg(pProHW,pProslic->channel,82);
	}
	while (data < (600/15 - 3));
	data = 0x3F;
	WriteReg(pProHW,pProslic->channel,72,data);/*  High Neon */
	return RC_NONE;
}

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** start tone generators
*/
int Si321x_ToneGenStart (proslicChanType *pProslic, uInt8 timerEn){
	uInt8 data;
	Si321x_ToneGenStop(pProslic);
	
	data = ReadReg(pProHW,pProslic->channel,OSC1);
	data |= 0x4 + (timerEn ? 0x18 : 0);
	WriteReg(pProHW,pProslic->channel,OSC1,data);

	data = ReadReg(pProHW,pProslic->channel,OSC2);
	data |= 0x4 + (timerEn ? 0x18 : 0);
	WriteReg(pProHW,pProslic->channel,OSC2,data);
	
	return RC_NONE;

}


/*
** Function: PROSLIC_StopTone
**
** Description: 
** Stops tone generators
**
** Input Parameters: 
** pProslic: pointer to Proslic object
**
** Return:
** none
*/
int Si321x_ToneGenStop (proslicChanType *pProslic){
	uInt8 data;
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_ToneGenStop\n");
	data = ReadReg(pProHW,pProslic->channel,OSC1);
	data &= ~(0x1C);
	WriteReg(pProHW,pProslic->channel,OSC1,data);

	data = ReadReg(pProHW,pProslic->channel,OSC2);
	data &= ~(0x1C);
	WriteReg(pProHW,pProslic->channel,OSC2,data);
	return 0;
}


/*
** Function: PROSLIC_StartRing
**
** Description: 
** start ring generator
*/
int Si321x_RingStart (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_RingStart\n");
#endif
	Si321x_SetLinefeedStatus(pProslic,LF_RINGING);
	return 0;
}


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
*/
int Si321x_RingStop (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_RingStop\n");
#endif
	Si321x_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	return 0;
}

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
*/
int Si321x_EnableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_EnableCID\n");
#endif
	Si321x_ToneGenStop(pProslic);
	WriteReg(pProHW,pProslic->channel,FSK_DATA,1);  /* prime buffer */
	WriteReg(pProHW,pProslic->channel,FSK_DATA,1);
	data = ReadReg(pProHW,pProslic->channel,21); /* mask register */
	if ((data&1) == 0){
		data |= 1;
		WriteReg(pProHW,pProslic->channel,21,data); /* mask register */
	}
	WriteReg(pProHW,pProslic->channel,OSC1,0x56);

	return 0;	
}

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
*/
int Si321x_DisableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_DisableCID\n");
#endif
	data = 0xA0;
	WriteReg(pProHW,pProslic->channel,OSC1,data);
	return 0;
}

static void waitForInterrupt (proslicChanType *pProslic,uInt8 *init){
/* Wait for an Interrupt from the ProSLIC => oscillator loaded */
	uInt8 data;
	data = ReadReg(pProHW,pProslic->channel,18);

	 if (data && *init !=0){
		 *init = 2;
	 }
	 if (*init == 0) *init=1; /* init has 3 states 0 => fsk initialized
							                     1 => fsk did first interrupt
												 2 => got premature interrupt
							*/
	 while (!data) {
		data = ReadReg(pProHW,pProslic->channel,18);
	 }
	WriteReg(pProHW,pProslic->channel,18,data);  /*  Status Register #1  clear interrupt*/
}

static void fskByte(proslicChanType *pProslic,uInt8 c,uInt8 *init){
	int i;
	
	WriteReg(pProHW,pProslic->channel,FSK_DATA,0); 
	waitForInterrupt(pProslic,init);    /*  start bit  STARTS */
	for (i=0;i<8;i++){
		WriteReg(pProHW,pProslic->channel,FSK_DATA,c);
		c>>=1;
		waitForInterrupt(pProslic,init);
	} /* for */
	WriteReg(pProHW,pProslic->channel,FSK_DATA,1);
	waitForInterrupt(pProslic,init);
}/* fskByte() */

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
*/
int Si321x_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes){
	uInt8 data; uInt8 init=0;
	while (numBytes-- > 0){
		data = *(buffer++);
		fskByte(pProslic,data,&init);
	}
	return 0;
}

int Si321x_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *buffer){
	*buffer = 1; /* we have no buffer so always avail */
	return 0;
}

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
*/
int Si321x_PCMStart (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_PCMStart\n");
#endif
	data = ReadReg(pProHW,pProslic->channel,PCM_MODE);
	data |= 0x20;
	WriteReg(pProHW,pProslic->channel,PCM_MODE,data);

	
	return 0;
}


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
*/
int Si321x_PCMStop (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_PCMStop\n");
#endif
	data = ReadReg(pProHW,pProslic->channel,PCM_MODE);
	data &= ~(0x20);
	WriteReg(pProHW,pProslic->channel,PCM_MODE,data);
	return 0;
}


/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
*/
int Si321x_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit){
	*pDigit = ReadReg(pProHW,pProslic->channel,DTMF_DIGIT);
	*pDigit = *pDigit & 0xf;
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
*/
int Si321x_PLLFreeRunStart (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si321x_PLLFreeRunStart\n");
#endif
	
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
*/
int Si321x_PLLFreeRunStop (proslicChanType *pProslic){

	return 0;
}

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
*/
int Si321x_PulseMeterStart (proslicChanType *pProslic){

	WriteReg(pProHW,pProslic->channel,PULSE_OSC,0xff); /* enable timers and oscillator */

	return 0;	
}

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
*/
int Si321x_PulseMeterStop (proslicChanType *pProslic){
	
	WriteReg(pProHW,pProslic->channel,PULSE_OSC,0); /* diasbles timers and oscillator */

	return 0;
}

/*
** Function: Si321x_dbgSetGain
**
** Description: 
** Provisionary function for setting up
** gains - NOTE: use of 2nd value is used for Analog gain control adjustment.
*/
int Si321x_dbgSetGain(proslicChanType *pProslic, int32 gain, int ag_value, int audio_gain_preset){
    int32 i;
    ProSLIC_GainScaleLookup *gain_table = NULL;

    switch(ag_value)
    {
        case SI321X_AG_0DB:
            gain_table = (ProSLIC_GainScaleLookup *)gainScaleTable_0dB;
            break;

        case SI321X_AG_POS35DB:
            gain -= 7; /* The value needed for 9 dB (+3.5) is the same as 2 dB (-3.5) */
            /* Fall through */
        case SI321X_AG_NEG35DB:
            gain_table = (ProSLIC_GainScaleLookup *)gainScaleTable_35dB;
            break;

        default:
            return RC_UNSUPPORTED_FEATURE;
    }

    if(gain < gain_table[0].gain )
    {
        return RC_GAIN_OUT_OF_RANGE;
    }

    for(i = 0; gain_table[i].gain != 0xFF; i++)
    {
        if( gain == gain_table[i].gain )
        {
            break;
        }
    }

    if( gain_table[i].gain == 0xFF )
    {
        return RC_GAIN_OUT_OF_RANGE;
    }            

    Si321x_AudioGain_Presets[audio_gain_preset].gain = ag_value;
    Si321x_AudioGain_Presets[audio_gain_preset].digGain = gain_table[i].scale;

    return RC_NONE;
}

/*
** Function: Si321X_dbgSetDCFeedVopen
**
** Description: 
** Provisionary function for setting up the DC Open Circuit voltage levels.
** 
*/
int Si321x_dbgSetDCFeedVopen(proslicChanType *pProslic, uInt32 v_vlim_val,int32 preset)
{
    int temp_value = KSENSE_MUL_INV((v_vlim_val<<1))/3; /* 1 Unit = 1.5 V */

    Si321x_Dcfeed_Presets[preset].voc = temp_value & 0x3F;
    return(Si321x_DCFeedSetup(pProslic, preset));
}

/*
** Function: Si321X_dbgSetDCFeedIloop
**
** Description: 
** Provisionary function for setting up the loop current w/ boundary checking.
*/
int Si321x_dbgSetDCFeedIloop(proslicChanType *pProslic, uInt32 current,int32 preset)
{
    if(current < SI321X_MIN_LOOP_CURRENT)
    {
        current = SI321X_MIN_LOOP_CURRENT;
    }

    if(current > SI321X_MAX_LOOP_CURRENT)
    {
        current = SI321X_MAX_LOOP_CURRENT;
    }

    Si321x_Dcfeed_Presets[preset].ilim= (current - SI321X_MIN_LOOP_CURRENT)/3; /* 0 = MIN LC, 3 mA steps */

    return(Si321x_DCFeedSetup(pProslic, preset));
}

/*
** Function: Si321x_AudioGainSetup
**
** Description: 
** Set audio gain of RX and TX paths  
** NOTE: preset is not used at this time.
**
*/

int Si321x_AudioGainSetup(proslicChanType *pProslic, int rxgain, int txgain, int preset)
{
    int agrx_adjust;
    int agtx_adjust;
   
    if(rxgain > 6)
    {
        agrx_adjust = SI321X_AG_POS35DB;
    }
    else
    {
        if(rxgain < gainScaleTable_0dB[0].gain)
        {
            agrx_adjust = SI321X_AG_NEG35DB;
        }
        else
        {
            agrx_adjust = SI321X_AG_0DB;
        }
    }

    if(txgain > 2)
    {
        agtx_adjust = SI321X_AG_0DB;

        if(txgain > 6)
        {
            agtx_adjust = SI321X_AG_POS35DB;
        }
    }
    else
    {
        agtx_adjust = SI321X_AG_NEG35DB;
    }

    Si321x_dbgSetGain(pProslic,rxgain,agrx_adjust,0);
    Si321x_RXAudioGainSetup(pProslic,0);
    Si321x_dbgSetGain(pProslic,txgain,agtx_adjust,1);
    Si321x_TXAudioGainSetup(pProslic,1);
   
    return RC_NONE;
}

