/*
** Copyright © 2007 by Silicon Laboratories
**
** $Id: si321x_gr909.c,v 1.1 2008-12-31 02:34:53 qwert Exp $
**
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
** Description: This is the Si321x GR909 implementation 
**
**
*/
#include "proslic_datatypes.h"
#include "si321x_intf.h"
#include "si321x_registers.h"
#include "si321x_gr909.h"

#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM	pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr


#define SI321X_LT_REN_RINGOSC 0x7EFD //20Hz
int32 SI321X_LT_REN_SLOPE = 451;
int32 SI321X_LT_REN_OFF =  43;
uInt16 SI321X_LT_REN_RINGX = 0x0081;
uInt16 SI321X_LT_REN_RINGOF = 0x1aaa;
uInt16 SI321X_LT_REN_VCHARGE = 0x1e; //46V
uInt16 SI321X_LT_REN_VSTOP = 0x1c; 
uInt16 SI321x_LT_REN_VOC = 0xc;
uInt16 SI321X_LT_ROH_V1 = 0x6; //6V
uInt16 SI321X_LT_ROH_V2 = 0xC ;//18V
int32 SI321X_LT_VOLT_SCALE = 376 ;/* in mV, for HV, set to 639*/
int32 SI321X_LT_RESF_RST = 20; // DC Sense Resistor on TIP 
int32 SI321X_LT_RESF_RSR =	20;	// DC Sense Resistor on RING 
int32 SI321X_LT_RESF_RSH =	68; // External shunt resistor
uInt16 SI321X_LT_RESF_VCM1 = 0x6; //7
uInt16 SI321X_LT_RESF_VOC1 = 0x1a; //0xC
uInt16 SI321X_LT_RESF_VOC2 = 0x1f; 
uInt16 SI321X_LT_RESF_NORSH_VOCSTOP = 40; /* 60V */
uInt16 SI321X_LT_RESF_TG_FAULT = 9000; /* in mV */
uInt16 SI321X_LT_RESF_TR_FAULT = 20000; /* in mV */
int32 SI321X_LT_RESF_ADJ = 0;
uInt8 SI321X_LT_RESF_INIT_VCM_LOWRESF = 0x20;

int32 ProSLIC_GR909_INIT(ProSLICGR909Type *pProSLIC909, SI321x_GR909_Config *config){
	if (config->bomOption == SI321X_LV_BOM){
		 SI321X_LT_REN_RINGX = 0x0081;
		 SI321X_LT_REN_RINGOF = 0x1aaa;
		 SI321X_LT_REN_VCHARGE = 0x1e; //45V
		 SI321X_LT_REN_VSTOP = 60; //22V
		 SI321x_LT_REN_VOC = 0xc;//18V
		 SI321X_LT_ROH_V1 = 0x6; //9V
		 SI321X_LT_ROH_V2 = 0xC ;//18V
		 SI321X_LT_VOLT_SCALE = 376 ;
		 SI321X_LT_RESF_RST = 20; // DC Sense Resistor on TIP 
         SI321X_LT_RESF_RSR = 20;	// DC Sense Resistor on RING
		 SI321X_LT_RESF_VCM1 = 0x6; 
		 SI321X_LT_RESF_VOC1 = 0x1a; 
		 SI321X_LT_RESF_VOC2 = 0x1f;
		 SI321X_LT_RESF_NORSH_VOCSTOP = 40;
		 SI321X_LT_RESF_TG_FAULT = 9000; /* in mV */
		 SI321X_LT_RESF_TR_FAULT = 20000;
		 SI321X_LT_RESF_ADJ = 0;
		 SI321X_LT_RESF_INIT_VCM_LOWRESF = 0x20;
	}
	else if (config->bomOption == SI321X_HV_BOM){
		 SI321X_LT_REN_RINGX = 0x004c;
		 SI321X_LT_REN_RINGOF = 0xFAF;
		 SI321X_LT_REN_VCHARGE = 0x12; //46V
		 SI321X_LT_REN_VSTOP = 35; //22V 
		 SI321x_LT_REN_VOC = 0x7; //18V
		 SI321X_LT_ROH_V1 = 0x4; //9V
		 SI321X_LT_ROH_V2 = 0x7 ;//18V
		 SI321X_LT_VOLT_SCALE = 639;
		 SI321X_LT_RESF_RST = 34; // DC Sense Resistor on TIP 
         SI321X_LT_RESF_RSR = 34;	// DC Sense Resistor on RING 
		 SI321X_LT_RESF_VCM1 = 4;
		 SI321X_LT_RESF_VOC1 = 0xC;
		 SI321X_LT_RESF_VOC2 = 0x15; //use this value to get good accuracy around 150k
		 SI321X_LT_RESF_NORSH_VOCSTOP = 24; //60V
		 SI321X_LT_RESF_TG_FAULT = 14000; // in mV - 402k
		 SI321X_LT_RESF_TR_FAULT = 24000; //402k
		 SI321X_LT_RESF_ADJ = 1700; //voltage offset induced by changing sense resistor
		 SI321X_LT_RESF_INIT_VCM_LOWRESF = 0x13;
	}
	SI321X_LT_REN_SLOPE = 108;
    SI321X_LT_REN_OFF =  57;
    SI321X_LT_RESF_RSH = config->rshValue; // External shunt resistor
	return 0;
}

static int32 LowResFault(ProSLICGR909Type *pProSLIC909, SI321x_LOWRMEAS_STATE *pState);

/*delay count function*/
static void delay_poll (si321x_test_state *pState,unsigned short delayCount){
	pState->waitIterations++;	
	if (pState->waitIterations == delayCount){
		pState->waitIterations=0;
		pState->stage++;
	}	
}
/* 
 * Preserve current register settings prior to the test.
 */
static void si321x_preserve_state(ProSLICGR909Type *pProSLIC909,si321x_test_state *pState)
{
	pState->old_settings.ring_osc_coef = (uInt16)pProSLIC909->ReadRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_OSC_COEF);
	pState->old_settings.ring_x = (uInt16)pProSLIC909->ReadRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_X);
	pState->old_settings.ring_y = (uInt16)pProSLIC909->ReadRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_Y);
	pState->old_settings.loop_close_thr = (uInt16)pProSLIC909->ReadRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,LOOP_CLOSE_THRESH);
	pState->old_settings.ring_osc_ctl= pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_OSC_CTL);
	pState->old_settings.common_v = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,COMMON_V);
	pState->old_settings.onhookv = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,ON_HOOK_V);
	pState->old_settings.audio_gain = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,AUDIO_GAIN);
	pState->old_settings.loop_i_limit = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,LOOP_I_LIMIT);
	pState->old_settings.int0	= pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK1);
	pState->old_settings.int1	= pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK2);
	pState->old_settings.int2	= pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,INTRPT_MASK3);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,64,0);//go open
	pState->old_settings.reg65	= pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,65);

	/* Disable interrupts in general to allow for tests to run without issues */
	
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK1, 0);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK2, 0);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK3, 0);
}
/* 
 * restore register settings to what they were prior to the tests.
 */
static void si321x_restore_state(ProSLICGR909Type *pProSLIC909,si321x_test_state *pState)
{
	
	pProSLIC909->WriteRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_OSC_COEF,pState->old_settings.ring_osc_coef);
	pProSLIC909->WriteRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_X,pState->old_settings.ring_x);
	pProSLIC909->WriteRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,RING_Y,pState->old_settings.ring_y);
	
	pProSLIC909->WriteRAM(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, LOOP_CLOSE_THRESH, pState->old_settings.loop_close_thr);
	
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, RING_OSC_CTL,pState->old_settings.ring_osc_ctl);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, COMMON_V, pState->old_settings.common_v);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, ON_HOOK_V, pState->old_settings.onhookv);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, LOOP_I_LIMIT, pState->old_settings.loop_i_limit);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, AUDIO_GAIN, pState->old_settings.audio_gain);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel,64,0);//go open
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, 65, pState->old_settings.reg65);

	/* Clear any interrupts pending */
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_STATUS1, 0xff);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_STATUS2, 0xff);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_STATUS3, 0xff);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK1, pState->old_settings.int0);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK2, pState->old_settings.int1);
	pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, INTRPT_MASK3, pState->old_settings.int2);

}

/* Abort a test in progress - restore prior register settings and delay to
 * ensure proper operation.
*/
int32	ProSLIC_GR909_ABORT(ProSLICGR909Type *pProSLIC909,si321x_test_state *pState)
{
	switch (pState->stage)
	{
		case 0:
			si321x_restore_state(pProSLIC909,pState);
			delay_poll(pState,1000/POLL_RATE -2 );
			return 0;
		case 1:
			return 1;
	}
	return 1;
}
/*Foreign/Hazordous voltage test*/
int32 ProSLIC_GR909_FOREIGN_VOLTAGES(ProSLICGR909Type *pProSLIC909,SI321x_VOLTAGES_STATE *pState)
{
	uInt8 i;
	int32 sumVtip = 0, sumVring = 0, sumVloop = 0;
	int32 tmp;
	uInt8 reg;
	// ----------------------------------------------------------------
	// Values in registers 80 and 81 will be updated at an 800Hz rate
	// Read these registers at rate lower than this to avoid redundancy
	// ----------------------------------------------------------------
	switch (pState->State.stage){
		case 0:

		si321x_preserve_state(pProSLIC909,&(pState->State)); /* Save off current register settings */
		/* NOTE: No register changes made, so we don't need to preserve any
 		 * registers (other than line state)
	     */
		if(pState->samples > SI321X_MAX_FEMF_SAMPLES) pState->samples = SI321X_MAX_FEMF_SAMPLES;  // Truncate
		pState->State.waitIterations=0;
		pState->State.sampleIterations=0;
		pState->State.stage++;
		pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, ON_HOOK_V, 0);//set voc=0
		pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, COMMON_V, 0); //vcm =0
		pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, LINE_STATE, 1);
#ifdef DEBUG
		LOGPRINT("Discharging REN\n");
#endif
		return 0;
		case 1:
		delay_poll(&(pState->State),250/(POLL_RATE)); /* Allow for decay */
		return 0;
		case 2:
		pProSLIC909->WriteReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, LINE_STATE, 0);
		pState->State.stage++;
#ifdef DEBUG
		LOGPRINT("Settling\n");
#endif
		return 0;
		case 3:
		delay_poll(&(pState->State),250/(POLL_RATE)); /* Allow for charge */
		return 0;
		case 4:
		reg = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, TIP_V_SENSE);
		pState->vtip[pState->State.sampleIterations] = (SI321X_LT_VOLT_SCALE * (int32)reg) + ((reg==0) ? 0 : SI321X_LT_RESF_ADJ);
		reg = pProSLIC909->ReadReg(pProSLIC909->pProHW,pProSLIC909->pProslic->channel, RING_V_SENSE);
		pState->vring[pState->State.sampleIterations] = (SI321X_LT_VOLT_SCALE * (int32)reg) + ((reg==0) ? 0 : SI321X_LT_RESF_ADJ);
		pState->vloop[pState->State.sampleIterations] = pState->vring[pState->State.sampleIterations] - pState->vtip[pState->State.sampleIterations];
#ifdef DEBUG
		LOGPRINT(/*0,*/"FEMF: iteration: %d tip: %d ring: %d loop: %d\n",
			pState->State.sampleIterations,
			pState->vtip[pState->State.sampleIterations],
			pState->vring[pState->State.sampleIterations],
			pState->vloop[pState->State.sampleIterations]);
#endif
		pState->State.sampleIterations++;
		if (pState->State.sampleIterations == pState->samples)
			pState->State.stage++;
		return 0;
		}
		
		for(i=0;i<pState->samples;i++)
		{
			
			sumVtip += pState->vtip[i];
			sumVring += pState->vring[i];
			sumVloop += pState->vloop[i];
			
		}

		/**********************************/
		/* Calculate average (DC) voltage */
		/**********************************/
		pProSLIC909->tgDC = sumVtip/pState->samples;
		pProSLIC909->rgDC = sumVring/pState->samples;
		pProSLIC909->trDC = sumVloop/pState->samples;
		/******************************/
		/* Calculate RMS (AC) voltage */
		/******************************/
		sumVtip = 0;
		sumVring = 0;
		sumVloop = 0;
		for(i=0;i<pState->samples;i++)
		{
			tmp = (pState->vtip[i]/10 - pProSLIC909->tgDC/10);
			sumVtip += ((tmp * tmp)/1000);
			tmp = pState->vring[i]/10 - pProSLIC909->rgDC/10;
			sumVring += ((tmp * tmp)/1000);
			tmp = pState->vloop[i]/10 - pProSLIC909->trDC/10;
			sumVloop += ((tmp * tmp)/1000);
		}


		pProSLIC909->tgAC = (sumVtip/pState->samples*10);
		pProSLIC909->rgAC = (sumVring/pState->samples*10);
		pProSLIC909->trAC = (sumVloop/pState->samples*10);
	    si321x_restore_state(pProSLIC909,&(pState->State)); /* restore prior register settings */
		return 1;
	
}





int32 ProSLIC_GR909_HAZARD_VOLTAGES(ProSLICGR909Type *pProSLIC909,SI321x_VOLTAGES_STATE *pState){
	return ProSLIC_GR909_FOREIGN_VOLTAGES(pProSLIC909,pState); //same test, thresholds change
}

// -------------------------------------
// measDcTipRing()
//
// DC voltage measurement with
// averaging. Returns values in mV.
//
// -------------------------------------
static int measDcTipRing_fixed(ProSLICGR909Type *pProSLIC909,si321x_test_state *pState, uInt16 samples, int32 *vTip, int32 *vRing)
{
	uInt8 reg;
	if (pState->sampleIterations > samples)
		return 1;
	reg = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, TIP_V_SENSE);
   *vTip += reg;
   reg = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_V_SENSE);
   *vRing += 0x00ff&reg;

   if (pState->sampleIterations == samples){
	   *vTip = (SI321X_LT_VOLT_SCALE * (*vTip))/samples;
	   *vRing = (SI321X_LT_VOLT_SCALE * (*vRing))/samples;
	   return 1;
   }

   
   return 0;
}
// --------------------------------
// measIQ1256()
//
// Linefeed transistor current 
// measurement with averaging.
//
// --------------------------------
static int measIQ1256_fixed(ProSLICGR909Type *pProSLIC909,si321x_test_state * pState, uInt16 samples, int32 *iq1, int32 *iq2, int32 *iq5, int32 *iq6)
{
		
	if (pState->sampleIterations > samples)
		return 1;
	   *iq1 += pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ1);	  
	   *iq2 += pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ2);	
	   *iq5 += pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5);	
	   *iq6 += pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6);	

#if (DEBUG)
  		LOGPRINT(/*0,*/"SI321X: IQ1256: iq5 = %u iq6 = %u \n", pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5),pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6));
#endif
   if (pState->sampleIterations == samples){
		*iq1 = ((SI321X_LT_IQ12_SCALE * (*iq1))/samples);
		*iq2 = ((SI321X_LT_IQ12_SCALE * (*iq2))/samples);
		*iq5 = ((SI321X_LT_IQ56_SCALE * (*iq5))/samples);
		*iq6 = ((SI321X_LT_IQ56_SCALE * (*iq6))/samples);
   }
   return 1;
}

// -----------------------------------------
// Roh
//
// Measure Rtr at two open loop voltages
// and compare to determine if diode bridge
// present.
//
// -----------------------------------------

int32 ProSLIC_GR909_RECEIVER_OFFHOOK(ProSLICGR909Type *pProSLIC909,SI321x_ROH_STATE *pState){

	int32 tmp;
	long zDiff;
	long Rmin = 15000;       // Worst-case phone impedance + loop
	long Rmax = 1500000;     // Max resistance considered a fault
	
	switch (pState->State.stage){
	
	case 0:
    // Set open loop voltage to 9v and measure current
		si321x_preserve_state(pProSLIC909,&(pState->State)); /* Save off current register settings */
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, 0);//voc=0
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, 0);//vcm=0
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 1);
		pState->State.stage++;
		pState->State.waitIterations=0;
		pState->State.sampleIterations=0;
		return 0;
	case 1:
		delay_poll(&pState->State,1000/POLL_RATE - 2);                // Bleed line
		return 0;
	case 2:
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
    	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LOOP_I_LIMIT, 0x7);// 41mA ILIM
		pProSLIC909->WriteRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LOOP_CLOSE_THRESH, 0x4000);// LCRT 80mA (keep onhook)
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_ROH_V1);//voc=9v
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0x1);      // Active state 
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 65, 0x40);//filter out noise on iq5/6
		pState->State.stage++;
		return 0;
	case 3:
		delay_poll(&pState->State,2000/POLL_RATE - 2);                 	// Current settle in 
                                										// presence of large REN load
		pState->State.sampleIterations=0; pState->vRing=0; pState->vTip=0; pState->IQ1val=0;pState->IQ2val=0;pState->IQ5val=0;pState->IQ6val=0;
		return 0;
	case 4:
		pState->State.sampleIterations++;
		tmp = measDcTipRing_fixed(pProSLIC909,&pState->State, SI321X_LT_ROH_SAMPLES, &pState->vTip, &pState->vRing);
		if (measIQ1256_fixed(pProSLIC909,&pState->State, SI321X_LT_ROH_SAMPLES, &pState->IQ1val, &pState->IQ2val, &pState->IQ5val, &pState->IQ6val) && tmp){
			pState->State.stage++;
			if (pState->vRing != 0)
				pState->vRing += SI321X_LT_RESF_ADJ;
			if(pState->IQ5val > (3*SI321X_LT_IQ56_SCALE)) 
				pState->Rv1 = ((pState->vRing)*10000)/pState->IQ5val;
			else             
				pState->Rv1 = SI321x_ROH_MAXRES;
#if (DEBUG)
			LOGPRINT(/*0,*/"ROH: IQ5val = %u Vring = %u\n",pState->IQ5val,pState->vRing);
#endif
			
			// Set open loop voltage to 18v and measure current
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_ROH_V2);
		}
		
	return 0;
	case 5:
    delay_poll(&pState->State,1000/POLL_RATE - 2); 
	pState->State.sampleIterations=0; pState->vRing=0; pState->vTip=0; pState->IQ1val=0;pState->IQ2val=0;pState->IQ5val=0;pState->IQ6val=0;
    return 0;
	case 6:
	pState->State.sampleIterations++;
	tmp = measDcTipRing_fixed(pProSLIC909,&pState->State, SI321X_LT_ROH_SAMPLES, &pState->vTip, &pState->vRing);
	if (measIQ1256_fixed(pProSLIC909,&pState->State,SI321X_LT_ROH_SAMPLES, &pState->IQ1val, &pState->IQ2val, &pState->IQ5val, &pState->IQ6val) && tmp){

			pState->State.stage++;
			if (pState->vRing != 0)
				pState->vRing += SI321X_LT_RESF_ADJ;
#if (DEBUG)
			LOGPRINT(/*0,*/"ROH: IQ5val = %u Vring = %u\n",pState->IQ5val,pState->vRing);
#endif
			if(pState->IQ5val > (3*SI321X_LT_IQ56_SCALE)) 
				pState->Rv2 = ((pState->vRing)*10000)/pState->IQ5val;
			else             
				pState->Rv2 = SI321x_ROH_MAXRES;

			if (pState->Rv1 != 0)
				zDiff = ((100*(pState->Rv1-pState->Rv2))/pState->Rv1); /* Multiplied by 100 to give us % */
			else if (pState->Rv1 == pState->Rv2)
				zDiff = 0;
			else //rv1 is zero...make it 1 so not dividing by zero
				zDiff = ((100*(pState->Rv1-pState->Rv2))); /* Multiplied by 100 to give us % */


			if (zDiff<0) zDiff*=-1;

			//pState->Rv1= 	pState->Rv1; /* Change scale from mOhm, to 1/10 ohm */
			//pState->Rv2 =	pState->Rv2;
#if (DEBUG)
			LOGPRINT(/*0,*/"ROH: rv1 = %ld rv2 = %ld iq5val = %d channel: %u zdiff: %lu\n",pState->Rv1,pState->Rv2,pState->IQ5val,pState->State.channelNumber,zDiff);
#endif			
			if((zDiff > SI321X_LT_ROH_PERCENT_DIFF)&&(pState->Rv1<Rmin)&&(pState->Rv2<Rmin)) 
				 pProSLIC909->rohTrue =  SI321x_FAIL_ROH_DETECTED;
			else
			 {
				if((pState->Rv1<Rmax)||(pState->Rv2<Rmax))
					pProSLIC909->rohTrue =  SI321x_FAIL_RESFAULT_INC;
				else
					 pProSLIC909->rohTrue =  0;
			 }
#if (DEBUG)
			LOGPRINT(/*0,*/"ROH Result = %u on channel: %u\n",pProSLIC909->rohTrue,pState->State.channelNumber);
#endif
			si321x_restore_state(pProSLIC909,&(pState->State)); /* restore prior register settings */
			return 1;
		}
	return 0;
	}
	return 1;
}

/*absolute value*/
static int32 abs (int32 a){
	if (a<0)
		return -1*a;
	return a;
}

/*approrximate square root function*/
static long sqrt_approx(long input)
{
long temp;
long x = input;
long error = 1;
	if(x <= 0)
		return 0;
	do
	{
		temp = (x + input/x)/2;
		if(abs(x - temp) == error || abs(x - temp) == 0)
			return temp;
		x = temp;
	}while(1);
}
// -------------------------------------
// Ren()
//
// Charge/discharge REN measurement.
// Calibration would help but not required
// REN load relates to discharge time
//
// -------------------------------------

int32 ProSLIC_GR909_REN(ProSLICGR909Type *pProSLIC909,SI321x_REN_STATE *pState){

	unsigned char vring; 
	int32 renValue; 
	int32 diff;
	int i;
	int32 ac;
	switch (pState->State.stage){
		case 0:
			si321x_preserve_state(pProSLIC909,&(pState->State)); /* Save off current register settings */
			pState->sumIq2 = 0;
			// Assummed that all calibrations have been completed and we are OPEN
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0x1);// Enter active-MODE and charge
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, 0);//vtip = 0
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_REN_VCHARGE);//vring=46.12v
			pState->State.stage++; 
			//pState->State.stage=5;
			pState->State.sampleIterations=0;
			pState->State.waitIterations=0;
			pState->pollCount=0;
			return 0;
		case 1:
			delay_poll(&(pState->State),(500/POLL_RATE)-2);
			return 0;
		case 2:
			pState->v1 = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_V_SENSE);
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
			pState->State.stage++; 
			return 0;
		case 3:
			vring = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_V_SENSE);
			pState->pollCount++;
			if (vring < (pState->v1-SI321X_LT_REN_VSTOP) || (pState->pollCount>(2000/POLL_RATE))) //timeout 3 seconds
				pState->State.stage++; 
			return 0;
		case 4:
			//straight line curve fit (saves math)
			pProSLIC909->renValue = (((pState->pollCount*POLL_RATE)*SI321X_LT_REN_SLOPE) / 10 - SI321X_LT_REN_OFF)/10;
			if(pProSLIC909->renValue < 0)
				pProSLIC909->renValue = 0;	
		   
		   if (pState->pollCount>(2000/POLL_RATE)) //timeout
			   pProSLIC909->renValue = 0;
		   si321x_restore_state(pProSLIC909,&(pState->State));
#if (DEBUG)   
		   LOGPRINT(/*0,*/"SI321X_LT: RENLD for channel %u is milren= %ld\n",pState->State.channelNumber, pProSLIC909->renValue);
#endif
		   pState->State.stage++;
		   return 0;
		break;
		case 5:
			// Setup Ringer
		pProSLIC909->WriteRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_OSC_COEF, SI321X_LT_REN_RINGOSC);	// Ring 17vrms, 24vpk, 20Hz, 0vdc
		pProSLIC909->WriteRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_X, SI321X_LT_REN_RINGX);
		pProSLIC909->WriteRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_V_OFF, SI321X_LT_REN_RINGOF); //laj: 20Vdc needed to see beyond phone diodes

		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, RING_OSC_CTL, 0x2); // Disable timers
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, 0);//vtip = 0
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, SI321x_LT_REN_VOC);// Set VOC to 18v (Ring amp must be > Voc always)
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 1); // Forward active
		pState->State.stage++;
		return 0;
		break;
		case 6:
		delay_poll(&(pState->State),(600/POLL_RATE)-2);
		return 0;
		break;
		case 7:
		// Bleed line again and switch to ringing
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 0);
		pState->State.stage++;
		return 0;
		break;
		case 8:
		delay_poll(&(pState->State),(200/POLL_RATE)-2);
		return 0;
		break;
		case 9:
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 1);
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 4); // Start Ringing
		pState->pollCount = 0;
		pState->State.stage++;
		return 0;
		case 10:
		delay_poll(&(pState->State),(2500/POLL_RATE)-2);
		return 0;
		break;
		case 11:
		//if ((10/POLL_RATE) > 2){
		//	delay_poll(&(pState->State),(10/POLL_RATE)-2);
		//	return 0;
		//}
		//else
			pState->State.stage=12;
		case 12:
		if (pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64) == 0){//check for power alarm
#if (DEBUG)
			pProSLIC909->renValue=0;
			LOGPRINT ("Power Alarm!\n");
			return 1;
#endif
		}
		// Capture 500ms of samples - must be recalibrated if this changes or POLL_RATE changes
		pState->iValues[pState->pollCount]= 
			pProSLIC909->ReadRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 89);
		
		if ((++pState->pollCount) == (N_REN_SAMPLES))
			pState->State.stage=13;
		else 
			pState->State.stage=11;
		return 0;
		break;
		case 13:
#if (DEBUG)

		/*for (i=0;i<N_REN_SAMPLES;i++){
			
			LOGPRINT ("Ivalue = %d\n",pState->iValues[i]);
		}*/
#endif
		pState->sumIq2=0;
		for (i=0;i<N_REN_SAMPLES;i++){
			pState->sumIq2 += pState->iValues[i];
		}
		pState->sumIq2/=N_REN_SAMPLES;
		// Restore Ringer and DC Feed Settings
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 1); // Forward active
		pState->State.stage++;
		return 0;
		break;
		case 14:
		delay_poll(&(pState->State),(100/POLL_RATE)-2); 
		return 0;
		case 15:
		ProSLIC_SetLinefeedStatus(pProSLIC909->pProslic, 0); // Open state
		si321x_restore_state(pProSLIC909,&(pState->State));
		
		if(pProSLIC909->CalFlag == 1){
			pProSLIC909->renValue = pState->sumIq2;
			return 1;
		}

		else{
			
			if(pState->sumIq2 > pState->renCal->renTrans)
				renValue = ((pState->sumIq2 - pState->renCal->highRenOffs)*1000)/pState->renCal->highRenSlope;
			else{
				renValue = ((pState->sumIq2 - pState->renCal->lowRenOffs)*1000)/pState->renCal->lowRenSlope;
			}
			if (renValue < 900)
				renValue = ((pState->sumIq2 - pState->renCal->extraLowRenOffset)*1000)/pState->renCal->extraLowRenSlope;
#if (DEBUG)
			if (pProSLIC909->pProslic->debugMode){
				LOGPRINT("IQ2(sum) = %d\n",pState->sumIq2);
				LOGPRINT("I method REN = %d mREN\n",renValue);
			}
#endif		
			if (renValue > 600)
				pProSLIC909->renValue = renValue;
			
		}
		if(pProSLIC909->renValue < 0) 
			pProSLIC909->renValue=0;
		break;

	}
   	return 1;
}

// ----------------------------------------------------
// ResFault
//
// Accurate resistive fault measurement for faults
// between 10k and 1M.  Requires 680k shunt resistor
// accross TIP/RING
//
// Note:  the presence of a large REN load results in
//        slow settle times in TIP-OPEN and RING-OPEN
//        linefeed states.  This function attempts to
//        precharge TIP and RING to the voltages that
//        they will be at in TIP-OPEN/RING-OPEN modes,
//        but if a fault is present, it will take at
//        least the time constant of the load capacitance
//        and 200k sense resistor, ie.
//
//             1 REN -> 1.6sec
//             2 REN -> 3.2sec
//             3 REN -> 4.8sec
//             4 REN -> 6.4sec
//             5 REN -> 8.0sec
//
// 
// ----------------------------------------------------
int32 ResFault(ProSLICGR909Type *pProSLIC909,SI321x_RMEAS_STATE *pState)
{
	int32 oldVoltage;
	int32 newVoltage;
	int32 i;

	switch (pState->State.stage){
	case 0:
		si321x_preserve_state(pProSLIC909,&(pState->State)); /* Save off current register settings */
		pState->lowR.whichPath=0;
    	// Initialize Fault Values
    	pProSLIC909->tg = SI321X_LT_RESF_INIT_RES;
   	 	pProSLIC909->rg = SI321X_LT_RESF_INIT_RES;
    	pProSLIC909->tr = SI321X_LT_RESF_INIT_RES;
		pState->State.waitIterations=0;

	// -------------------------------------------
	// Test 3: Resistive Faults
	// -------------------------------------------
	// Assumming device has been initialized and is in forward active mode

		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
		pState->State.stage++;
		return 0;

	case 1:
		delay_poll(&pState->State,100/POLL_RATE-2);
		return 0;
	case 2:
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, SI321X_LT_RESF_VCM1);//vcm=9v
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_RESF_VOC1);// Voc = 46.5-Vcm+1.5 = 39
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 1);
		pState->State.stage++;
		return 0;

	case 3:
		delay_poll (&pState->State,60/POLL_RATE-2);
		return 0;

	case 4:
	// Set Vcm to the expected TIP voltage if there are no faults.  This reduces
	// the RC delay if a high REN load is present.  If a fault truely exists, 
	// it will increase the time constant and settle the TIP voltage faster.
	//
	// For the EVB case with a 680k Rsh and 46.5v VOC, 9v is the closest Vcm
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 3);
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_RESF_VOC2);
		pState->State.stage++;
		pState->State.sampleIterations=0; pState->dcRingV[0]=0; pState->dcTipV[0]=0;
		pState->lowR.State.stage=0;
		return 0;

	case 5:
		delay_poll(&pState->State,6000/POLL_RATE-2);
		pState->max = 0;
		return 0;

	case 6:
		pState->State.sampleIterations++;

	if (measDcTipRing_fixed(pProSLIC909,&pState->State,SI321X_LT_RESF_SAMPLES,pState->dcTipV,pState->dcRingV)){
		pState->State.stage=8;
		if (pState->dcTipV[0] != 0)
			pState->dcTipV[0] += SI321X_LT_RESF_ADJ;
		if (pState->dcRingV[0] != 0)
			pState->dcRingV[0] += SI321X_LT_RESF_ADJ;
	
		// Resistive fault equations are only valid for certain TIP voltage ranges.  
		if (pState->dcTipV[0] < SI321X_LT_RESF_TG_FAULT){ // Check for tip-gnd fault
					pProSLIC909->tg=((pState->dcTipV[0]-pState->dcRingV[0])/SI321X_LT_RESF_RSH);
					pProSLIC909->tg =(-1*(pState->dcTipV[0]*100)) / (pProSLIC909->tg+((pState->dcTipV[0]+1500)/SI321X_LT_RESF_RST));
					pProSLIC909->tg *= 100;
			
			// Min detected is 1.5k - make lower resolution measurement if < 10k
			if(pProSLIC909->tg <= SI321X_LT_RESF_MINRESF) {
				pState->lowR.whichPath=RTG;  
				pState->State.stage=7;
			}

		}
		else if (pState->dcTipV[0] > SI321X_LT_RESF_TR_FAULT){ // Check for tip-ring fault
			pProSLIC909->tr=((pState->dcRingV[0]-pState->dcTipV[0])*100)/(((1500+pState->dcTipV[0])/SI321X_LT_RESF_RST)-((pState->dcRingV[0]-pState->dcTipV[0])/SI321X_LT_RESF_RSH));
			pProSLIC909->tr *= 100;
			if (pProSLIC909->tr <= SI321X_LT_RESF_MINRESF){ 
				pState->lowR.whichPath=RTR;   
				pState->State.stage=7;
			}
			
		}
		
	}

    return 0;
	case 7:
	if (LowResFault (pProSLIC909,&pState->lowR)){
		pState->State.stage++;
		if (pProSLIC909->tg <= SI321X_LT_RESF_MINRESF){
			pProSLIC909->tg = pState->lowR.rVal;
		}
		else if (pProSLIC909->tr <= SI321X_LT_RESF_MINRESF){
			pProSLIC909->tr = pState->lowR.rVal;
		}
	}
	return 0;
	case 8:
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 5);
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, SI321X_LT_RESF_VOC1 | 0x40);// Voc=46.5-Vcm+1.5
	pState->State.stage++;
	return 0;
	case 9:
	delay_poll(&pState->State,60/POLL_RATE-2);
	return 0;
	case 10:
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 7);
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, SI321X_LT_RESF_VOC2 | 0x40);
	pState->State.stage++;
	pState->State.sampleIterations=0; pState->dcRingV[1]=0; pState->dcTipV[1]=0;
	return 0;
	case 11:
	delay_poll(&pState->State,6000/POLL_RATE-2);
	return 0;
	case 12:
	pState->State.sampleIterations++;

	if (measDcTipRing_fixed(pProSLIC909,&pState->State,8,&(pState->dcTipV[1]),&(pState->dcRingV[1]))){
		if (pState->dcRingV[1] != 0)
			pState->dcRingV[1]+= SI321X_LT_RESF_ADJ; 
		if (pState->dcTipV[1] != 0)
			pState->dcTipV[1] += SI321X_LT_RESF_ADJ; 
		pState->State.stage=14;
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
		if (pState->dcRingV[1] < SI321X_LT_RESF_TG_FAULT) {// Check for ring-gnd fault
			pProSLIC909->rg = (-100*pState->dcRingV[1])/(((1500+pState->dcRingV[1])/SI321X_LT_RESF_RSR)+((pState->dcRingV[1]-pState->dcTipV[1])/SI321X_LT_RESF_RSH));
			pProSLIC909->rg *= 100;
			if(pProSLIC909->rg <= SI321X_LT_RESF_MINRESF) 
				{ pState->State.stage=13;pState->lowR.whichPath=RRG;}
			else
				pState->State.stage=14;
		} 
		
	}

	return 0;
	case 13: //meas rrg <10k

	if (LowResFault (pProSLIC909,&pState->lowR)){
		pState->State.stage++;
		pProSLIC909->rg = pState->lowR.rVal;		
	}

	return 0;
	case 14:
	si321x_restore_state(pProSLIC909,&(pState->State)); /* restore prior register settings */
	return 1;
	
	}   
        
   return 0;
}

// ------------------------------------------------
// LowResFault
//
// Accurate resistive fault measurement for faults
// between 0 and 10k.  Works with or without
// shunt resistor accross TIP/RING, but it must
// be accounted for if installed.
//
// ------------------------------------------------
static int32 LowResFault(ProSLICGR909Type * pProSLIC909,SI321x_LOWRMEAS_STATE *pState)
{


unsigned char iLoop;
int32 tmp;
uInt8 temp=0;
int32 iLoopCalc, iLongCalc;
unsigned char vocVal = 0x20;

switch (pState->State.stage){
	case 0:
	pState->State.waitIterations=0;
	pState->rVal=SI321X_LT_RESF_INIT_RES;
	pState->i=SI321X_LT_RESF_INIT_VCM_LOWRESF;
    // Select which linefeed state to make measurement
    switch(pState->whichPath)
    {
        case RTR:
            pState->lfState=1;
        break;
        case RRG:
            pState->lfState=1;
        break;
        case RTG:
            pState->lfState=5;
        break;
    }
	// Assumming device has been initialized and is in forward active mode
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0); 
	pState->State.stage++;
	return 0;
	case 1:
	delay_poll(&pState->State,100/POLL_RATE-2);
	return 0;
	case 2:

    pProSLIC909->WriteRAM(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 28, 0x4000);  // LCRT 40mA
    pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 71, 7);       // ILIM 41mA
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 73, 2);     // Vcm = 3v
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, pState->i);     // Voc = 48v
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, pState->lfState);    // Go active
	pState->State.stage++;
	return 0;
	case 3:
	delay_poll(&pState->State,300/POLL_RATE-2);
    return 0;
	case 4:
    // Adjust VCM to keep ILOOP < 25mA to prevent overstressing linefeed transistors
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, (uInt8)((pState->i)|((pState->lfState==5) ? 0x40 : 0))); 
	delay_poll (&pState->State,100/POLL_RATE-2);
	return 0;
	case 5:
	iLoop = 0x3f & pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel,79);
	if (pState->i == 0){
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 72, 0x20);
            pState->rVal = 250;             // Return lowest possible measurement
			pState->State.stage=7;	
			return 1;
	}
	pState->i--;
	if (iLoop <= 20)//25mA
		pState->State.stage=6;
	else 
		pState->State.stage=4;
	pState->State.sampleIterations=0; pState->dcRingV=0;pState->dcTipV=0; pState->IQ1val=0;pState->IQ2val=0;pState->IQ5val=0;pState->IQ6val=0;
	return 0;
	case 6:
	delay_poll (&pState->State,200/POLL_RATE-2);
	return 0;
	case 7:
     // Measure linefeed transistor current, line voltages, and calculated fault
	pState->State.sampleIterations++;

    tmp = measDcTipRing_fixed(pProSLIC909,&pState->State,8, &pState->dcTipV, &pState->dcRingV);
	if (measIQ1256_fixed(pProSLIC909,&pState->State,8,&pState->IQ1val,&pState->IQ2val,&pState->IQ5val,&pState->IQ6val) && tmp){
		if (pState->dcRingV != 0)
			pState->dcRingV += SI321X_LT_RESF_ADJ; 
		if (pState->dcTipV != 0)
			pState->dcTipV += SI321X_LT_RESF_ADJ; 
		switch(pState->whichPath)
		{
			case RTR:    // Fault calculated from derived Iloop
			iLoopCalc = (pState->IQ1val+pState->IQ5val)/2;
			iLongCalc = (pState->IQ5val-pState->IQ1val)/2;
			pState->rVal = (((pState->dcRingV-pState->dcTipV)*1000)/(iLoopCalc-iLongCalc));
			break;
			case RRG:
			iLongCalc = (pState->IQ5val-pState->IQ1val)/2;
			if (iLongCalc == 0){
				temp++;
				pState->rVal = SI321X_LT_RESF_INIT_RES;
			}
			else
				pState->rVal = ((pState->dcRingV*1000)/(2*iLongCalc));
			break;
			case RTG:
			iLongCalc = (pState->IQ6val-pState->IQ2val)/2;
			if (iLongCalc == 0){
				temp++;
				pState->rVal = SI321X_LT_RESF_INIT_RES;
			}
			else
				pState->rVal = ((pState->dcTipV*1000)/(2*iLongCalc));
			break;
		}
	if (temp==2)
		pState->rVal=0;

	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 64, 0); 

	pState->State.stage++;
	}
	return 0;
	case 8:
		return 1;
	}
	return 1;
}

// ------------------------------------------------------
// ResFaultNoRshDiff
//
// Gross resistive fault detection without shunt resistor
// installed across tip-ring.  Detects the presence of an
// impedance Rmin < Rtr < Rmax across tip-ring. Maximum
// impedance detected is 60v/1.25mA = 48k.
// TEST ASSUMES si321x_lt_ResFaultNoRshLong will then be run!
//
// ------------------------------------------------------
static int32 ResFaultNoRshDiff(ProSLICGR909Type *pProSLIC909,SI321x_RMEASCOARSE_STATE *pState){
	/* This function measures resistive faults between tip and ring assuming no shunt resistor is present.
	*  The range of this measurement is restricted to small resistances (<30k). See the GR909 app note for
	*  more details.
	*/
	int32 tmp; 
	switch (pState->DiffState.stage)
	{
		case 0:
			si321x_preserve_state(pProSLIC909,&(pState->DiffState)); /* Save off current register settings */

		pState->DiffState.waitIterations=0;
		pState->DiffState.sampleIterations=0;
		pProSLIC909->tr=100000000;    // Default to 100meg
		pState->voc = 0x2; //3V
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, 0);//vcm=0
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, 0);
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 1);
		//pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, AUDIO_GAIN, 0x30); //mute audio
		pState->DiffState.stage++;
		return 0;
		case 1:
			delay_poll(&pState->DiffState,1000/POLL_RATE -2 );
			return 0;
		case 2:
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, 0);//vcm=0
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, pState->voc);
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 1);
		pState->DiffState.stage++;
		return 0;
		case 3:
			delay_poll (&pState->DiffState,100/POLL_RATE -2 );
		return 0;
		case 4:
		// Gradually increase Voc from 3v to 60v and monitor loop current.  
		// As soon as ILOOP[5:0] > 6 (but still less than LCRT), exit loop
		// and make resistance calculation.
		pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, pState->voc);
		delay_poll (&pState->DiffState,100/POLL_RATE -2);

		return 0;
		case 5:
		
		pState->iLoop=0x3f & pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LOOP_I_SENSE);
#if (DEBUG)		
		LOGPRINT(/*0,*/"iloop in diff: %u %d %02x\n",pState->iLoop,pState->voc,pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE));	
#endif
		if ((pState->iLoop < 6) && (pState->voc <= SI321X_LT_RESF_NORSH_VOCSTOP)){
			pState->voc+=2;
			pState->DiffState.stage=4;
		}
		else{
			pState->DiffState.stage=6;
			if (pState->iLoop > 1){
				pState->vTip = SI321X_LT_VOLT_SCALE * pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, TIP_V_SENSE);
				pState->vRing = SI321X_LT_VOLT_SCALE * pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel,RING_V_SENSE);;
				if (pState->vRing != 0)
					pState->vRing += SI321X_LT_RESF_ADJ; 
				if (pState->vTip != 0)
					pState->vTip += SI321X_LT_RESF_ADJ; 
				pProSLIC909->tr=((pState->vRing-pState->vTip)*1000)/(pState->iLoop*SI321X_LT_ISCALE);
	
				pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 5);
				 //reverse active
				
			}else {
				pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
				      // Go Open
				pState->DiffState.stage=8;
			}
		}

		return 0;
		case 6:
			delay_poll(&pState->DiffState,400/POLL_RATE -2);
		return 0;
		case 7:
			tmp = 0x3f & pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LOOP_I_SENSE);
			if (tmp<2)
				pProSLIC909->tr=100000000; //resistnace is not t-r
			pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);     // Go Open
			pState->DiffState.stage++;
#if (DEBUG)
			LOGPRINT(/*0,*/"rtr = %u vtip: %u vring = %u chn: %u\n",
					pProSLIC909->tr,
					pState->vTip,
					pState->vRing,
				pState->DiffState.channelNumber);
#endif
			return 1;
		case 8:
			return 1;
	}
	si321x_restore_state(pProSLIC909,&(pState->DiffState)); /* restore prior register settings */
	return 1;

}
// ------------------------------------------------------
// ResFaultNoRshLong
//
// Gross resistive fault detection without shunt resistor
// installed across tip-ring.  Detects the presence of an
// impedance Rmin < Rtr < Rmax across tip-ring. Maximum
// impedance detected is 60v/0.632mA = 95k.
// Test assumes si321x_lt_ResFaultNoRshDiff() was run first! 
//
// ------------------------------------------------------
static int32 ResFaultNoRshLong(ProSLICGR909Type *pProSLIC909,SI321x_RMEASCOARSE_STATE *pState){
	/*This function measurement resistive faults between tip-gnd and rng-gnd. 
	* The measurement assumes no shunt resistor between tip and ring and so the range is limited
	* to small resistances (<50k). See the GR909 app note for more details.
	*/
	
int32 tmp;

switch (pState->LongState.stage){
	case 0:
	pState->LongState.waitIterations=0;
	pState->LongState.sampleIterations=0;
	pProSLIC909->tg=SI321X_LT_RESF_NORSH_DEF_RES;    
	pProSLIC909->rg=SI321X_LT_RESF_NORSH_DEF_RES;    
	pState->vCm=0x2; //1.5V
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);
	pState->LongState.stage++;
	return 0;
	case 1:
	delay_poll(&pState->LongState,1000/POLL_RATE -2);
	return 0;
	case 2:
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, ON_HOOK_V, 0); // Voc = 0v
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, pState->vCm);
    pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 1); // Go forward active
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, 65, 0x40);//filter out noise on iq5/6
	pState->LongState.stage++;
	return 0;
	case 3:
	delay_poll(&pState->LongState,100/POLL_RATE-2);
	return 0;
	case 4:
    // Gradually increase Voc from 3v to 60v and monitor loop current.  
    // As soon as iTip or iRing > 0x3f (20mA), log voltage and 
    // make calculation
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, COMMON_V, pState->vCm);                // no fault condition.  No impact on accuracy.
	delay_poll(&pState->LongState,280/POLL_RATE-2);
	return 0;
	case 5:
	pState->iTip = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ6);
	pState->iRing = pProSLIC909->ReadReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, IQ5);
  
	if ((pState->vCm <= SI321X_LT_RESF_NORSH_VOCSTOP) && (pState->iTip < 0x3f) && (pState->iRing < 0x3f)){
		pState->LongState.stage=4;
		pState->vCm+=2;
	}
	else{
		pState->LongState.stage=6;
		pState->LongState.sampleIterations=0; pState->vRing=0; pState->vTip=0; pState->IQ1val=0;pState->IQ2val=0;pState->IQ5val=0;pState->IQ6val=0;
	}
	return 0;
	case 6:
	// Now that limiting value is sensed, measure more accurately
	pState->LongState.sampleIterations++;
	tmp = measDcTipRing_fixed(pProSLIC909,&pState->LongState,SI321X_LT_RESF_NORSH_SAMPLES, &pState->vTip, &pState->vRing);
	if (measIQ1256_fixed(pProSLIC909,&pState->LongState,SI321X_LT_RESF_NORSH_SAMPLES,&pState->IQ1val,&pState->IQ2val,&pState->IQ5val,&pState->IQ6val)&& tmp){
		pState->LongState.stage++;	
	}
	return 0;
	case 7:

	// Calculate longitudinal resistances if valid
	if (pState->vRing != 0)
		pState->vRing += SI321X_LT_RESF_ADJ; 
	if (pState->vTip != 0)
		pState->vTip += SI321X_LT_RESF_ADJ; 
     if(pState->iTip > 3){
        pProSLIC909->tg = (pState->vTip*1000)/pState->IQ6val;
    }
    
    if(pState->iRing > 3) {
        pProSLIC909->rg = (pState->vRing*1000)/pState->IQ5val;
    }
#if (DEBUG)
    LOGPRINT(/*0,*/"rrg = %u rtg: %u chn: %u\n",
					pProSLIC909->rg,
					pProSLIC909->tg,
					pState->LongState.channelNumber);
#endif
	pProSLIC909->WriteReg(pProSLIC909->pProHW, pProSLIC909->pProslic->channel, LINE_STATE, 0);  // Go Open
	si321x_restore_state(pProSLIC909,&(pState->DiffState)); /* restore prior register settings */
	pState->LongState.stage++;
	return 0;
	case 8:
	delay_poll(&pState->LongState,100/POLL_RATE-2);
	return 0;
	default:
		return 1;
	}
	si321x_restore_state(pProSLIC909,&(pState->DiffState)); /* restore prior register settings */
	return 1;

}






int32 ProSLIC_GR909_RESISTIVE_FAULT(ProSLICGR909Type *pProSLIC909,SI321x_RMEAS_STATE *pState){
	 // External Resistor between TIP-RING installed
        return ResFault(pProSLIC909,pState);
   	

}

int32 ProSLIC_GR909_RESISTIVE_FAULT_NOSH(ProSLICGR909Type *pProSLIC909,SI321x_RMEASCOARSE_STATE *pState){
	int32 i;
	i=ResFaultNoRshDiff(pProSLIC909,pState);
	if (i) //for r_tr //if we are done do next part of test
		return ResFaultNoRshLong(pProSLIC909,pState); //for r_tg & r_rg
	return i;
}



//$Log: si321x_gr909.c,v $
//Revision 1.1  2008-12-31 02:34:53  qwert
//Add si3210 driver
//
//Revision 1.18  2008/04/16 19:26:25  lajordan
//fixed ren test
//
//Revision 1.17  2008/04/16 16:39:42  lajordan
//no message
//
//Revision 1.16  2008/01/21 21:13:47  lajordan
//rename
//
//Revision 1.14  2007/08/09 15:44:32  lajordan
//fixed speedup caps REN test bug
//
//Revision 1.13  2007/06/05 15:40:09  lajordan
//no message
//
//Revision 1.12  2007/06/05 15:31:07  lajordan
//no message
//
//Revision 1.11  2007/06/04 23:14:07  lajordan
//no message
//
//Revision 1.10  2007/06/04 18:31:45  lajordan
//no message
//
//Revision 1.9  2007/05/29 19:49:58  lajordan
//changed to current sense ren test
//
//Revision 1.8  2007/05/24 15:18:42  lajordan
//no message
//
//Revision 1.7  2007/05/23 23:38:39  lajordan
//no message
//
//Revision 1.6  2007/05/23 22:36:42  lajordan
//found offset error
//
//Revision 1.5  2007/05/22 23:15:03  lajordan
//no message
//
//Revision 1.4  2007/05/22 22:46:47  lajordan
//no message
//
//Revision 1.3  2007/05/22 21:57:06  lajordan
//updated for HV BOM
//
//Revision 1.2  2007/04/03 20:26:19  lajordan
//improved ren test
//
//Revision 1.1  2007/04/02 19:27:10  lajordan
//no message
//
//Revision 1.19  2007/02/05 23:03:53  lajordan
//register read/ram read functions def changed
//
//Revision 1.18  2007/02/01 17:34:07  lajordan
//fixed resfault hang
//
//Revision 1.17  2007/02/01 04:08:39  lajordan
//updated to polling
//
//Revision 1.16  2006/11/29 20:33:06  lajordan
//comments
//
//Revision 1.15  2006/11/29 19:59:48  lajordan
//removed unused code
//
//Revision 1.13  2006/08/09 19:44:12  sasinha
//cosmetic
//
//Revision 1.12  2006/08/09 18:30:35  sasinha
//removed sqrt_approx from AC test
//
//Revision 1.11  2006/08/09 18:22:01  sasinha
//added PSTN
//
//Revision 1.10  2006/08/02 18:48:27  lajordan
//moved ui out of gr909.c
//
//Revision 1.9  2006/07/29 00:13:40  lajordan
//added dc offset to ringing REN test
//
//Revision 1.8  2006/07/28 23:43:15  lajordan
//added log fields
//