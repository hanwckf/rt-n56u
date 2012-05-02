/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: pstn.c,v 1.1 2008-12-31 02:34:53 qwert Exp $
**
** pstn.c
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
** This is the example implementation for checking whether the
** ProSLIC is connect to a live PSTN line.
**
**
*/
#include "proslic_datatypes.h"
#include "proslic_ctrl.h"
#include "proslic_timer_intf.h"
#include "ProSLIC.h"
#include "Si321x_Intf.h"
#include "Si321x_Registers.h"
#include "Si321x_GR909.h"

#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM	pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr

static int32 abs (int32 a){
	if (a<0)
		return -1*a;
	return a;
}

//BELOW THIS POINT ARE FUNCTIONS FOR PSTN PRESENCE CHECK
static uInt8 PSTNOnHook(ProSLICGR909Type *pProSLIC909){
	/* This function is used to detect the presence of another phone line plugged into the ProSLIC (instead of a phone).
	*  This function is used in the case where the ProSLIC detects an on-hook condition during the test.
	*/
	uInt8 iq5,iq6;
	uInt8 data, Linefeed;
	int32 i = 0;

	iq5 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5); 
	iq6 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6); 

	if ((iq5 > 3) || (iq6 > 3)) //check IQ5 and IQ6 for excess current
			return 1; //line detected
	
	Linefeed = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64); 
	Linefeed = 0x0f&Linefeed;
	if(Linefeed == 1)
		data = 0x1c;
	else // == 5
		data = 0x5c; //polarity change

	//we passed - now try changing voc to create some current and make sure
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, data); //set VOC lower (42V)
	pProSLIC909->Delay(pProSLIC909->pProTimer, 1000);

	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64);
	if(data == 0){ //power alarm
		return 1; //fail
	}	


	iq5 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5); 
	iq6 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6); 

	if ((iq5 > 3) || (iq6 > 3)) //check IQ5 and IQ6 for excess current
			return 1; //PSTN line detected

	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72,0x20); //set back to VOC = 48V 

	return 0; //no PSTN line detected
}

static uInt8 PSTNOffHook (ProSLICGR909Type *pProSLIC909){
	/* This function is used to detect the presence of another phone line plugged into the ProSLIC (instead of a phone).
	*  This function is used in the case where the ProSLIC detects an off-hook condition during the test.
	*/
	uInt8 iq5, iq6;
	uInt8 data, data2;
	int32 vloop;
	uInt8 temp;

	//we detected offhook 
	//first check for high voltage on tip and ring
	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_V_SENSE); 
	data2 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, TIP_V_SENSE); 
    vloop = abs(data-data2);
    if ( (vloop*100) > 6648) // 2500000/376 = 6648 ==> 25.0/0.376
		return 1; //pstn line detected, voltage too high

    //now move Vcm around for the case where PSTN is not pulling voltages high
    //external line will pull voltage and cause currents to change
	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5); 
	data2 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6);
	
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73, 6); //change VCM
	pProSLIC909->Delay(pProSLIC909->pProTimer, 2000);
	
	temp = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64);
	if(temp == 0) //power alarm
		return 1; //fail
		

	iq5 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5); 
	iq6 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6); 
    if ((abs(data - iq5) > 3) || (abs(data2 - iq6) > 3)){ //checking for current change
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73, 2); //restore Vcm
		return 1; //fail
    }

	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73, 2); //restore Vcm
    return 0; //no PSTN line detected
}

static uInt8 PSTNLineCheck (ProSLICGR909Type *pProSLIC909){
	/*This function is used to detect a foreign phone line plugged into the ProSLIC*/
	uInt8 data;
			
	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LOOP_STAT); 
	data = data&1;

	if (!data) //we are on-hook
		return(PSTNOnHook(pProSLIC909));
	else //we are off-hook
		return(PSTNOffHook(pProSLIC909));
	
}




static int32 measDcTipRing(proslicChanType *pProslic, int32 samples, int32 *vTip, int32 *vRing){
	/*This function samples tip and ring every 8ms and returns the average*/
	uInt8 i;
	int32 sumVtip = 0, sumVring= 0, vScale = 376; //(equiv to 0.376)
	uInt8 data;

	for(i=0;i<samples;i++){
		data = ReadReg(pProHW, pProslic->channel, 80);
		sumVtip += vScale*((int32)(data));
		data = ReadReg(pProHW, pProslic->channel, 81);
		sumVring += vScale*((int32)(0x00ff&data));
		Delay(pProTimer, 8);
	}

	*vTip = sumVtip/samples;
	*vRing = sumVring/samples;
	return 0;
}



int32 ProSLIC_GR909_PSTN_FAST(ProSLICGR909Type *pProSLIC909){
	/*This function tries to detect presence of foreign phone line connected to the ProSLIC. It does so
	* without being as thorough or discreet as the other test but it is much faster.
	*/
	
	int32 time = 0, millisecs = 0;
	uInt8 data;
	int32 vTip, vRing;
	uInt8 voc, vcm, Linefeed;

	if(pProSLIC909->pProslic->debugMode)
		LOGPRINT("\n************** Invasive Test: **************\n");
	vcm = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel,73); //store vcm
	voc = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72); //store voc
	Linefeed = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel,64); //read & store Linefeed
	data = 0x0f&Linefeed;
	if(!(data == 1 || data == 5))
			ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 1);
	
	//check for power alarm
	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64); //read Linefeed
	if(data == 0) //power alarm
		return 1;

	//Linefeed will be set to 1 at this point--->v. small decay time
	
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73, 0); //Vcm = 0v
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, 0); //Voc = 0v
	ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 0); //Restore Linefeed
	pProSLIC909->Delay(pProSLIC909->pProTimer, 80);
	measDcTipRing(pProSLIC909->pProslic, 16, &vTip, &vRing);
	
	if(pProSLIC909->pProslic->debugMode){
		LOGPRINT("vTip = %d\n",vTip);
		LOGPRINT("vRing = %d\n",vRing);
	}

	if(vTip > 3 || vRing > 3)
		return 1; //Line detected

	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73,vcm); //Restore Vcm
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72,voc); //Restore Voc
	ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, Linefeed); //Restore Linefeed
	return 0;
}

int32 ProSLIC_PSTN(ProSLICGR909Type *pProSLIC909){
	/*This function tries to detect presence of foreign phone line connected to the ProSLIC. 
	*/
	uInt8 data;

	pProSLIC909->Delay(pProSLIC909->pProTimer, 100);
	data = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64); //read Linefeed

	data = 0x0f&data;

	if(data == 0){ //if we are already open just do quick test
		if(ProSLIC_GR909_PSTN_FAST(pProSLIC909))
			return 0;
		else
			return 1;
	}
	else if(data == 1|| data == 5){
		if (PSTNLineCheck(pProSLIC909))//you should check again in 10sec just in case we were in transient state
			return 0;
		else
			return 1;
	}
	//if linefeed not equal to 0, 1, or 5 --> exit;
	return 2;
}

//$Log: pstn.c,v $
//Revision 1.1  2008-12-31 02:34:53  qwert
//Add si3210 driver
//
//Revision 1.7  2007/06/05 15:40:04  lajordan
//no message
//
//Revision 1.6  2007/04/03 20:26:19  lajordan
//improved ren test
//
//Revision 1.5  2007/04/02 19:49:31  lajordan
//no message
//
//Revision 1.4  2007/02/16 00:34:54  lajordan
//no message
//
//Revision 1.3  2007/02/05 23:03:48  lajordan
//register read/ram read functions def changed
//
//Revision 1.2  2007/02/01 04:09:31  lajordan
//moved pstn functions out of gr909.c
//
//Revision 1.1  2007/02/01 04:08:38  lajordan
//updated to polling
//