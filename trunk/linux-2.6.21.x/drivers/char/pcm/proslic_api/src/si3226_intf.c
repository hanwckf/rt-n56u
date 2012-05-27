/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si3226_intf.c,v 1.3 2012-01-09 12:56:42 qwert Exp $
**
** SI3226_Intf.c
** SI3226 ProSLIC interface implementation file
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
** This is the implementation file for the main ProSLIC API and is used 
** in the ProSLIC demonstration code. 
**
*/
extern int printk ( const char * format, ... );
#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si3226_intf.h"
#include "si3226.h"
#include "si3226_registers.h"
#include "proslic_api_config.h"

#define PRAM_ADDR (334 + 0x400)
#define PRAM_DATA (335 + 0x400)

#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr
#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteRAM	pProslic->deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAM		pProslic->deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsed pProslic->deviceId->ctrlInterface->timeElapsed_fptr
#define SetSemaphore pProslic->deviceId->ctrlInterface->Semaphore_fptr

#define WriteRegX		deviceId->ctrlInterface->WriteRegister_fptr
#define ReadRegX		deviceId->ctrlInterface->ReadRegister_fptr
#define pProHWX			deviceId->ctrlInterface->hCtrl
#define DelayX			deviceId->ctrlInterface->Delay_fptr
#define pProTimerX		deviceId->ctrlInterface->hTimer
#define WriteRAMX		deviceId->ctrlInterface->WriteRAM_fptr
#define ReadRAMX		deviceId->ctrlInterface->ReadRAM_fptr
#define TimeElapsedX	deviceId->ctrlInterface->timeElapsed_fptr

#define BROADCAST 0xff



/*
** Local functions are defined first
*/

/*
** Function: setUserMode
**
** Description: 
** Puts ProSLIC into user mode or out of user mode
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** on: specifies whether user mode should be turned on (TRUE) or off (FALSE)
**
** Return:
** none
*/
static int setUserMode (proslicChanType *pProslic,BOOLEAN on){
	uInt8 data;
	if (SetSemaphore != NULL){
		while (!(SetSemaphore (pProHW,1)));
		if (on == TRUE){
			if (pProslic->deviceId->usermodeStatus<2)
				pProslic->deviceId->usermodeStatus++;
		} else {
			if (pProslic->deviceId->usermodeStatus>0)
				pProslic->deviceId->usermodeStatus--;
			if (pProslic->deviceId->usermodeStatus != 0)
				return -1;
		}
	}
	data = ReadReg(pProHW,pProslic->channel,126);
	if (((data&1) != 0) == on)
		return 0;
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,2);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,8);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,0xe);
	WriteReg(pProHW,pProslic->channel,USERMODE_ENABLE,0);
	if (SetSemaphore != NULL)
		SetSemaphore(pProHW,0);
	return 0;
}


/*
** Function: setUserModeBroadcast
**
** Description: 
** Puts ProSLIC into user mode via broadcast
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** on: specifies whether user mode should be turned on (TRUE) or off (FALSE)
**
** Return:
** none
*/
static int setUserModeBroadcast (proslicChanType *pProslic,BOOLEAN on){
	uInt8 data;
	if (SetSemaphore != NULL){
		while (!(SetSemaphore (pProHW,1)));
		if (on == TRUE){
			if (pProslic->deviceId->usermodeStatus<2)
				pProslic->deviceId->usermodeStatus++;
		} else {
			if (pProslic->deviceId->usermodeStatus>0)
				pProslic->deviceId->usermodeStatus--;
			if (pProslic->deviceId->usermodeStatus != 0)
				return -1;
		}
	}
	data = ReadReg(pProHW,pProslic->channel,USERMODE_ENABLE);/*we check first channel. we assume all channels same user mode state*/
	if (((data&1) != 0) == on)
		return 0;
	WriteReg(pProHW,BROADCAST,USERMODE_ENABLE,2);
	WriteReg(pProHW,BROADCAST,USERMODE_ENABLE,8);
	WriteReg(pProHW,BROADCAST,USERMODE_ENABLE,0xe);
	WriteReg(pProHW,BROADCAST,USERMODE_ENABLE,0);
	if (SetSemaphore != NULL)
		SetSemaphore(pProHW,0);
	return 0;
}

/*
** Function: probeDaisyChain
**
** Description: 
** Determine number of devices on chain
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
**
** Return:
** number of channels detected
*/
static int probeDaisyChain (proslicChanType *pProslic){
	int i=0;
	WriteReg(pProHW,BROADCAST,RAM_ADDR_HI,0x40);
	while (ReadReg(pProHW,i++,RAM_ADDR_HI) == 0x40 && (i<=32));
	WriteReg(pProHW,BROADCAST,RAM_ADDR_HI,0x0);
	return i-1;
}

/*
** Function: cal_iteration
**
** Description: 
** Calibrate channels from different devices in parallel
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object
** iter: channel
** data: pointer to array of cal enable bits
** numChan: total number of channels in chain
**
** Return:
** error
*/

static int cal_iteration (proslicChanType *pProslic,int iter,uInt8 *data2,uInt8 numChan){
	int i;
	uInt8 data,data3;
	int millisecs=0;
	for (i=0;i<numChan;i+=2){ /*we do each channel in each chip*/
		WriteReg(pProHW,i+iter,CALR0,data2[0]);
		WriteReg(pProHW,i+iter,CALR1,data2[1]);
		WriteReg(pProHW,i+iter,CALR2,data2[2]);
		WriteReg(pProHW,i+iter,CALR3,data2[3]);
	}
	do{
		data3=0;
		for (i=0;i<numChan;i+=2){
			data = ReadReg(pProHW,i+iter,CALR3);
			data3 |= data;
		}
		Delay(pProTimer,1);
		if (millisecs++ > 3000){
#ifdef ENABLE_DEBUG
			if (pProslic->debugMode)
				LOGPRINT("Calibration timeout channel %d\n",iter);
#endif
		}
	}while ((data3&0x80)&&(millisecs<3000));
	return data3;
}


/*
** Function: Si3226_Calibrate
**
** Description: 
** Performs calibration based on passed ptr to array of
** desired CALRn settings.
**
** Run serial calibration.  Return error code on timeout
**
*/
int Si3226_Calibrate(proslicChanType_ptr *pProslic, int maxChan, uInt8 *calr, int maxTime)
{
int i,j;
int cal_en = 0;
int cal_en_chan = 0;
int timer = 0;

    /*
    ** Launch calibration on 1st port of each device in parallel, then
    ** launch on 2nd port of each device. 
    */
    for(j=0;j<CHAN_PER_DEVICE;j++)
    {
        for(i=j;i<maxChan;i+=CHAN_PER_DEVICE)
        {
            if(pProslic[i]->channelEnable)
            {
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,CALR0,calr[0]);
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,CALR1,calr[1]);
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,CALR2,calr[2]);
                pProslic[i]->WriteRegX(pProslic[i]->pProHWX,pProslic[i]->channel,CALR3,calr[3]);
            }
        }

        timer = 0;
        do 
        {
            cal_en = 0;
            pProslic[0]->DelayX(pProslic[0]->pProTimerX,10);
            for(i=j;i<maxChan;i+=CHAN_PER_DEVICE)
            {
                if(pProslic[i]->channelEnable)
                {
                    cal_en_chan = pProslic[i]->ReadRegX(pProslic[i]->pProHWX,pProslic[i]->channel,CALR3);
                    if((cal_en_chan&0x80)&&(timer == maxTime))
                    {
    #ifdef ENABLE_DEBUG
                        if(pProslic[i]->debugMode)
                        {
                            LOGPRINT("Calibration timout channel %d\n",i);
                        }
    #endif
                        pProslic[i]->channelEnable = 0;
                        pProslic[i]->error = RC_CAL_TIMEOUT;
                    }
                    cal_en |= cal_en_chan;
                }
            }         
        }while((timer++ <= maxTime)&&(cal_en&0x80));
    }

    return cal_en;
}
/*
** Function: Si3226_PowerUpConverter
**
** Description: 
** Powers up DC/DC converter
**
** Returns:
** int (error)
**
*/
int Si3226_PowerUpConverter(proslicChanType_ptr pProslic)
{
errorCodeType error = RC_NONE;
int32 vbath,vbat;
int timer = 0;
    /*
    ** Steps 1-5:
    ** - powerup dc/dc w/ OV clamping and shutdown
    ** - delay
    ** - clear dcdc status
    ** - switch to OV clamping only (no shutdown)
    ** - delay
    */
    
    setUserMode(pProslic,TRUE);
    WriteRAM(pProHW,pProslic->channel,PD_DCDC,0x700000L);
    WriteRAM(pProHW,pProslic->channel,PD_DCDC,0x600000L);
    Delay(pProTimer,50);
    WriteRAM(pProHW,pProslic->channel,DCDC_STATUS,0L);   
    WriteRAM(pProHW,pProslic->channel,PD_DCDC,0x400000L);
    Delay(pProTimer,50);

    /*
    ** Step 6:
    ** - monitor vbat vs expected level (VBATH_EXPECT)
    */
    vbath = ReadRAM(pProHW,pProslic->channel,VBATH_EXPECT);
    do
    {
	    vbat = ReadRAM(pProHW,pProslic->channel,MADC_VBAT);
        if(vbat & 0x10000000L)
            vbat |= 0xF0000000L;
		Delay(pProTimer,10);
    }while((vbat < (vbath - COMP_5V))&&(timer++ < TIMEOUT_DCDC_UP));  /* 2 sec timeout */
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
            LOGPRINT ("VBAT Up = %d.%d v\n",(vbat/SCALE_V_MADC)/1000, ((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000));
        }
#endif
    if(timer > 200)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_UP_TIMEOUT;
        WriteRAM(pProHW,pProslic->channel,PD_DCDC, 0x900000L); /* shutdown converter */
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
	        LOGPRINT ("Si3226 DCDC Power up timeout channel %d - disabling channel\n",pProslic->channel);
        }
#endif      
    }

    setUserMode(pProslic,FALSE);
    return error;  
}

/*
** Function: Si3226_PowerDownConverter
**
** Description: 
** Safely powerdown dcdc converter after ensuring linefeed
** is in the open state.  Test powerdown by setting error
** flag if detected voltage does no fall below 5v.
**
** Returns:
** int (error)
**
*/
int Si3226_PowerDownConverter(proslicChanType_ptr pProslic)
{
errorCodeType error = RC_NONE;
ramData vbat;
int timer = 0;
    
    setUserMode(pProslic,TRUE);
    WriteReg(pProHW,pProslic->channel,LINEFEED, LF_OPEN);
    Delay(pProTimer,50);
    WriteRAM(pProHW,pProslic->channel,PD_DCDC,0x300000L);
    Delay(pProTimer,50);

    /*
    ** Verify VBAT falls below 10v (QCUK slower because
    ** because of larger output cap)
    */
    do
    {
	    vbat = ReadRAM(pProHW,pProslic->channel,MADC_VBAT);
		Delay(pProTimer,10);
    }while((vbat > COMP_10V)&&(timer++ < TIMEOUT_DCDC_DOWN));  /* 1000 msec timeout */
#ifdef ENABLE_DEBUG
    if(pProslic->debugMode)
    {
        LOGPRINT ("VBAT Down = %d.%d v\n",(vbat/SCALE_V_MADC)/1000, ((vbat/SCALE_V_MADC) - (vbat/SCALE_V_MADC)/1000*1000));
    }
#endif
    if(timer > 20)
    {
        /* Error handling - shutdown converter, disable channel, set error tag */
        pProslic->channelEnable = 0;
        error = RC_VBAT_DOWN_TIMEOUT;
#ifdef ENABLE_DEBUG
        if(pProslic->debugMode)
        {
	        LOGPRINT ("Si3226 DCDC Power Down timeout channel %d\n",pProslic->channel);
        }
#endif      
    }

    setUserMode(pProslic,FALSE);
    return error;  
}


/*
** Function: calibrate
**
** Description: 
** Performs calibration (use deprecated) - replaced by Si3226_Calibrate()
**
*/

static int calibrate (proslicChanType *pProslic, int broadcast){
	/*
	** This function will perform the ProSLIC calibration sequence 
	*/
	int i,j,k;
	uInt8 data [] = {0x0, 0x0, 0x1, 0x80};/*madc cal is done before other cals*/
	uInt8 data3=0;
	uInt8 numChan=0;
	ramData vbat,vbat_min=0,vbath;
	if (broadcast){
		/*# fix coefficient for zcal*/
		WriteRAM(pProHW,BROADCAST,746,0x8F00000L);
		WriteRAM(pProHW,BROADCAST,927,0x1FFE0000L);
		setUserModeBroadcast(pProslic,TRUE);
		WriteRAM(pProHW,BROADCAST,1537,0x3200000L); /*fix lkg stndby offset for oht*/
		setUserModeBroadcast(pProslic,FALSE);
		numChan = probeDaisyChain(pProslic);
		for (i=0;i<2;i++){
			data3 |= cal_iteration(pProslic,i,data,numChan);
		}
	}
	else {/*to do - add parallel cals on separate devices*/
		/*# fix coefficient for zcal*/
		WriteRAM(pProHW,pProslic->channel,746,0x8F00000L);
		WriteRAM(pProHW,pProslic->channel,927,0x1FFE0000L);
		setUserMode(pProslic,TRUE);
		WriteRAM(pProHW,pProslic->channel,1537,0x3200000L); /*fix lkg stndby offset for oht*/
		setUserMode(pProslic,FALSE);
		numChan = 1;
		i=0;
		WriteReg(pProHW,pProslic->channel,CALR0,data[0]);
		WriteReg(pProHW,pProslic->channel,CALR1,data[1]);
		WriteReg(pProHW,pProslic->channel,CALR2,data[2]);
		WriteReg(pProHW,pProslic->channel,CALR3,data[3]);
		do{
			data3 = ReadReg(pProHW,pProslic->channel,CALR3);
			Delay(pProTimer,1);

			if (i++ > 3000){
#ifdef ENABLE_DEBUG
				if (pProslic->debugMode)
					LOGPRINT("Calibration timeout channel %d\n",pProslic->channel);
#endif
				pProslic->error = RC_CAL_TIMEOUT;
				pProslic->channelEnable = 0;
			
			}
		} while (i<3000 && (data3&0x80));
	}
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("MADC cal done. channel %d\n",pProslic->channel);
#endif

	/*run the rest of the cals*/
	data[1] = 0xbf; /*no zcal*/
	data[2] = 0xf8;
	data[3] = 0x80;
	if (broadcast){
		
		for (i=0;i<2;i++){
			/*start up convertor*/
			
			setUserModeBroadcast(pProslic,TRUE);
			WriteRAM(pProHW,BROADCAST,1538, 0x600000L);
			Delay(pProTimer,100);
			WriteRAM(pProHW,BROADCAST,1538, 0x400000L);
			setUserModeBroadcast(pProslic,FALSE);
			
			k=0;
			do {
				vbat_min = 0x7FFFFFFFL;
				vbath = ReadRAM(pProHW,i,VBATH_EXPECT);
				for (j=0;j<numChan;j+=2){
					vbat = ReadRAM(pProHW,j+i,MADC_VBAT);
					if (vbat < vbat_min)
						vbat_min = vbat;
				}
				Delay(pProTimer,10);
				if (k > 200){
#ifdef ENABLE_DEBUG
					LOGPRINT ("Si3226 DC-DC Power up timeout\n");
#endif
					return RC_VBAT_UP_TIMEOUT;
				}
			} while ((vbat_min < (vbath-0x51EB82L)) && (k++ < 200)); /*2 second timeout*/
			data3 |= cal_iteration(pProslic,i,data,numChan);
		}
	}
	else {
		
		/*converter startup sequence*/
		setUserMode(pProslic,TRUE);
		WriteRAM(pProHW,pProslic->channel,1538, 0x600000L);
		Delay(pProTimer,100);
		WriteRAM(pProHW,pProslic->channel,1538, 0x400000L); 
		setUserMode(pProslic,FALSE);
		
		k=0;
		do {
				vbath = ReadRAM(pProHW,pProslic->channel,VBATH_EXPECT);
				vbat = ReadRAM(pProHW,pProslic->channel,MADC_VBAT);			
				Delay(pProTimer,10);
				if (k > 200){
#ifdef ENABLE_DEBUG
					LOGPRINT ("Si3226 DC-DC Power up timeout\n");
#endif
					return RC_VBAT_UP_TIMEOUT;
				}
		} while ((vbat < (vbath-0x51EB82L)) && (k++ < 200)); /*2 second timeout, 5V margin*/
		i=0;
		WriteReg(pProHW,pProslic->channel,CALR0,data[0]);
		WriteReg(pProHW,pProslic->channel,CALR1,data[1]);
		WriteReg(pProHW,pProslic->channel,CALR2,data[2]);
		WriteReg(pProHW,pProslic->channel,CALR3,data[3]);
		do{
			data3 = ReadReg(pProHW,pProslic->channel,CALR3);
			Delay(pProTimer,100);

			if (i++ > 30){
#ifdef ENABLE_DEBUG
				if (pProslic->debugMode)
					LOGPRINT("Calibration timeout channel %d\n",pProslic->channel);
#endif
				pProslic->error = RC_CAL_TIMEOUT;
				pProslic->channelEnable = 0;
			
			}
		} while (i<30 && (data3&0x80));
	}
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Calibration done. channel %d\n",pProslic->channel);
#endif
	return data3;
}


/*
** Function: LoadRegTables
**
** Description: 
** Generic function to load register/RAM with predefined addr/value 
*/
static int LoadRegTables (proslicChanType *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int broadcast){
	uInt16 i;
	uInt8 channel;
	if (broadcast){
		channel = BROADCAST;
		setUserModeBroadcast(pProslic,TRUE);
	}
	else {
		channel = pProslic->channel;
		setUserMode(pProslic,TRUE);
	}

	i=0; 
	if (pRamTable != 0){
		while (pRamTable[i].address != 0xffff){
			WriteRAM(pProHW,channel,pRamTable[i].address,pRamTable[i].initValue); 
			i++;
		}
	}
	i=0;
	if (pRegTable != 0){
		while (pRegTable[i].address != 0xff){
			WriteReg(pProHW,channel,pRegTable[i].address,pRegTable[i].initValue);
			i++;
		}
	}
	if (broadcast)
		setUserModeBroadcast(pProslic,FALSE);
	else
		setUserMode(pProslic,FALSE);

	return 0;
}


/*
** Function: enableSi3226Loopman
**
** Description: 
** Turns of ISNS HVIC mode and enables MADC_LOOPMAN
** for current sensing 
*/

#ifdef DISABLE_LOOPMAN
#else
int enableSi3226Loopman (proslicChanType *pProslic, int broadcast){
	uInt8 channel;
	if (broadcast){
		setUserModeBroadcast(pProslic,TRUE);
		channel = BROADCAST;
	}
	else{
		setUserMode (pProslic,TRUE); /*make sure we are in user mode */
		channel = pProslic->channel;
	}

    /* Removed forced standby disable - fixed on revC */

    /* enable loopman */
	WriteRAM(pProHW, channel,MADC_LOOP_MAN, 0x300000L);
	WriteReg(pProHW, channel,PDN,0x80); /*enable MADC to prevent power alarm. this could also be done before going active*/
	if (broadcast){
		setUserModeBroadcast(pProslic,FALSE);
	}
	else {
		setUserMode(pProslic,FALSE); /*turn off user mode*/
	}
	return 0;
}

/*
** Function: disableSi3226Loopman
**
** Description: 
** Turns off MADC_LOOPMAN and enables HVIC ISNS mode for
** current sensing
*/
int disableSi3226Loopman (proslicChanType *pProslic, int broadcast){
	uInt8 channel;
	if (broadcast){
		setUserModeBroadcast(pProslic,TRUE);
		channel = BROADCAST;
	}
	else{
		setUserMode (pProslic,TRUE); /*make sure we are in user mode */
		channel = pProslic->channel;
	}
	WriteRAM(pProHW, channel,HVIC_CNTL_MAN, 0x200000L);
	WriteRAM(pProHW, channel,MADC_LOOP_MAN, 0x200000L);
	if (broadcast){
		setUserModeBroadcast(pProslic,FALSE);
	}
	else {
		setUserMode(pProslic,FALSE); /*turn off user mode*/
	}
	return 0;
}
#endif

/*
** Function: LoadSi3226Patch
**
** Description: 
** Load patch from external file
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** broadcast:  broadcast flag
**
** Return:
** 0
*/

static int LoadSi3226Patch (proslicChanType *pProslic, const proslicPatch *pPatch,int broadcast){
	int32 loop; 
	uInt8 jmp_table=PATCH_JMPTBL_START_ADDR;
	uInt8 channel;
    uInt8 enablePRAMLoad = 1;
	if (pPatch == NULL)
		return 0;
	if (broadcast){
		setUserModeBroadcast(pProslic,TRUE);
		channel = BROADCAST;
	}
	else{
		setUserMode (pProslic,TRUE); /*make sure we are in user mode to load patch*/
		channel = pProslic->channel;
	}

	WriteReg (pProHW, channel, JMPEN,0); /*disable Patch RAM*/

	for (loop=0;loop<PATCH_NUM_ENTRIES;loop++){
		/*zero out the jump table*/
		WriteReg (pProHW, channel, jmp_table,0);
		WriteReg (pProHW, channel, jmp_table+1,0);
	
		jmp_table+=2;
	}

    if(pProslic->channel > 0) {
        if((pProslic->channel & 1)&&((pProslic-1)->channelEnable)) { /* odd channel, previous even channel enabled */
            enablePRAMLoad = 0;
        }
    }


    if(enablePRAMLoad) {
	    WriteRAM(pProHW, channel,PRAM_ADDR, 0); /*write patch ram address register
	    If the data is all 0, you have hit the end of the programmed values and can stop loading.*/
        for (loop=0; loop<PATCH_MAX_SIZE; loop++){
		    if (pPatch->patchData[loop] != 0){
			    if ((pProslic->deviceId->chipRev < 3) && broadcast)
				    WriteRAM(pProHW, channel,PRAM_ADDR, loop<<19); /*write patch ram address register (only necessary for broadcast rev c and earlier)*/
                WriteRAM(pProHW, channel,PRAM_DATA,pPatch->patchData[loop]<<9); /*loading patch, note. data is shifted*/
		    }
            else
                loop = 1024;
        }
    }

    /*zero out RAM_ADDR_HI*/
	WriteReg (pProHW, channel, RAM_ADDR_HI,0);

	jmp_table=PATCH_JMPTBL_START_ADDR;
	for (loop=0;loop<PATCH_NUM_ENTRIES;loop++){
		/* Load the jump table with the new values.*/
		if (pPatch->patchEntries[loop] != 0){
			WriteReg (pProHW, channel, jmp_table,(pPatch->patchEntries[loop])&0xff);
			WriteReg (pProHW, channel, jmp_table+1,pPatch->patchEntries[loop]>>8);
		}
		jmp_table+=2;
	}

	WriteRAM(pProHW,channel,PATCH_ID,pPatch->patchSerial); /*write patch identifier*/
 
    /* Write patch support RAM locations (if any) */
    for (loop=0; loop<PATCH_MAX_SUPPORT_RAM; loop++){
        if(pPatch->psRamAddr[loop] != 0) {
            WriteRAM(pProHW,channel,pPatch->psRamAddr[loop],pPatch->psRamData[loop]);
        }
        else {
            loop = PATCH_MAX_SUPPORT_RAM;
        }
    }


#ifdef DISABLE_VERIFY_PATCH
	WriteReg (pProHW, channel, JMPEN,1); /*enable the patch (do not enable if you want to verify)*/
#endif
	if (broadcast){
		setUserModeBroadcast(pProslic,FALSE);
	}
	else {
		setUserMode(pProslic,FALSE); /*turn off user mode*/
	}
	return 0;
}

/*
** Function: handleError
**
** Description: 
** Called whenever an error is encountered with the proslic
** for future implementation or customization of error handling
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
	LOGPRINT ("Error encountered\n\n");
#endif
	/*TODO: add something to recover from power alarm here?*/
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
int Si3226_Reset (proslicChanType_ptr pProslic){
	/*
	** resets ProSLIC, wait 250ms, release reset, wait 250ms
	*/
	Reset(pProHW,1);
	Delay(pProTimer,250);
	Reset(pProHW,0);
	Delay(pProTimer,250);
	return 0;
}

/*
** Function: ProSLIC_ShutdownChannel
**
** Description: 
** Safely shutdown channel w/o interruptions to
** other active channels
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0

*/
int Si3226_ShutdownChannel (proslicChanType_ptr pProslic){
	/*
	** set linefeed to open state, powerdown dcdc converter
	*/
    Si3226_SetLinefeedStatus(pProslic,LF_OPEN);
    Delay(pProTimer,10);
    Si3226_PowerDownConverter(pProslic);
    
	return 0;
}

/*
** Function: Si3226_VerifyControlInterface
**
** Description: 
** Check control interface readback cababilities
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
**
** Return:
** 0
*/
int Si3226_VerifyControlInterface (proslicChanType_ptr pProslic)
{
	int i;
	int numOfChan = probeDaisyChain(pProslic);
	if (numOfChan == 0)
		return RC_SPI_FAIL;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Found %d channels\n",numOfChan);
#endif
	WriteReg(pProHW,BROADCAST,PCMRXLO,0x5a);
	WriteRAM(pProHW,BROADCAST,UNUSED449,0x12345678L);

	for (i=0;i<numOfChan;i++){
		/*Try to write innocuous register to test SPI is working*/
		
		if (ReadReg(pProHW,i,PCMRXLO) != 0x5A){
			handleError(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
			if (pProslic->debugMode)
				LOGPRINT("Proslic %d not communicating. Register access fail.\n",i);
#endif
			return RC_SPI_FAIL;
		}	
		if (ReadRAM(pProHW,i,UNUSED449) != 0x12345678L){
			handleError(pProslic,RC_SPI_FAIL);
#ifdef ENABLE_DEBUG
			if (pProslic->debugMode)
				LOGPRINT("Proslic %d not communicating. RAM access fail.\n",i);
#endif
			return RC_SPI_FAIL;
		}

	}
	return 0;
}
/*
** Function: Si3226_Init
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
/* TODO: Generic patch label (prevent unnecessary sw revs)
   TODO: broadcast gen param load and patch load (save ex time)
   TODO: patch verification failure error handling
   TODO: dcdc powerup error handling
   TODO: calibration timeout error handling
*/
extern const proslicPatch RevCPatch;
extern const proslicPatch RevDPatch;
extern const proslicPatch RevEPatch;
extern Si3226_General_Cfg Si3226_General_Configuration;
int Si3226_Init (proslicChanType_ptr *pProslic, int size){
	/*
	** This function will initialize the chipRev and chipType members in pProslic
	** as well as load the initialization structures.
	*/
	uInt8 data; 
    uInt8 calSetup[] = {0x00, 0x00, 0x01, 0x80}; /* CALR0 - CALR3 */
	int k;
    proslicPatch *patch;

    /*
    ** Read channel id to establish chipRev and chipType
    */
	for (k=0;k<size;k++){
		data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,ID);
		pProslic[k]->deviceId->chipRev = data&0x7;
		pProslic[k]->deviceId->chipType= ((data&0x31)>>3) + SI3226;
		printk("chipRev=%d chipType=%d [0x%02X]\n",pProslic[k]->deviceId->chipRev,pProslic[k]->deviceId->chipType,data);
	}

    /*
    ** Probe each channel and enable all channels that respond 
    */
    
	for (k=0;k<size;k++){
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO,0x5A);
		if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO) != 0x5A){
			pProslic[k]->channelEnable = 0;
			pProslic[k]->error = RC_SPI_FAIL;
			printk("PCMRXLO=0x%02X\n",pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO));
            return RC_SPI_FAIL;
		}
	}
	
    /*
    ** Load patch 
    */
	for (k=0;k<size;k++){
		if (pProslic[k]->channelEnable){
            switch(pProslic[k]->deviceId->chipRev) {
                case C:
                    patch = &RevCPatch;
                    printk("Rev C\n");
                    break;
                case D:
                    patch = &RevDPatch;
                    printk("Rev D\n");
                    break;
                case E:
                    patch = &RevEPatch;
                    printk("Rev E\n");
                    break;
            }
        
			Si3226_LoadPatch(pProslic[k],patch);
#ifdef DISABLE_VERIFY_PATCH
#else
			data = Si3226_VerifyPatch(pProslic[k],patch);
			if (data){
				pProslic[k]->channelEnable=0;
				pProslic[k]->error = RC_PATCH_ERR;	
                return RC_PATCH_ERR;
			} else {
				setUserMode(pProslic[k],TRUE);
				pProslic[k]->WriteRegX (pProslic[k]->pProHWX, pProslic[k]->channel,JMPEN,1);  
				setUserMode(pProslic[k],FALSE);
			}
#endif				

		}

	}

    /*
	** Load general parameters - includes all BOM dependencies.
    ** Any differences in settings among revisions is handled 
    ** in preset.
    */
	for (k=0;k<size;k++){ 
		if (pProslic[k]->channelEnable){
			setUserMode(pProslic[k],TRUE);      
	
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_VREF_MIN,Si3226_General_Configuration.dcdc_vref_min);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_VREF_MIN_RNG,Si3226_General_Configuration.dcdc_vref_min_ring);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,VBATR_EXPECT,Si3226_General_Configuration.vbatr_expect);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_NORM,Si3226_General_Configuration.dcdc_fsw_norm);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_NORM_LO,Si3226_General_Configuration.dcdc_fsw_norm_lo);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_DIN_LIM,Si3226_General_Configuration.dcdc_din_lim);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_RING,Si3226_General_Configuration.dcdc_fsw_ring);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_RING_LO,Si3226_General_Configuration.dcdc_fsw_ring_lo);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_OITHRESH,Si3226_General_Configuration.dcdc_oithresh);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_OVTHRESH,Si3226_General_Configuration.dcdc_ovthresh);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_UVHYST,Si3226_General_Configuration.dcdc_uvhyst);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_UVTHRESH,Si3226_General_Configuration.dcdc_uvthresh);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_VOUT_LIM,Si3226_General_Configuration.dcdc_vout_lim);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_VHYST,Si3226_General_Configuration.dcdc_fsw_vhyst);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_FSW_VTHLO,Si3226_General_Configuration.dcdc_fsw_vthlo);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_SWDRV_POL,Si3226_General_Configuration.dcdc_swdrv_pol);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_SWFET,Si3226_General_Configuration.dcdc_swfet);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_VREF_CTRL,Si3226_General_Configuration.dcdc_vref_ctrl);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,DCDC_RNGTYPE,Si3226_General_Configuration.dcdc_rngtype);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,COEF_P_HVIC,Si3226_General_Configuration.coef_p_hvic);
			pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,P_TH_HVIC,Si3226_General_Configuration.p_th_hvic);
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CM_CLAMP,Si3226_General_Configuration.cm_clamp);
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,AUTO,Si3226_General_Configuration.autoRegister); 

            /* Hardcoded mods to default settings - applies to all rev's and configs */
            data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX, pProslic[k]->channel,GPIO_CFG1);
            data &= 0xF9;
            data |= 0x60;
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,GPIO_CFG1,data); /* coarse sensors analog mode */
            pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,PDN,0x80); /* madc powered in open state */
            pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,TXACHPF_A1_1,0x71EB851L); /* Fix HPF corner */
            pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,ROW0_C2, 0x72339DL);   /* improved DTMF det */
            pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,ROW1_C2, 0x57A804L);   /* improved DTMF det */
			setUserMode(pProslic[k],FALSE);
		}
	}

    /*
	** Calibrate (madc offset)
    */
    Si3226_Calibrate(pProslic,size,calSetup,TIMEOUT_MADC_CAL);

    /*
	** Bring up DC/DC converters sequentially to minimize
    ** peak power demand on VDC
    */
	for (k=0;k<size;k++)
    { 
		if (pProslic[k]->channelEnable)
        {
			setUserMode(pProslic[k],TRUE);      
            pProslic[k]->error = Si3226_PowerUpConverter(pProslic[k]);
            setUserMode(pProslic[k],FALSE);
        }
    }

    /*
	** Execute remaining cals (except madc, lb)
    */
    calSetup[1] = CAL_STD_CALR1;
    calSetup[2] = CAL_STD_CALR2;
    Si3226_Calibrate(pProslic,size,calSetup,TIMEOUT_GEN_CAL);

    /*
    ** Enable Loopman
    */
	for (k=0;k<size;k++){ /*chan 0 shut off during chan 1 bring up*/
		if (pProslic[k]->channelEnable){
#ifdef DISABLE_LOOPMAN
#else
				enableSi3226Loopman(pProslic[k],FALSE);
#endif
		}
	}

	return 0;
}

/*
** Function: Si3226_InitBroadcast
**
** Description: 
** - probe SPI to establish daisy chain length
** - load patch
** - initialize general parameters
** - calibrate madc
** - bring up DC/DC converters
** - calibrate everything except madc & lb
**
** Input Parameters: 
** pProslic: pointer to PROSLIC object array
** fault: error code
**
** Return:
** error code
*/
int Si3226_InitBroadcast (proslicChanType_ptr *pProslic){
	uInt8 data; 
	int16 size;
    uInt8 calSetup[] = {0x00, 0x00, 0x01, 0x80}; /* CALR0 - CALR3 */
	int k;

	/*
	** Probe daisy chain to see how many channels
	** require initialization
	*/
	size = probeDaisyChain(pProslic[0]);

    /*
    ** Read channel id to establish chipRev and chipType
    */
	for (k=0;k<size;k++){
		data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,ID);
		pProslic[k]->deviceId->chipRev = data&0x7;
		pProslic[k]->deviceId->chipType= ((data&0x31)>>3) + SI3226;
	}

    /*
    ** Probe each channel and enable all channels that respond 
    */
	for (k=0;k<size;k++){
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO,0x5a);
		if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO) != 0x5A){
			pProslic[k]->channelEnable = 0;
			pProslic[k]->error = RC_SPI_FAIL;
		}
	}
	
    /*
    ** Broadcast patch load, but must verify sequentially 
    */
	if (pProslic[0]->deviceId->chipRev == 2) {
		LoadSi3226Patch(pProslic[0],&RevCPatch,1);
	}
	else if (pProslic[0]->deviceId->chipRev == 3){
		LoadSi3226Patch(pProslic[0],&RevDPatch,1);
	}

	for (k=0;k<size;k++){
		if (pProslic[k]->channelEnable){
			if (pProslic[k]->deviceId->chipRev == 2 ) {/*rev c*/
#ifdef DISABLE_VERIFY_PATCH
#else
				data = Si3226_VerifyPatch(pProslic[k],&RevCPatch);
				if (data){
					pProslic[k]->channelEnable=0;
					pProslic[k]->error = RC_PATCH_ERR;	
				} else {
					setUserMode(pProslic[k],TRUE);
					pProslic[k]->WriteRegX (pProslic[k]->pProHWX, pProslic[k]->channel,JMPEN,1);  
					setUserMode(pProslic[k],FALSE);
				}
#endif				

			}
			if (pProslic[k]->deviceId->chipRev == 3 ) {/*rev d*/
#ifdef DISABLE_VERIFY_PATCH
#else
				data = Si3226_VerifyPatch(pProslic[k],&RevDPatch);
				if (data){
					pProslic[k]->channelEnable=0;
					pProslic[k]->error = RC_PATCH_ERR;	
				} else {
					setUserMode(pProslic[k],TRUE);
					pProslic[k]->WriteRegX (pProslic[k]->pProHWX, pProslic[k]->channel,JMPEN,1);  
					setUserMode(pProslic[k],FALSE);
				}
#endif				

			}
		}

	}

    /*
	** Load general parameters - includes all BOM dependencies.
    ** Any differences in settings among revisions is handled 
    ** in preset.
    */

    setUserModeBroadcast(pProslic[0],TRUE);     
	
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_VREF_MIN,Si3226_General_Configuration.dcdc_vref_min);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_VREF_MIN_RNG,Si3226_General_Configuration.dcdc_vref_min_ring);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,VBATR_EXPECT,Si3226_General_Configuration.vbatr_expect);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_NORM,Si3226_General_Configuration.dcdc_fsw_norm);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_NORM_LO,Si3226_General_Configuration.dcdc_fsw_norm_lo);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_DIN_LIM,Si3226_General_Configuration.dcdc_din_lim);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_RING,Si3226_General_Configuration.dcdc_fsw_ring);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_RING_LO,Si3226_General_Configuration.dcdc_fsw_ring_lo);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_OITHRESH,Si3226_General_Configuration.dcdc_oithresh);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_OVTHRESH,Si3226_General_Configuration.dcdc_ovthresh);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_UVHYST,Si3226_General_Configuration.dcdc_uvhyst);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_UVTHRESH,Si3226_General_Configuration.dcdc_uvthresh);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_VOUT_LIM,Si3226_General_Configuration.dcdc_vout_lim);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_VHYST,Si3226_General_Configuration.dcdc_fsw_vhyst);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_FSW_VTHLO,Si3226_General_Configuration.dcdc_fsw_vthlo);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_SWDRV_POL,Si3226_General_Configuration.dcdc_swdrv_pol);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_SWFET,Si3226_General_Configuration.dcdc_swfet);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_VREF_CTRL,Si3226_General_Configuration.dcdc_vref_ctrl);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,DCDC_RNGTYPE,Si3226_General_Configuration.dcdc_rngtype);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,COEF_P_HVIC,Si3226_General_Configuration.coef_p_hvic);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,P_TH_HVIC,Si3226_General_Configuration.p_th_hvic);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,CM_CLAMP,Si3226_General_Configuration.cm_clamp);
    pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,AUTO,Si3226_General_Configuration.autoRegister); 

    /* Hardcoded mods to default settings - applies to all rev's and configs */
    data = pProslic[0]->ReadRegX(pProslic[0]->pProHWX, pProslic[0]->channel,GPIO_CFG1);
    data &= 0xF9;
    data |= 0x60;
    pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,GPIO_CFG1,data); /* coarse sensors analog mode */
    pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,PDN,0x80); /* madc powered in open state */
    pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,TXACHPF_A1_1,0x71EB851L); /* Fix HPF corner */

	setUserModeBroadcast(pProslic[0],FALSE);

    /*
	** Calibrate (madc offset)
    */
    Si3226_Calibrate(pProslic,size,calSetup,TIMEOUT_MADC_CAL);

    /*
	** Bring up DC/DC converters sequentially to minimize
    ** peak power demand on VDC
    */
	for (k=0;k<size;k++)
    { 
		if (pProslic[k]->channelEnable)
        {
			setUserMode(pProslic[k],TRUE);      
            pProslic[k]->error = Si3226_PowerUpConverter(pProslic[k]);
            setUserMode(pProslic[k],FALSE);
        }
    }

    /*
	** Execute remaining cals (except madc, lb)
    */
    calSetup[1] = CAL_STD_CALR1;
    calSetup[2] = CAL_STD_CALR2;
    Si3226_Calibrate(pProslic,size,calSetup,TIMEOUT_GEN_CAL);

    /*
    ** Enable Loopman
    */
	for (k=0;k<size;k++){ /*chan 0 shut off during chan 1 bring up*/
		if (pProslic[k]->channelEnable){
#ifdef DISABLE_LOOPMAN
#else
				enableSi3226Loopman(pProslic[k],FALSE);
#endif
		}
	}


	return 0;
}
int Si3226_PrintDebugData (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
		int i;
		for (i=0;i<99;i++)
			LOGPRINT ("Si3226 Register %d = %X\n",i,ReadReg(pProHW,pProslic->channel,i));
		for (i=0;i<1024;i++)
			LOGPRINT ("Si3226 RAM %d = %X\n",i,ReadRAM(pProHW,pProslic->channel,i));
#endif
		return 0;
}

/*
** Function: Si3226_LBCal
**
** Description: 
** Sequential canned longitudinal balance calibration. 
**
** Input Parameters: 
** pProslic: pointer to array of PROSLIC channel objects
** size:     number of PROSLIC channel objects   
**
** Return:
** 0
*/
int Si3226_LBCal (proslicChanType_ptr *pProslic, int size)
{
	int k,i=0;
	uInt8 lf; uInt8 data;
	int error=0;

	for (k=0;k<size;k++){
		if (pProslic[k]->channelEnable){
			lf = pProslic[k]->ReadRegX(pProslic[k]->pProHWX, pProslic[k]->channel,LINEFEED); 
			Si3226_SetLinefeedStatus(pProslic[k],LF_OPEN);
#ifdef DISABLE_LOOPMAN
#else
			disableSi3226Loopman(pProslic[k],FALSE);
#endif
#ifdef ENABLE_DEBUG
        if(pProslic[k]->debugMode)
        {
			LOGPRINT("Starting LB Cal on channel %d\n",pProslic[k]->channel);
        }
#endif

			Si3226_SetLinefeedStatus(pProslic[k],LF_FWD_ACTIVE);
			
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CALR0,CAL_LB_ALL); /*enable LB cal*/
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CALR3,0x80); /*start cal*/
			i=0;
			do {
				data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,CALR3);
				pProslic[k]->DelayX(pProslic[k]->pProTimerX,10);
				if (i++ >= TIMEOUT_LB_CAL){
#ifdef ENABLE_DEBUG
				if (pProslic[k]->debugMode)
					LOGPRINT("Calibration timeout channel %d\n",pProslic[k]->channel);
#endif
				pProslic[k]->error = RC_CAL_TIMEOUT;
				pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,LINEFEED,LF_OPEN); 
				return RC_CAL_TIMEOUT;
			
			}
			} while (data&0x80 && i<=TIMEOUT_LB_CAL);
#ifdef DISABLE_LOOPMAN
#else
			enableSi3226Loopman(pProslic[k],FALSE);
#endif
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,LINEFEED,lf); 
	
		}
	}
	return 0;
}



/*
** Function: Si3226_GetLBCalResult
**
** Description: 
** Read applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** resultx:  pointer to 4 RAM results
**
** Return:
** 0
*/
int Si3226_GetLBCalResult (proslicChanType *pProslic,int32*result1,int32*result2,int32*result3,int32*result4){
	setUserMode(pProslic,TRUE);
	*result1 = ReadRAM(pProHW,pProslic->channel,CMDAC_FWD);
	*result2 = ReadRAM(pProHW,pProslic->channel,CMDAC_REV);
	*result3 = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACT);
    *result4 = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACR);
	setUserMode(pProslic,FALSE);
	return 0;
}
/*
** Function: Si3226_GetLBCalResultPacked
**
** Description: 
** Read applicable calibration coefficients
** and pack into single 32bit word
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** result:   pointer to packed result
**
** Return:
** 0
**
** Packed Result Format
**
** Bits 31:24   CMDAC_FWD[25:18]
** Bits 23:16   CMDAC_REV[25:18]
** Bits 15:8    CAL_TRNRD_DACT[20:13]
** Bits 7:0     CAL_TRNRD_DACR[20:13]
*/
int Si3226_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result){
int32 tmpResult;
	setUserMode(pProslic,TRUE);
	tmpResult = ReadRAM(pProHW,pProslic->channel,CMDAC_FWD);
    *result = (tmpResult<<6)&0xff000000L;
	tmpResult = ReadRAM(pProHW,pProslic->channel,CMDAC_REV);
    *result |= (tmpResult>>1)&0x00ff0000L;
	tmpResult = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACT);
    *result |= (tmpResult>>5)&0x0000ff00L;
    tmpResult = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACR);
    *result |= (tmpResult>>13)&0x000000ffL;
	setUserMode(pProslic,FALSE);
	return 0;
}

/*
** Function: Si3226_LoadPreviousLBCal
**
** Description: 
** Load applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** resultx:  pointer to 4 RAM results
**
** Return:
** 0
*/
int Si3226_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2, int32 result3, int32 result4){
	setUserMode(pProslic,TRUE);
	WriteRAM(pProHW,pProslic->channel,CMDAC_FWD,result1);
	WriteRAM(pProHW,pProslic->channel,CMDAC_REV,result2);
	WriteRAM(pProHW,pProslic->channel,CAL_TRNRD_DACT,result3);
	WriteRAM(pProHW,pProslic->channel,CAL_TRNRD_DACR,result4);
	setUserMode(pProslic,FALSE);
	return 0;
}

/*
** Function: Si3226_LoadPreviousLBCalPacked
**
** Description: 
** Load applicable calibration coefficients
**
** Input Parameters: 
** pProslic: pointer to PROSLIC channel object
** result:   pointer to packed cal results
**
** Return:
** 0
*/
int Si3226_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result){
int32 ramVal;
	setUserMode(pProslic,TRUE);
    ramVal = (*result&0xff000000L)>>6;
	WriteRAM(pProHW,pProslic->channel,CMDAC_FWD,ramVal);
    ramVal = (*result&0x00ff0000L)<<1;
	WriteRAM(pProHW,pProslic->channel,CMDAC_REV,ramVal);
    ramVal = (*result&0x0000ff00L)<<5;
	WriteRAM(pProHW,pProslic->channel,CAL_TRNRD_DACT,ramVal);
    ramVal = (*result&0x000000ffL)<<13;
	WriteRAM(pProHW,pProslic->channel,CAL_TRNRD_DACR,ramVal);
#ifdef API_TEST
    ramVal = ReadRAM(pProHW,pProslic->channel,CMDAC_FWD);
    LOGPRINT ("UNPACKED CMDAC_FWD = %08x\n",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,CMDAC_REV);
    LOGPRINT ("UNPACKED CMDAC_REF = %08x\n",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACT);
    LOGPRINT ("UNPACKED CAL_TRNRD_DACT = %08x\n",ramVal);
    ramVal = ReadRAM(pProHW,pProslic->channel,CAL_TRNRD_DACR);
    LOGPRINT ("UNPACKED CAL_TRNRD_DACR = %08x\n",ramVal);
#endif
	setUserMode(pProslic,FALSE);
	return 0;
}


int Si3226_SetLinefeedStatusBroadcast (proslicChanType_ptr pProslic, uInt8 newLinefeed){
	WriteReg(pProHW,BROADCAST,LINEFEED,newLinefeed);
	return 0;
}

/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
*/
int Si3226_Cal (proslicChanType_ptr *pProslic, int size){
	/*
	** This function will perform the ProSLIC calibration sequence (for all channels)
	*/
	int i,j=0;
	for (i=0;i<size;i++){
		if (pProslic[i]->channelEnable){
#ifdef DISABLE_LOOPMAN
#else
			/* disableSi3226Loopman(pProslic[i],FALSE); */
#endif
			j |= calibrate(pProslic[i],0);
#ifdef DISABLE_LOOPMAN
#else
			/* enableSi3226Loopman(pProslic[i],FALSE); */
#endif
		}
	}
	return j;
}



/*
** Function: PROSLIC_LoadRegTables
**
** Description: 
** Loads registers and ram in the ProSLIC
*/
int Si3226_LoadRegTables (proslicChanType_ptr *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int size){
	uInt16 i;
	for (i=0;i<size;i++){
		if (pProslic[i]->channelEnable)
			LoadRegTables(pProslic[i],pRamTable,pRegTable,0);
	}
	return 0;
}


/*
** Function: PROSLIC_LoadPatch
**
** Description: 
** Loads patch to the ProSLIC
*/
int Si3226_LoadPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
	LoadSi3226Patch(pProslic,pPatch,0);
	return 0;
}

/*
** Function: PROSLIC_VerifyPatch
**
** Description: 
** Verifiess patch to the ProSLIC
*/
int Si3226_VerifyPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
	int loop, jmp_table=82;
	uInt8 data; uInt32 ramdata;
	int err = 0;
	if (pPatch == NULL)
		return 0;
	setUserMode (pProslic,TRUE); /*make sure we are in user mode to read patch*/

	WriteReg (pProHW, pProslic->channel, JMPEN,0); /*disable the patch*/

	WriteRAM(pProHW, pProslic->channel,PRAM_ADDR, 0); /*write patch ram address register*/
	
	/* If the data is all 0, you have hit the end of the programmed values and can stop loading.*/
    for (loop=0; loop<1024; loop++){
		if (pPatch->patchData[loop] != 0){
			ramdata = ReadRAM(pProHW, pProslic->channel,PRAM_DATA); /*note. data is shifted*/
			if (pPatch->patchData[loop]<<9 != ramdata){
				loop = 1024;			
				err = 1;
			}
		}
        else
            loop = 1024;
    }
	
	/*zero out RAM_ADDR_HI*/
	WriteReg (pProHW, pProslic->channel, RAM_ADDR_HI,0);

    
	jmp_table=82;
	for (loop=0;loop<8;loop++){
		/* check the jump table with the new values.*/
		if (pPatch->patchEntries[loop] != 0){
			data = ReadReg (pProHW, pProslic->channel, jmp_table);
			if (data != ((pPatch->patchEntries[loop])&0xff))
				err = 1;
			data = ReadReg (pProHW, pProslic->channel, jmp_table+1);
			if (data != (pPatch->patchEntries[loop]>>8))
				err = 1;
		}
		jmp_table+=2;
	}
 
	
	
	if (err){
#ifdef ENABLE_DEBUG
		if (pProslic->debugMode)
			LOGPRINT("Si3226 Patch data corrupted: channel %d\n",pProslic->channel);
#endif
	}
	else {
		WriteReg (pProHW, pProslic->channel, JMPEN,1); /*enable the patch*/
	}
	setUserMode(pProslic,FALSE); /*turn off user mode*/
	return err;
}


/*
** Function: PROSLIC_EnableInterrupts
**
** Description: 
** Enables interrupts
*/
int Si3226_EnableInterrupts (proslicChanType_ptr pProslic){
	WriteReg (pProHW,pProslic->channel,IRQEN1,Si3226_General_Configuration.irqen1);
	WriteReg (pProHW,pProslic->channel,IRQEN2,Si3226_General_Configuration.irqen2);
	WriteReg (pProHW,pProslic->channel,IRQEN3,Si3226_General_Configuration.irqen3);
	WriteReg (pProHW,pProslic->channel,IRQEN4,Si3226_General_Configuration.irqen4);
	return 0;
}

/*
** Function: PROSLIC_SetLoopbackMode
**
** Description: 
** Program loopback mode
*/
int Si3226_SetLoopbackMode (proslicChanType_ptr pProslic, ProslicLoopbackModes newMode){
	uInt8 regTemp;
	regTemp = ReadReg (pProHW,pProslic->channel,LOOPBACK);
	switch (newMode){
		case PROSLIC_LOOPBACK_NONE:
			WriteReg (pProHW,pProslic->channel,LOOPBACK,regTemp&~(0x11));
			break;
		case PROSLIC_LOOPBACK_DIG:
			WriteReg (pProHW,pProslic->channel,LOOPBACK,regTemp|(0x1));
			break;
		case PROSLIC_LOOPBACK_ANA:
			WriteReg (pProHW,pProslic->channel,LOOPBACK,regTemp|(0x10));
			break;
	}
	return 0;
}


/*
** Function: PROSLIC_SetMuteStatus
**
** Description: 
** configure RX and TX path mutes
*/
int Si3226_SetMuteStatus (proslicChanType_ptr pProslic, ProslicMuteModes muteEn){
	uInt8 regTemp;
    uInt8 newRegValue;

	regTemp = ReadReg (pProHW,pProslic->channel,DIGCON);
	
	WriteReg (pProHW,pProslic->channel,DIGCON,regTemp&~(0x3));
    newRegValue = regTemp &~(0x3);
	
	if (muteEn & PROSLIC_MUTE_RX){
        newRegValue |= 1;
	}
	if (muteEn & PROSLIC_MUTE_TX){
        newRegValue |= 2;
	}

    if(newRegValue != regTemp)
    {
		WriteReg (pProHW,pProslic->channel,DIGCON,newRegValue);
    }

	return 0;
}

/*
**
** PROSLIC CONFIGURATION FUNCTIONS
**
** These functions configure the ProSLIC
** referencing extern conifiguration structures
** generated by the ProSLIC API Config Tool
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
extern Si3226_Ring_Cfg Si3226_Ring_Presets[];
int Si3226_RingSetup (proslicChanType *pProslic, int preset){

    WriteRAM(pProHW,pProslic->channel,RTPER,Si3226_Ring_Presets[preset].rtper);
    WriteRAM(pProHW,pProslic->channel,RINGFR,Si3226_Ring_Presets[preset].freq);
    WriteRAM(pProHW,pProslic->channel,RINGAMP,Si3226_Ring_Presets[preset].amp);
    WriteRAM(pProHW,pProslic->channel,RINGPHAS,Si3226_Ring_Presets[preset].phas);
    WriteRAM(pProHW,pProslic->channel,RINGOF,Si3226_Ring_Presets[preset].offset);
    WriteRAM(pProHW,pProslic->channel,SLOPE_RING,Si3226_Ring_Presets[preset].slope_ring);
    WriteRAM(pProHW,pProslic->channel,IRING_LIM,Si3226_Ring_Presets[preset].iring_lim);
    WriteRAM(pProHW,pProslic->channel,RTACTH,Si3226_Ring_Presets[preset].rtacth);
    WriteRAM(pProHW,pProslic->channel,RTDCTH,Si3226_Ring_Presets[preset].rtdcth);
    WriteRAM(pProHW,pProslic->channel,RTACDB,Si3226_Ring_Presets[preset].rtacdb);
    WriteRAM(pProHW,pProslic->channel,RTDCDB,Si3226_Ring_Presets[preset].rtdcdb);
    WriteRAM(pProHW,pProslic->channel,VOV_RING_BAT,Si3226_Ring_Presets[preset].vov_ring_bat);
    WriteRAM(pProHW,pProslic->channel,VOV_RING_GND,Si3226_Ring_Presets[preset].vov_ring_gnd);
    WriteRAM(pProHW,pProslic->channel,VBATR_EXPECT,Si3226_Ring_Presets[preset].vbatr_expect);
    WriteReg(pProHW,pProslic->channel,RINGTALO,Si3226_Ring_Presets[preset].talo);
    WriteReg(pProHW,pProslic->channel,RINGTAHI,Si3226_Ring_Presets[preset].tahi);
    WriteReg(pProHW,pProslic->channel,RINGTILO,Si3226_Ring_Presets[preset].tilo);
    WriteReg(pProHW,pProslic->channel,RINGTIHI,Si3226_Ring_Presets[preset].tihi);
  
    WriteRAM(pProHW,pProslic->channel,DCDC_VREF_MIN_RNG,Si3226_Ring_Presets[preset].dcdc_vref_min_rng);
    WriteReg(pProHW,pProslic->channel,RINGCON,Si3226_Ring_Presets[preset].ringcon);
    WriteReg(pProHW,pProslic->channel,USERSTAT,Si3226_Ring_Presets[preset].userstat);
    WriteRAM(pProHW,pProslic->channel,VCM_RING,Si3226_Ring_Presets[preset].vcm_ring);
    WriteRAM(pProHW,pProslic->channel,VCM_RING_FIXED,Si3226_Ring_Presets[preset].vcm_ring_fixed);
    WriteRAM(pProHW,pProslic->channel,DELTA_VCM,Si3226_Ring_Presets[preset].delta_vcm);

    setUserMode(pProslic,TRUE);
    WriteRAM(pProHW,pProslic->channel,DCDC_RNGTYPE,Si3226_Ring_Presets[preset].dcdc_rngtype);
    setUserMode(pProslic,FALSE);
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
extern Si3226_Tone_Cfg Si3226_Tone_Presets[];
int Si3226_ToneGenSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,OSC1FREQ,Si3226_Tone_Presets[preset].osc1.freq);
	WriteRAM(pProHW,pProslic->channel,OSC1AMP,Si3226_Tone_Presets[preset].osc1.amp);
	WriteRAM(pProHW,pProslic->channel,OSC1PHAS,Si3226_Tone_Presets[preset].osc1.phas);
	WriteReg(pProHW,pProslic->channel,O1TAHI,(Si3226_Tone_Presets[preset].osc1.tahi));
	WriteReg(pProHW,pProslic->channel,O1TALO,(Si3226_Tone_Presets[preset].osc1.talo));
	WriteReg(pProHW,pProslic->channel,O1TIHI,(Si3226_Tone_Presets[preset].osc1.tihi));
	WriteReg(pProHW,pProslic->channel,O1TILO,(Si3226_Tone_Presets[preset].osc1.tilo));
	WriteRAM(pProHW,pProslic->channel,OSC2FREQ,Si3226_Tone_Presets[preset].osc2.freq);
	WriteRAM(pProHW,pProslic->channel,OSC2AMP,Si3226_Tone_Presets[preset].osc2.amp);
	WriteRAM(pProHW,pProslic->channel,OSC2PHAS,Si3226_Tone_Presets[preset].osc2.phas);
	WriteReg(pProHW,pProslic->channel,O2TAHI,(Si3226_Tone_Presets[preset].osc2.tahi));
	WriteReg(pProHW,pProslic->channel,O2TALO,(Si3226_Tone_Presets[preset].osc2.talo));
	WriteReg(pProHW,pProslic->channel,O2TIHI,(Si3226_Tone_Presets[preset].osc2.tihi));
	WriteReg(pProHW,pProslic->channel,O2TILO,(Si3226_Tone_Presets[preset].osc2.tilo));
	WriteReg(pProHW,pProslic->channel,OMODE,(Si3226_Tone_Presets[preset].omode));
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
extern Si3226_FSK_Cfg Si3226_FSK_Presets[];
int Si3226_FSKSetup (proslicChanType *pProslic, int preset){
	uInt8 data; 


	WriteReg(pProHW,pProslic->channel,O1TAHI,0);
	WriteReg(pProHW,pProslic->channel,O1TIHI,0);
	WriteReg(pProHW,pProslic->channel,O1TILO,0);
	WriteReg(pProHW,pProslic->channel,O1TALO,0x13);

	data = ReadReg(pProHW,pProslic->channel,OMODE);
	if (Si3226_FSK_Presets[preset].eightBit)
		data |= 0x80;
	else 
		data &= ~(0x80);
	WriteReg(pProHW,pProslic->channel,FSKDEPTH,Si3226_FSK_Presets[preset].fskdepth);
	WriteReg(pProHW,pProslic->channel,OMODE,data);
	WriteRAM(pProHW,pProslic->channel,FSK01,Si3226_FSK_Presets[preset].fsk01);
	WriteRAM(pProHW,pProslic->channel,FSK10,Si3226_FSK_Presets[preset].fsk10);
	WriteRAM(pProHW,pProslic->channel,FSKAMP0,Si3226_FSK_Presets[preset].fskamp0);
	WriteRAM(pProHW,pProslic->channel,FSKAMP1,Si3226_FSK_Presets[preset].fskamp1);
	WriteRAM(pProHW,pProslic->channel,FSKFREQ0,Si3226_FSK_Presets[preset].fskfreq0);
	WriteRAM(pProHW,pProslic->channel,FSKFREQ1,Si3226_FSK_Presets[preset].fskfreq1);
	return 0;
}
#endif

int Si3226_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *fsk_buf_avail){
	uInt8 data;
	data = ReadReg(pProHW,pProslic->channel,IRQ1);
	WriteReg(pProHW,pProslic->channel,IRQ1,data); /*clear (for GCI)*/
	*fsk_buf_avail = (data&0x40) ? 1 : 0;
	return 0;
}
/*
** Function: PROSLIC_DTMFDecodeSetup
**
** Description: 
** configure dtmf decode
*/
#ifdef DISABLE_DTMF_SETUP
#else
extern Si3226_DTMFDec_Cfg Si3226_DTMFDec_Presets[];
int Si3226_DTMFDecodeSetup (proslicChanType *pProslic, int preset){
	
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_1,Si3226_DTMFDec_Presets[preset].dtmfdtf_b0_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_1,Si3226_DTMFDec_Presets[preset].dtmfdtf_b1_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_1,Si3226_DTMFDec_Presets[preset].dtmfdtf_b2_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_1,Si3226_DTMFDec_Presets[preset].dtmfdtf_a1_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_1,Si3226_DTMFDec_Presets[preset].dtmfdtf_a2_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_2,Si3226_DTMFDec_Presets[preset].dtmfdtf_b0_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_2,Si3226_DTMFDec_Presets[preset].dtmfdtf_b1_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_2,Si3226_DTMFDec_Presets[preset].dtmfdtf_b2_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_2,Si3226_DTMFDec_Presets[preset].dtmfdtf_a1_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_2,Si3226_DTMFDec_Presets[preset].dtmfdtf_a2_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_3,Si3226_DTMFDec_Presets[preset].dtmfdtf_b0_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_3,Si3226_DTMFDec_Presets[preset].dtmfdtf_b1_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_3,Si3226_DTMFDec_Presets[preset].dtmfdtf_b2_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_3,Si3226_DTMFDec_Presets[preset].dtmfdtf_a1_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_3,Si3226_DTMFDec_Presets[preset].dtmfdtf_a2_3);
	return 0;
}
#endif
/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic, which links presets
** from multiple functional blocks to a single country
** profile.
*/
int Si3226_SetProfile (proslicChanType *pProslic, int preset){
	/* 
	** TODO:  Add functionality to API Config Tool to create
	**        country linkage structures.
	*/
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
extern Si3226_Impedance_Cfg Si3226_Impedance_Presets [];
int Si3226_ZsynthSetup (proslicChanType *pProslic, int preset){
	uInt8 lf;
    uInt8 cal_en = 0;
    uInt8 timer = 500;

	lf = ReadReg(pProHW,pProslic->channel,LINEFEED);
	WriteReg(pProHW,pProslic->channel,LINEFEED,0);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C0,Si3226_Impedance_Presets[preset].audioEQ.txaceq_c0);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C1,Si3226_Impedance_Presets[preset].audioEQ.txaceq_c1);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C2,Si3226_Impedance_Presets[preset].audioEQ.txaceq_c2);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C3,Si3226_Impedance_Presets[preset].audioEQ.txaceq_c3);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C0,Si3226_Impedance_Presets[preset].audioEQ.rxaceq_c0);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C1,Si3226_Impedance_Presets[preset].audioEQ.rxaceq_c1);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C2,Si3226_Impedance_Presets[preset].audioEQ.rxaceq_c2);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C3,Si3226_Impedance_Presets[preset].audioEQ.rxaceq_c3);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C2,Si3226_Impedance_Presets[preset].hybrid.ecfir_c2);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C3,Si3226_Impedance_Presets[preset].hybrid.ecfir_c3);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C4,Si3226_Impedance_Presets[preset].hybrid.ecfir_c4);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C5,Si3226_Impedance_Presets[preset].hybrid.ecfir_c5);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C6,Si3226_Impedance_Presets[preset].hybrid.ecfir_c6);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C7,Si3226_Impedance_Presets[preset].hybrid.ecfir_c7);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C8,Si3226_Impedance_Presets[preset].hybrid.ecfir_c8);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C9,Si3226_Impedance_Presets[preset].hybrid.ecfir_c9);
	WriteRAM(pProHW,pProslic->channel,ECIIR_B0,Si3226_Impedance_Presets[preset].hybrid.ecfir_b0);
	WriteRAM(pProHW,pProslic->channel,ECIIR_B1,Si3226_Impedance_Presets[preset].hybrid.ecfir_b1);
	WriteRAM(pProHW,pProslic->channel,ECIIR_A1,Si3226_Impedance_Presets[preset].hybrid.ecfir_a1);
	WriteRAM(pProHW,pProslic->channel,ECIIR_A2,Si3226_Impedance_Presets[preset].hybrid.ecfir_a2);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A1,Si3226_Impedance_Presets[preset].zsynth.zsynth_a1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A2,Si3226_Impedance_Presets[preset].zsynth.zsynth_a2);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B1,Si3226_Impedance_Presets[preset].zsynth.zsynth_b1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B0,Si3226_Impedance_Presets[preset].zsynth.zsynth_b0);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B2,Si3226_Impedance_Presets[preset].zsynth.zsynth_b2);
	WriteReg(pProHW,pProslic->channel,RA,Si3226_Impedance_Presets[preset].zsynth.ra);
	WriteRAM(pProHW,pProslic->channel,TXACGAIN,Si3226_Impedance_Presets[preset].txgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN,Si3226_Impedance_Presets[preset].rxgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN_SAVE,Si3226_Impedance_Presets[preset].rxgain);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_B0_1,Si3226_Impedance_Presets[preset].rxachpf_b0_1);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_B1_1,Si3226_Impedance_Presets[preset].rxachpf_b1_1);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_A1_1,Si3226_Impedance_Presets[preset].rxachpf_a1_1);

    /* Perform Zcal in case OHT used (eg. no offhook event to trigger auto Zcal) */
 	WriteReg(pProHW,pProslic->channel,CALR0,0x00);   
 	WriteReg(pProHW,pProslic->channel,CALR1,0x40);   
 	WriteReg(pProHW,pProslic->channel,CALR2,0x00); 
 	WriteReg(pProHW,pProslic->channel,CALR3,0x80);  /* start cal */

    /* Wait for zcal to finish */
    do {
        cal_en = ReadReg(pProHW,pProslic->channel,CALR3);
        Delay(pProTimer,1);
        timer--;
    }while((cal_en&0x80)&&(timer>0));  
     
	WriteReg(pProHW,pProslic->channel,LINEFEED,lf);

    if(timer > 0) return 0;
    else          return RC_CAL_TIMEOUT;

}
#endif

/*
** Function: PROSLIC_GciCISetup
**
** Description: 
** configure CI bits (GCI mode)
*/
#ifdef DISABLE_CI_SETUP
#else
extern Si3226_CI_Cfg Si3226_CI_Presets [];
int Si3226_GciCISetup (proslicChanType *pProslic, int preset){
	WriteReg(pProHW,pProslic->channel,GCI_CI,Si3226_CI_Presets[preset].gci_ci);
	return 0;
}
#endif
/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
*/
int Si3226_ModemDetSetup (proslicChanType *pProslic, int preset){
	/*TO DO
	Will be filled in at a later date*/
	return 0;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
#ifdef DISABLE_AUDIOGAIN_SETUP
#else
extern Si3226_audioGain_Cfg Si3226_audioGain_Presets[];
int Si3226_TXAudioGainSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,TXACGAIN,Si3226_audioGain_Presets[preset].acgain);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C0,Si3226_audioGain_Presets[preset].aceq_c0);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C1,Si3226_audioGain_Presets[preset].aceq_c1);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C2,Si3226_audioGain_Presets[preset].aceq_c2);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C3,Si3226_audioGain_Presets[preset].aceq_c3);
	return 0;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si3226_RXAudioGainSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,RXACGAIN,Si3226_audioGain_Presets[preset].acgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN_SAVE,Si3226_audioGain_Presets[preset].acgain);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C0,Si3226_audioGain_Presets[preset].aceq_c0);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C1,Si3226_audioGain_Presets[preset].aceq_c1);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C2,Si3226_audioGain_Presets[preset].aceq_c2);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C3,Si3226_audioGain_Presets[preset].aceq_c3);
	return 0;
}
#endif




/*
** Function: PROSLIC_DCFeedSetup
**
** Description: 
** configure dc feed
*/
#ifdef DISABLE_DCFEED_SETUP
#else
extern Si3226_DCfeed_Cfg Si3226_DCfeed_Presets[];
int Si3226_DCFeedSetup (proslicChanType *pProslic, int preset){
	uInt8 lf;
	lf = ReadReg(pProHW,pProslic->channel,LINEFEED);
	WriteReg(pProHW,pProslic->channel,LINEFEED,0);
	WriteRAM(pProHW,pProslic->channel,SLOPE_VLIM,Si3226_DCfeed_Presets[preset].slope_vlim);
	WriteRAM(pProHW,pProslic->channel,SLOPE_RFEED,Si3226_DCfeed_Presets[preset].slope_rfeed);
	WriteRAM(pProHW,pProslic->channel,SLOPE_ILIM,Si3226_DCfeed_Presets[preset].slope_ilim);
	WriteRAM(pProHW,pProslic->channel,SLOPE_DELTA1,Si3226_DCfeed_Presets[preset].delta1);
	WriteRAM(pProHW,pProslic->channel,SLOPE_DELTA2,Si3226_DCfeed_Presets[preset].delta2);
	WriteRAM(pProHW,pProslic->channel,V_VLIM,Si3226_DCfeed_Presets[preset].v_vlim);
	WriteRAM(pProHW,pProslic->channel,V_RFEED,Si3226_DCfeed_Presets[preset].v_rfeed);
	WriteRAM(pProHW,pProslic->channel,V_ILIM,Si3226_DCfeed_Presets[preset].v_ilim);
	WriteRAM(pProHW,pProslic->channel,CONST_RFEED,Si3226_DCfeed_Presets[preset].const_rfeed);
	WriteRAM(pProHW,pProslic->channel,CONST_ILIM,Si3226_DCfeed_Presets[preset].const_ilim);
	WriteRAM(pProHW,pProslic->channel,I_VLIM,Si3226_DCfeed_Presets[preset].i_vlim);
	WriteRAM(pProHW,pProslic->channel,LCRONHK,Si3226_DCfeed_Presets[preset].lcronhk);
	WriteRAM(pProHW,pProslic->channel,LCROFFHK,Si3226_DCfeed_Presets[preset].lcroffhk);
	WriteRAM(pProHW,pProslic->channel,LCRDBI,Si3226_DCfeed_Presets[preset].lcrdbi);
	WriteRAM(pProHW,pProslic->channel,LONGHITH,Si3226_DCfeed_Presets[preset].longhith);
	WriteRAM(pProHW,pProslic->channel,LONGLOTH,Si3226_DCfeed_Presets[preset].longloth);
	WriteRAM(pProHW,pProslic->channel,LONGDBI,Si3226_DCfeed_Presets[preset].longdbi);
	WriteRAM(pProHW,pProslic->channel,LCRMASK,Si3226_DCfeed_Presets[preset].lcrmask);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_POLREV,Si3226_DCfeed_Presets[preset].lcrmask_polrev);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_STATE,Si3226_DCfeed_Presets[preset].lcrmask_state);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_LINECAP,Si3226_DCfeed_Presets[preset].lcrmask_linecap);
	WriteRAM(pProHW,pProslic->channel,VCM_OH,Si3226_DCfeed_Presets[preset].vcm_oh);
	WriteRAM(pProHW,pProslic->channel,VOV_BAT,Si3226_DCfeed_Presets[preset].vov_bat);
	WriteRAM(pProHW,pProslic->channel,VOV_GND,Si3226_DCfeed_Presets[preset].vov_gnd);
	WriteReg(pProHW,pProslic->channel,LINEFEED,lf);
	return 0;
}
#endif
/*
** Function: PROSLIC_GPIOSetup
**
** Description: 
** configure gpio
*/
#ifdef DISABLE_GPIO_SETUP
#else
extern Si3226_GPIO_Cfg Si3226_GPIO_Configuration ;
int Si3226_GPIOSetup (proslicChanType *pProslic){
	uInt8 data;
	data = ReadReg(pProHW,pProslic->channel,GPIO);
	data |= Si3226_GPIO_Configuration.outputEn << 4;
	WriteReg(pProHW,pProslic->channel,GPIO,data);
	data = Si3226_GPIO_Configuration.analog << 4;
	data |= Si3226_GPIO_Configuration.direction;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG1,data);
	data = Si3226_GPIO_Configuration.manual << 4;
	data |= Si3226_GPIO_Configuration.polarity;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG2,data);
	data |= Si3226_GPIO_Configuration.openDrain;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG3,data);
	WriteReg(pProHW,pProslic->channel,BATSELMAP,Si3226_GPIO_Configuration.batselmap);
	return 0;
}
#endif

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
int Si3226_PulseMeterSetup (proslicChanType *pProslic, int preset){
	/*not applicable to this part number*/
	return 0;
}

/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
#ifdef DISABLE_PCM_SETUP
#else
extern Si3226_PCM_Cfg Si3226_PCM_Presets [];
int Si3226_PCMSetup (proslicChanType *pProslic, int preset){
	uInt8 regTemp;
	
	
    /* TODO:  Remove hardcoded coefficients for wideband mode */
	if (Si3226_PCM_Presets[preset].widebandEn){
		regTemp = ReadReg(pProHW,pProslic->channel,DIGCON);
		WriteReg(pProHW,pProslic->channel,DIGCON,regTemp|0xC);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_1,0x27EA83L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_1,0x27EA83L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_1,0x487977EL);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_2,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_2,0x7E8704DL);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B2_2,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_2,0x368C302L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A2_2,0x18EBB1A4L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_3,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_3,0x254C75AL);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B2_3,0x7FFFFFFL);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_3,0x639A165L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A2_3,0x1B6738A0L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_1,0x4FD507L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_1,0x4FD507L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_1,0x487977EL);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_2,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_2,0x7E8704DL);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B2_2,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_2,0x368C302L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A2_2,0x18EBB1A4L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_3,0x8000000L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_3,0x254C75AL);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B2_3,0x7FFFFFFL);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_3,0x639A165L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A2_3,0x1B6738A0L);
		regTemp = ReadReg(pProHW,pProslic->channel,ENHANCE);
		WriteReg(pProHW,pProslic->channel,ENHANCE,regTemp|1);
	} else {
		regTemp = ReadReg(pProHW,pProslic->channel,DIGCON);
		WriteReg(pProHW,pProslic->channel,DIGCON,regTemp&~(0xC));
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_1,0x3538E80L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_1,0x3538E80L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_1,0x1AA9100L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_2,0x216D100L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_2,0x2505400L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B2_2,0x216D100L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_2,0x2CB8100L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A2_2,0x1D7FA500L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B0_3,0x2CD9B00L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B1_3,0x1276D00L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_B2_3,0x2CD9B00L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A1_3,0x2335300L);
		WriteRAM(pProHW,pProslic->channel,TXACIIR_A2_3,0x19D5F700L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_1,0x6A71D00L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_1,0x6A71D00L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_1,0x1AA9100L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_2,0x216D100L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_2,0x2505400L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B2_2,0x216D100L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_2,0x2CB8100L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A2_2,0x1D7FA500L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B0_3,0x2CD9B00L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B1_3,0x1276D00L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_B2_3,0x2CD9B00L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A1_3,0x2335300L);
		WriteRAM(pProHW,pProslic->channel,RXACIIR_A2_3,0x19D5F700L);
		regTemp = ReadReg(pProHW,pProslic->channel,ENHANCE);
		WriteReg(pProHW,pProslic->channel,ENHANCE,regTemp&~(1));
	}

    /*
    ** Only update format and control options
    ** Enable/Disable and timeslots handled 
    ** with API control functions
    */

	regTemp = Si3226_PCM_Presets[preset].pcmFormat;
	regTemp |= Si3226_PCM_Presets[preset].pcm_tri << 5;	
	WriteReg(pProHW,pProslic->channel,PCMMODE,regTemp);
	regTemp = ReadReg(pProHW,pProslic->channel,PCMTXHI);
	regTemp &= 3;
	regTemp |= Si3226_PCM_Presets[preset].tx_edge<<4;
	WriteReg(pProHW,pProslic->channel,PCMTXHI,regTemp);

	return 0;
}
#endif
/*
** Function: PROSLIC_PCMSetup
**
** Description: 
** configure pcm
*/
int Si3226_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount){
	uInt8 data;
	data = txcount & 0xff;
	WriteReg(pProHW,pProslic->channel,PCMTXLO,data);
    data = ReadReg(pProHW,pProslic->channel,PCMTXHI);
    data &= 0x10;  /* keep TX_EDGE bit */
	data |= ((txcount >> 8)&0x03) ;
	WriteReg(pProHW,pProslic->channel,PCMTXHI,data);
	data = rxcount & 0xff;
	WriteReg(pProHW,pProslic->channel,PCMRXLO,data);
	data = rxcount >> 8 ;
	WriteReg(pProHW,pProslic->channel,PCMRXHI,data);

	return 0;
}

/*
**
** PROSLIC CONTROL FUNCTIONS
**
*/



/*
** Function: PROSLIC_GetInterrupts
**
** Description: 
** Reads interrupt registers status (IRQ1-4)
*/
int Si3226_GetInterrupts (proslicChanType *pProslic,proslicIntType *pIntData){
	/*Reading the interrupt registers and will clear any bits which are set (SPI mode only)
	Multiple interrupts may occur at once so bear that in mind when
	writing an interrupt handling routine*/
	uInt8 data[4];
	int i,j,k;
	pIntData->number = 0;
	
	data[0] = ReadReg(pProHW,pProslic->channel,IRQ1);
	data[1] = ReadReg(pProHW,pProslic->channel,IRQ2);
	data[2] = ReadReg(pProHW,pProslic->channel,IRQ3);
	data[3] = ReadReg(pProHW,pProslic->channel,IRQ4);
#ifdef GCI_MODE
	WriteReg(pProHW,pProslic->channel,IRQ1,data[0]); /*clear interrupts (gci only)*/
	WriteReg(pProHW,pProslic->channel,IRQ2,data[1]);
	WriteReg(pProHW,pProslic->channel,IRQ3,data[2]);
	WriteReg(pProHW,pProslic->channel,IRQ4,data[3]);
#endif
		for (i=0;i<4;i++){
		for (j=0;j<8;j++){
			if (data[i]&(1<<j)){
				switch (j + (i*8)){
                    /* IRQ 1 */
					case IRQ_OSC1_T1_SI3226:   /* IRQ1.0 */
						k=IRQ_OSC1_T1;
						break;
					case IRQ_OSC1_T2_SI3226:   /* IRQ1.1 */
						k=IRQ_OSC1_T2;
						break;
					case IRQ_OSC2_T1_SI3226:   /* IRQ1.2 */
						k=IRQ_OSC2_T1;
						break;
					case IRQ_OSC2_T2_SI3226:   /* IRQ1.3 */
						k=IRQ_OSC2_T2;
						break;
					case IRQ_RING_T1_SI3226:   /* IRQ1.4 */
						k=IRQ_RING_T1;
						break;
					case IRQ_RING_T2_SI3226:   /* IRQ1.5 */
						k=IRQ_RING_T2;
						break;
					case IRQ_FSKBUF_AVAIL_SI3226:/* IRQ1.6 */
						k=IRQ_FSKBUF_AVAIL;
						break;
					case IRQ_VBAT_SI3226:      /* IRQ1.7 */
						k=IRQ_VBAT;
						break;
                    /* IRQ2 */
					case IRQ_RING_TRIP_SI3226: /* IRQ2.0 */
						k=IRQ_RING_TRIP;
						break;
					case IRQ_LOOP_STAT_SI3226: /* IRQ2.1 */
						k=IRQ_LOOP_STATUS;
						break;
					case IRQ_LONG_STAT_SI3226: /* IRQ2.2 */
						k=IRQ_LONG_STAT;
						break;
					case IRQ_VOC_TRACK_SI3226: /* IRQ2.3 */
						k=IRQ_VOC_TRACK;
						break;
					case IRQ_DTMF_SI3226:      /* IRQ2.4 */
						k=IRQ_DTMF;
						break;
					case IRQ_INDIRECT_SI3226:  /* IRQ2.5 */
						k=IRQ_INDIRECT;
						break;
					case IRQ_TXMDM_SI3226:     /* IRQ2.6 */
						k = IRQ_TXMDM;
						break;
					case IRQ_RXMDM_SI3226:     /* IRQ2.7 */
						k=IRQ_RXMDM;
						break;
                    /* IRQ3 */
					case IRQ_P_HVIC_SI3226:       /* IRQ3.0 */
						k=IRQ_P_HVIC;
						break;
					case IRQ_P_THERM_SI3226:       /* IRQ3.1 */
						k=IRQ_P_THERM;
						break;
					case IRQ_PQ3_SI3226:       /* IRQ3.2 */
						k=IRQ_PQ3;  
						break;
					case IRQ_PQ4_SI3226:       /* IRQ3.3 */
						k=IRQ_PQ4;
						break;
					case IRQ_PQ5_SI3226:       /* IRQ3.4 */
						k=IRQ_PQ5;
						break;
					case IRQ_PQ6_SI3226:       /* IRQ3.5 */
						k=IRQ_PQ6;
						break;
					case IRQ_DSP_SI3226:       /* IRQ3.6 */
						k=IRQ_DSP;
						break;
					case IRQ_MADC_FS_SI3226:       /* IRQ3.7 */
						k=IRQ_MADC_FS;
						break;
                    /* IRQ4 */
					case IRQ_USER_0_SI3226: /* IRQ4.0 */
						k=IRQ_USER_0;
						break;
					case IRQ_USER_1_SI3226: /* IRQ4.1 */
						k=IRQ_USER_1;
						break;
					case IRQ_USER_2_SI3226: /* IRQ4.2 */
						k=IRQ_USER_2;
						break;
					case IRQ_USER_3_SI3226: /* IRQ4.3 */
						k=IRQ_USER_3;
						break;
					case IRQ_USER_4_SI3226: /* IRQ4.4 */
						k=IRQ_USER_4;
						break;
					case IRQ_USER_5_SI3226: /* IRQ4.5 */
						k=IRQ_USER_5;
						break;
					case IRQ_USER_6_SI3226: /* IRQ4.6 */
						k=IRQ_USER_6;
						break;
					case IRQ_USER_7_SI3226: /* IRQ4.7 */
						k=IRQ_USER_7;
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
int Si3226_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat){
	if (ReadReg(pProHW,pProslic->channel,LCRRTP) & 2)
		*pHookStat=OFFHOOK;
	else
		*pHookStat=ONHOOK;
	return 0;
}

/*
** Function: PROSLIC_SetLinefeedStatus
**
** Description: 
** Sets linefeed state
*/
int Si3226_SetLinefeedStatus (proslicChanType *pProslic,uInt8 newLinefeed){
	WriteReg (pProHW, pProslic->channel, LINEFEED,newLinefeed);
	return 0;
}

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
*/
int Si3226_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState){
	uInt8 data=0;
	switch (newPolRevState){
		case POLREV_STOP:
			data = 0;
			break;
		case POLREV_START:
			data = 2;
			break;
		case WINK_START:
			data = 6;
			break;
		case WINK_STOP:
			data = 4;
			break;
	}
	if (abrupt)
		data |= 1;
	WriteReg(pProHW,pProslic->channel,POLREV,data);
	
	return 0;
}

/*
** Function: PROSLIC_GPIOControl
**
** Description: 
** Sets gpio of the proslic
*/
int Si3226_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read){
	if (read)
		*pGpioData = 0xf & ReadReg(pProHW,pProslic->channel,GPIO);
	else{
		WriteReg(pProHW,pProslic->channel,GPIO,(*pGpioData)|(ReadReg(pProHW,pProslic->channel,GPIO)&0xf0));
	}
	return 0;
}

/*
** Function: PROSLIC_MWI
**
** Description: 
** implements message waiting indicator
*/
int Si3226_MWI (proslicChanType *pProslic,uInt8 lampOn){
	/*message waiting (neon flashing) requires modifications to vbath_expect and slope_vlim.
	The old values are restored to turn off the lamp. We assume all channels set up the same.
	During off-hook event lamp must be disabled manually. */
	static int32 vbath_save = 0;
	static int32 slope_vlim_save = 0;
	uInt8 hkStat; int32 slope_vlim_tmp;
	slope_vlim_tmp = ReadRAM(pProHW,pProslic->channel,SLOPE_VLIM);
	Si3226_ReadHookStatus(pProslic,&hkStat);

	if (lampOn && (hkStat == OFFHOOK) ) {/*cant neon flash during offhook*/
#ifdef ENABLE_DEBUG
		if (pProslic->debugMode)	
			LOGPRINT ("Si3226 MWI cannot operate offhook\n");
#endif
		return 1;
	}

	if (lampOn) {
		if (slope_vlim_tmp != 0x8000000L) { /*check we're not already on*/
			vbath_save = ReadRAM(pProHW,pProslic->channel,VBATH_EXPECT);
			slope_vlim_save = slope_vlim_tmp;
		}
		WriteRAM(pProHW,pProslic->channel,VBATH_EXPECT,0x7AE147AL);/*120V*/
		WriteRAM(pProHW,pProslic->channel,SLOPE_VLIM,0x8000000L);
	} else {
		if (vbath_save != 0) { /*check we saved some valid value first*/
			WriteRAM(pProHW,pProslic->channel,VBATH_EXPECT,vbath_save);
			WriteRAM(pProHW,pProslic->channel,SLOPE_VLIM,slope_vlim_save);
		}
	}

	return 0;
}

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** start tone generators
*/
int Si3226_ToneGenStart (proslicChanType *pProslic,uInt8 timerEn){
	uInt8 data;
	data = ReadReg(pProHW,pProslic->channel,OCON);
	data |= 0x11 + (timerEn ? 0x66 : 0);
	WriteReg(pProHW,pProslic->channel,OCON,data);
	return 0;
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
int Si3226_ToneGenStop (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 ToneGenStop\n");
#endif
	data = ReadReg(pProHW,pProslic->channel,OCON);
	data &= ~(0x77);
	WriteReg(pProHW,pProslic->channel,OCON,data);
	return 0;
}


/*
** Function: PROSLIC_StartRing
**
** Description: 
** start ring generator
*/
int Si3226_RingStart (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 RingStart\n");
#endif
	Si3226_SetLinefeedStatus(pProslic,LF_RINGING);
	return 0;
}


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
*/
int Si3226_RingStop (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 RingStop\n");
#endif
	Si3226_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	return 0;
}

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
*/
int Si3226_EnableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 EnableCID\n");
#endif
	WriteReg(pProHW,pProslic->channel,OCON,0);

	data = ReadReg(pProHW,pProslic->channel,OMODE);
	data |= 0xA;
	WriteReg(pProHW,pProslic->channel,OMODE,data);

	WriteReg(pProHW,pProslic->channel,OCON,0x5);
	return 0;
}

/*
** Function: PROSLIC_DisableCID
**
** Description: 
** disable fsk
*/
int Si3226_DisableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 DisableCID\n");
#endif
	WriteReg(pProHW,pProslic->channel,OCON,0);
	data = ReadReg(pProHW,pProslic->channel,OMODE);
	data &= ~(0x8);
	WriteReg(pProHW,pProslic->channel,OMODE,data);
	return 0;
}

/*
** Function: PROSLIC_SendCID
**
** Description: 
** send fsk data
*/
int Si3226_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 SendCID\n");
#endif
	while (numBytes-- > 0){
		WriteReg(pProHW,pProslic->channel,FSKDAT,*(buffer++));
	}
	return 0;
}

/*
** Function: PROSLIC_PCMStart
**
** Description: 
** Starts PCM
*/
int Si3226_PCMStart (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 PCMStart[%d]\n",pProslic->channel);
#endif
	data = ReadReg(pProHW,pProslic->channel,PCMMODE);
	data |= 0x10;
	WriteReg(pProHW,pProslic->channel,PCMMODE,data);
	return 0;
}


/*
** Function: PROSLIC_PCMStop
**
** Description: 
** Disables PCM
*/
int Si3226_PCMStop (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226 PCMStop %d\n",pProslic->channel);
#endif
	data = ReadReg(pProHW,pProslic->channel,PCMMODE);
	data &= ~(0x10);
	WriteReg(pProHW,pProslic->channel,PCMMODE,data);
	return 0;
}



/*
** Function: PROSLIC_ReadDTMFDigit
**
** Description: 
** Read DTMF digit (would be called after DTMF interrupt to collect digit)
*/
int Si3226_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit){
	*pDigit = ReadReg(pProHW,pProslic->channel,TONDTMF) & 0xf;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si3226: DTMFReadDigit %d\n",*pDigit);
#endif
	
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
*/
int Si3226_PLLFreeRunStart (proslicChanType *pProslic){
	uInt8 tmp;
	tmp = ReadReg(pProHW,pProslic->channel,ENHANCE);
	WriteReg(pProHW,pProslic->channel,ENHANCE,tmp|0x4);
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
*/
int Si3226_PLLFreeRunStop (proslicChanType *pProslic){
	uInt8 tmp;
	tmp = ReadReg(pProHW,pProslic->channel,ENHANCE);
	WriteReg(pProHW,pProslic->channel,ENHANCE,tmp&~(0x4));
	return 0;
}

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
*/
int Si3226_PulseMeterStart (proslicChanType *pProslic){
	/*not applicable to this part number*/
	return 0;	
}

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
*/
int SI3226_PulseMeterStop (proslicChanType *pProslic){
	/*not applicable to this part number*/
	return 0;
}


/*
** Function: PROSLIC_dbgSetDCFeed
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage 
** and loop current.
*/
int Si3226_dbgSetDCFeed (proslicChanType *pProslic, uInt32 v_vlim_val, uInt32 i_ilim_val, int32 preset){
/* Note:  * needs more descriptive return codes in the event of an out of range arguement */
    uInt16 vslope = 160;
    uInt16 rslope = 720;
    uInt32 vscale1 = 1386; 
    uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */
    uInt32 iscale1 = 913;
    uInt32 iscale2 = 334;    /* 913x334 = 304942 */
    uInt32 i_rfeed_val, v_rfeed_val, const_rfeed_val, i_vlim_val, const_ilim_val, v_ilim_val;
    int32 signedVal;

    /* Assumptions must be made to minimize computations.  This limits the
    ** range of available settings, but should be more than adequate for
    ** short loop applications.
    **
    ** Assumtions:
    ** 
    ** SLOPE_VLIM      =>  160ohms
    ** SLOPE_RFEED     =>  720ohms
    ** I_RFEED         =>  3*I_ILIM/4
    ** 
    ** With these assumptions, the DC Feed parameters now become 
    **
    ** Inputs:      V_VLIM, I_ILIM
    ** Constants:   SLOPE_VLIM, SLOPE_ILIM, SLOPE_RFEED, SLOPE_DELTA1, SLOPE_DELTA2
    ** Outputs:     V_RFEED, V_ILIM, I_VLIM, CONST_RFEED, CONST_ILIM
    **
    */

    /* Validate arguements */
    if((i_ilim_val < 15)||(i_ilim_val > 45)) return 1;   /* need error code */
    if((v_vlim_val < 30)||(v_vlim_val > 52)) return 1;   /* need error code */

    /* Calculate voltages in mV and currents in uA */
    v_vlim_val *= 1000;
    i_ilim_val *= 1000;

    /* I_RFEED */
    i_rfeed_val = (3*i_ilim_val)/4;

    /* V_RFEED */
    v_rfeed_val = v_vlim_val - (i_rfeed_val*vslope)/1000;

    /* V_ILIM */ 
    v_ilim_val = v_rfeed_val - (rslope*(i_ilim_val - i_rfeed_val))/1000;

    /* I_VLIM */
    i_vlim_val = (v_vlim_val*1000)/4903;

    /* CONST_RFEED */
    signedVal = v_rfeed_val * (i_ilim_val - i_rfeed_val);
    signedVal /= (v_rfeed_val - v_ilim_val);
    signedVal = i_rfeed_val + signedVal;

    /*  signedVal in uA here */
    signedVal *= iscale1;
    signedVal /= 100;
    signedVal *= iscale2;
    signedVal /= 10;

    if(signedVal < 0)
    {
        const_rfeed_val = (signedVal)+ (1L<<29);
    }
    else
    {
        const_rfeed_val = signedVal & 0x1FFFFFFF;
    }

    /* CONST_ILIM */
    const_ilim_val = i_ilim_val;

    /* compute RAM values */
    v_vlim_val *= vscale1;
    v_vlim_val /= 100;
    v_vlim_val *= vscale2;
    v_vlim_val /= 10;

    v_rfeed_val *= vscale1;
    v_rfeed_val /= 100;
    v_rfeed_val *= vscale2;
    v_rfeed_val /= 10;

    v_ilim_val *= vscale1;
    v_ilim_val /= 100;
    v_ilim_val *= vscale2;
    v_ilim_val /= 10;

    const_ilim_val *= iscale1;
    const_ilim_val /= 100;
    const_ilim_val *= iscale2;
    const_ilim_val /= 10;

    i_vlim_val *= iscale1;
    i_vlim_val /= 100;
    i_vlim_val *= iscale2;
    i_vlim_val /= 10;

	Si3226_DCfeed_Presets[preset].slope_vlim = 0x18842BD7L;
	Si3226_DCfeed_Presets[preset].slope_rfeed = 0x1E8886DEL;
	Si3226_DCfeed_Presets[preset].slope_ilim = 0x40A0E0L;
	Si3226_DCfeed_Presets[preset].delta1 = 0x1EABA1BFL;
	Si3226_DCfeed_Presets[preset].delta2 = 0x1EF744EAL;
	Si3226_DCfeed_Presets[preset].v_vlim = v_vlim_val;
	Si3226_DCfeed_Presets[preset].v_rfeed = v_rfeed_val;
	Si3226_DCfeed_Presets[preset].v_ilim = v_ilim_val;
	Si3226_DCfeed_Presets[preset].const_rfeed = const_rfeed_val;
	Si3226_DCfeed_Presets[preset].const_ilim = const_ilim_val;
	Si3226_DCfeed_Presets[preset].i_vlim = i_vlim_val;
	
	return 0;
}

/*
** Function: PROSLIC_dbgSetDCFeedVopen
**
** Description: 
** provisionary function for setting up
** dcfeed given desired open circuit voltage.
** Entry I_ILIM value will be used.
*/
int Si3226_dbgSetDCFeedVopen (proslicChanType *pProslic, uInt32 v_vlim_val, int32 preset)
{
uInt32 i_ilim_val;
uInt32 iscale1 = 913;
uInt32 iscale2 = 334;    /* 913x334 = 304942 */

    /* Read present CONST_ILIM value */
    i_ilim_val = Si3226_DCfeed_Presets[preset].const_ilim;


    i_ilim_val /= iscale2;
    i_ilim_val /= iscale1;

    return Si3226_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val,preset);
}

/*
** Function: PROSLIC_dbgSetDCFeedIloop
**
** Description: 
** provisionary function for setting up
** dcfeed given desired loop current.
** Entry V_VLIM value will be used.
*/
int Si3226_dbgSetDCFeedIloop (proslicChanType *pProslic, uInt32 i_ilim_val, int32 preset)
{
uInt32 v_vlim_val;
uInt32 vscale1 = 1386; 
uInt32 vscale2 = 1422;   /* 1386x1422 = 1970892 broken down to minimize trunc err */

    /* Read present V_VLIM value */
    v_vlim_val = Si3226_DCfeed_Presets[preset].v_vlim;

    v_vlim_val /= vscale2;
    v_vlim_val /= vscale1;

    return Si3226_dbgSetDCFeed(pProslic,v_vlim_val,i_ilim_val, preset);
}




typedef struct
{
    uInt8   freq;
    ramData ringfr;      /* trise scale for trap */
    uInt32  ampScale;
} ProSLIC_SineRingFreqLookup;

typedef struct
{
    uInt8    freq;
    ramData  rtacth;
    ramData rtper;
    ramData rtdb;
} ProSLIC_SineRingtripLookup;

typedef struct
{
    uInt8   freq;
    uInt16  cfVal[6];
} ProSLIC_TrapRingFreqLookup;

typedef struct
{
    uInt8   freq;
    ramData rtper;
    ramData rtdb;
    uInt32  rtacth[6];
} ProSLIC_TrapRingtripLookup;


/*
** Function: PROSLIC_dbgRingingSetup
**
** Description: 
** Provisionary function for setting up
** Ring type, frequency, amplitude and dc offset.
** Main use will be by peek/poke applications.
*/
int Si3226_dbgSetRinging (proslicChanType *pProslic, ProSLIC_dbgRingCfg *ringCfg, int preset){
    int errVal,i;
    uInt32 vScale = 1608872L;   /* (2^28/170.25)*((100+4903)/4903) */
    ramData dcdcVminTmp;

    const ProSLIC_SineRingFreqLookup sineRingFreqTable[] =
/*  Freq RINGFR, vScale */
        {{15, 0x7F6E930L, 18968L},
         {16, 0x7F5A8E0L, 20234L},
         {20, 0x7EFD9D5L, 25301L},
         {22, 0x7EC770AL, 27843L},
         {23, 0x7EAA6E2L, 29113L},
         {25, 0x7E6C925L, 31649L},
         {30, 0x7DBB96BL, 38014L},
         {34, 0x7D34155L, 42270L}, /* Actually 33.33Hz */
         {35, 0x7CEAD72L, 44397L},
         {40, 0x7BFA887L, 50802L},
         {45, 0x7AEAE74L, 57233L},
         {50, 0x79BC384L, 63693L},
         {0,0,0}}; /* terminator */

    const ProSLIC_SineRingtripLookup sineRingtripTable[] =
/*  Freq rtacth */
        { {15, 11440000L, 0x6A000L, 0x4000L },
          {16, 10810000L, 0x64000L, 0x4000L },
          {20, 8690000L,  0x50000L, 0x8000L }, 
          {22, 7835000L,  0x48000L, 0x8000L },
          {23, 7622000L,  0x46000L, 0x8000L }, 
          {25, 6980000L,  0x40000L, 0xA000L }, 
          {30, 5900000L,  0x36000L, 0xA000L }, 
          {34, 10490000L, 0x60000L, 0x6000L }, /* Actually 33.33 */
          {35, 10060000L, 0x5C000L, 0x6000L }, 
          {40, 8750000L,  0x50000L, 0x8000L }, 
          {45, 7880000L,  0x48000L, 0x8000L }, 
          {50, 7010000L,  0x40000L, 0xA000L }, 
          {0,0L}}; /* terminator */

    const ProSLIC_TrapRingFreqLookup trapRingFreqTable[] =
/*  Freq multCF11 multCF12 multCF13 multCF14 multCF15 multCF16*/
    {
        {15, {69,122, 163, 196, 222,244}},
        {16, {65,115, 153, 184, 208,229}},
        {20, {52,92, 122, 147, 167,183}},
        {22, {47,83, 111, 134, 152,166}},
        {23, {45,80, 107, 128, 145,159}},
        {25, {42,73, 98, 118, 133,146}},
        {30, {35,61, 82, 98, 111,122}},
        {34, {31,55, 73, 88, 100,110}},
        {35, {30,52, 70, 84, 95,104}},
        {40, {26,46, 61, 73, 83,91}},
        {45, {23,41, 54, 65, 74,81}},
        {50, {21,37, 49, 59, 67,73}},
        {0,{0L,0L,0L,0L}} /* terminator */
    }; 


    const ProSLIC_TrapRingtripLookup trapRingtripTable[] =
/*  Freq rtper rtdb rtacthCR11 rtacthCR12 rtacthCR13 rtacthCR14 rtacthCR15 rtacthCR16*/
    {
        {15, 0x6A000L,  0x4000L, {16214894L, 14369375L, 12933127L, 11793508L, 10874121L, 10121671L}},
        {16, 0x64000L,  0x4000L, {15201463L, 13471289L, 12124806L, 11056414L, 10194489L, 9489067L}},
        {20, 0x50000L,  0x6000L, {12161171L, 10777031L, 9699845L, 8845131L, 8155591L, 7591253L}},
        {22, 0x48000L,  0x6000L, {11055610L, 9797301L, 8818041L, 8041028L, 7414174L, 6901139L}},
        {23, 0x46000L,  0x6000L, {10574931L, 9371331L, 8434648L, 7691418L, 7091818L, 6601090L}},
        {25, 0x40000L,  0x8000L, {9728937L, 8621625L, 7759876L, 7076105L, 6524473L, 6073003L}},
        {30, 0x36000L,  0x8000L, {8107447L, 7184687L, 6466563L, 5896754L, 5437061L, 5060836L}},
        {34, 0x60000L,  0x6000L, {7297432L, 6466865L, 5820489L, 5307609L, 4893844L, 4555208L}},
        {35, 0x5C000L,  0x6000L, {6949240L, 6158303L, 5542769L, 5054361L, 4660338L, 4337859L}},
        {40, 0x50000L,  0x6000L, {6080585L, 5388516L, 4849923L, 4422565L, 4077796L, 3795627L}},
        {45, 0x48000L,  0x6000L, {5404965L, 4789792L, 4311042L, 3931169L, 3624707L, 3373890L}},
        {50, 0x40000L,  0x8000L, {4864468L, 4310812L, 3879938L, 3538052L, 3262236L, 3036501L}},
        {0,0x0L, 0x0L, {0L,0L,0L,0L}} /* terminator */
    }; 

    errVal = 0;

    switch(ringCfg->ringtype)
    {
    case ProSLIC_RING_SINE:
        i=0;
        do
        {
            if(sineRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (sineRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(sineRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = 1;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3226_Ring_Presets[preset].freq = sineRingFreqTable[i].ringfr;
        Si3226_Ring_Presets[preset].amp = ringCfg->amp * sineRingFreqTable[i].ampScale;
        Si3226_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3226_Ring_Presets[preset].phas = 0L;

        /* Don't alter anything in RINGCON other than clearing the TRAP bit */
        Si3226_Ring_Presets[preset].ringcon &= 0xFE;

        Si3226_Ring_Presets[preset].rtper = sineRingtripTable[i].rtper;
        Si3226_Ring_Presets[preset].rtacdb = sineRingtripTable[i].rtdb;
        Si3226_Ring_Presets[preset].rtdcdb = sineRingtripTable[i].rtdb;
        Si3226_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3226_Ring_Presets[preset].rtacth = sineRingtripTable[i].rtacth;
        break;

    case ProSLIC_RING_TRAP_CF11:  
    case ProSLIC_RING_TRAP_CF12:     
    case ProSLIC_RING_TRAP_CF13: 
    case ProSLIC_RING_TRAP_CF14: 
    case ProSLIC_RING_TRAP_CF15:  
    case ProSLIC_RING_TRAP_CF16:  
        i=0;
        do
        {
            if(trapRingFreqTable[i].freq >= ringCfg->freq) 
            {
                break;
            }
            i++;
        } while (trapRingFreqTable[i].freq);

        /* Set to maximum value if exceeding maximum value from table */
        if(trapRingFreqTable[i].freq == 0)
        {
            i--;
            errVal = 1;
        }

        /* Update RINGFR RINGAMP, RINGOFFSET, and RINGCON */
        Si3226_Ring_Presets[preset].amp = ringCfg->amp * vScale;
        Si3226_Ring_Presets[preset].freq = Si3226_Ring_Presets[preset].amp/trapRingFreqTable[i].cfVal[ringCfg->ringtype];
        Si3226_Ring_Presets[preset].offset = ringCfg->offset * vScale;
        Si3226_Ring_Presets[preset].phas = 262144000L/trapRingFreqTable[i].freq;

        /* Don't alter anything in RINGCON other than setting the TRAP bit */
        Si3226_Ring_Presets[preset].ringcon |= 0x01; 

        /* RTPER and debouce timers  */
        Si3226_Ring_Presets[preset].rtper = trapRingtripTable[i].rtper;
        Si3226_Ring_Presets[preset].rtacdb = trapRingtripTable[i].rtdb;
        Si3226_Ring_Presets[preset].rtdcdb = trapRingtripTable[i].rtdb;  


        Si3226_Ring_Presets[preset].rtdcth = 0xFFFFFFFL;
        Si3226_Ring_Presets[preset].rtacth = trapRingtripTable[i].rtacth[ringCfg->ringtype];


        break;
    }

    /* 
    ** DCDC tracking sluggish under light load at higher ring freq.
    ** Reduce tracking depth above 40Hz.  This should have no effect
    ** if using the Buck-Boost architecture.
    */
    if((sineRingFreqTable[i].freq >= 40)||(Si3226_General_Configuration.bomOpt == BO_DCDC_BUCK_BOOST))
    {
        dcdcVminTmp = ringCfg->amp + ringCfg->offset;
        dcdcVminTmp *= 1000;
        dcdcVminTmp *= SCALE_V_MADC;
        Si3226_Ring_Presets[preset].dcdc_vref_min_rng = dcdcVminTmp;
    }
    else
    {
        Si3226_Ring_Presets[preset].dcdc_vref_min_rng = 0x1800000L;
    }

    return errVal;

}



typedef struct
{
    int32   gain;
    uInt32 scale;
} ProSLIC_GainScaleLookup;

/*to be verified*/
#define GAIN_DELTA_MAX 6
#define GAIN_DELTA_MIN -12
#define GAIN_MAX 6
#define GAIN_MIN -12
#define MAX_AC_GAIN 3
#define MIN_AC_GAIN -6

/*
** Function: PROSLIC_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX gain
*/
int Si3226_dbgSetTXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
	int errVal = 0;
	int32 i;
	/*we adjust the gain starting from the impedance preset specified
	and store the results in the audio gain preset */
	int32 gain_delta; 
	int32 gain_1, gain_2;
	const ProSLIC_GainScaleLookup gainScaleTable[] =
/*  gain, scale=10^(gain/20) */
   { {-6, 501},
    {-5, 562},
    {-4, 631},
    {-3, 708},
    {-2, 794},
    {-1, 891},
    {0, 1000},
	{1, 1122},
	{2, 1259},
	{3, 1413},
	{4, 1585},
	{5, 1778},
	{6, 1995},
    {0xff,0}}; /* terminator */

	gain_delta = ((int32)gain) - Si3226_Impedance_Presets[impedance_preset].txgain_db;

	if (gain_delta > GAIN_DELTA_MAX || gain_delta < GAIN_DELTA_MIN)
		errVal = RC_GAIN_DELTA_TOO_LARGE;
	if (gain > GAIN_MAX || gain < GAIN_MIN)
		errVal = RC_GAIN_OUT_OF_RANGE;
	
	gain_1 = gain_delta;
	gain_2 = 0;

	if (gain_delta > MAX_AC_GAIN)
	{
		gain_1 = MAX_AC_GAIN;
		gain_2 = gain_delta - MAX_AC_GAIN;
	}
	if (gain_delta < MIN_AC_GAIN)
	{
		gain_1 = MIN_AC_GAIN;
		gain_2 = gain_delta - MIN_AC_GAIN;
	}

    i=0;
    do
    {
 
       if(gainScaleTable[i].gain >= gain_1) 
        {
		
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

	Si3226_audioGain_Presets[audio_gain_preset].acgain = 
             (Si3226_Impedance_Presets[impedance_preset].txgain/1000)*gainScaleTable[i].scale;

	i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_2) 
        {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
		errVal = RC_GAIN_DELTA_TOO_LARGE;
    }
    	/*sign extend negative numbers*/
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3 |= 0xf0000000L;

        Si3226_audioGain_Presets[audio_gain_preset].aceq_c0 = 
             ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c0/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c1 = 
             ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c1/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c2 = 
             ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c2/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c3 = 
             ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.txaceq_c3/1000)*gainScaleTable[i].scale;


    return errVal;
}

/*
** Function: PROSLIC_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX gain
*/
int Si3226_dbgSetRXGain (proslicChanType *pProslic, int32 gain, int impedance_preset, int audio_gain_preset){
	int errVal = 0;
	int32 i;
	/*we adjust the gain starting from the impedance preset specified
	and store the results in the audio gain preset */
	int32 gain_delta; 
	int32 gain_1, gain_2;
	const ProSLIC_GainScaleLookup gainScaleTable[] =
/*  gain, scale=10^(gain/20) */
   { {-6, 501},
    {-5, 562},
    {-4, 631},
	{-3, 708},
    {-2, 794},
    {-1, 891},
    {0, 1000},
	{1, 1122},
	{2, 1259},
	{3, 1413},
	{4, 1585},
	{5, 1778},
	{6, 1995},
    {0xff,0}}; /* terminator */

	gain_delta = ((int32)gain) - Si3226_Impedance_Presets[impedance_preset].rxgain_db;
	if (gain_delta > GAIN_DELTA_MAX || gain_delta < GAIN_DELTA_MIN)
		errVal = RC_GAIN_DELTA_TOO_LARGE;
	if (gain > GAIN_MAX || gain < GAIN_MIN)
		errVal = RC_GAIN_OUT_OF_RANGE;
	
	gain_1 = gain_delta;
	gain_2 = 0;

	if (gain_delta > MAX_AC_GAIN)
	{
		gain_1 = MAX_AC_GAIN;
		gain_2 = gain_delta - MAX_AC_GAIN;
	}
	if (gain_delta < MIN_AC_GAIN)
	{
		gain_1 = MIN_AC_GAIN;
		gain_2 = gain_delta - MIN_AC_GAIN;
	}

    i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_1) 
        {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
		errVal = RC_GAIN_DELTA_TOO_LARGE;
    }
	Si3226_audioGain_Presets[audio_gain_preset].acgain = 
             (Si3226_Impedance_Presets[impedance_preset].rxgain/1000)*gainScaleTable[i].scale;

	i=0;
    do
    {
        if(gainScaleTable[i].gain >= gain_2) 
        {
            break;
        }
        i++;
    } while (gainScaleTable[i].gain!=0xff);

    /* Set to maximum value if exceeding maximum value from table */
    if(gainScaleTable[i].gain == 0xff)
    {
        i--;
		errVal = RC_GAIN_DELTA_TOO_LARGE;
    }
	/*sign extend negative numbers*/
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2 |= 0xf0000000L;
	if (Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 & 0x10000000L)
		Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3 |= 0xf0000000L;

	Si3226_audioGain_Presets[audio_gain_preset].aceq_c0 = 
               ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c0/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c1 = 
               ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c1/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c2 = 
               ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c2/1000)*gainScaleTable[i].scale;
	Si3226_audioGain_Presets[audio_gain_preset].aceq_c3 = 
               ((int32)Si3226_Impedance_Presets[impedance_preset].audioEQ.rxaceq_c3/1000)*gainScaleTable[i].scale;


    return errVal;
}

/*
** Function: Si3226_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si3226_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor)
{
    if(pProslic->channelEnable)
    {
        monitor->vtr    = ReadRAM(pProHW,pProslic->channel,VDIFF_FILT);
        if(monitor->vtr & 0x10000000L)
           monitor->vtr |= 0xf0000000L;
        monitor->vtr /= SCALE_V_MADC;

        monitor->vtip    = ReadRAM(pProHW,pProslic->channel,VTIP);
        if(monitor->vtip & 0x10000000L)
           monitor->vtip |= 0xf0000000L;
        monitor->vtip /= SCALE_V_MADC;

        monitor->vring    = ReadRAM(pProHW,pProslic->channel,VRING);
        if(monitor->vring & 0x10000000L)
           monitor->vring |= 0xf0000000L;
        monitor->vring /= SCALE_V_MADC;

        monitor->vbat    = ReadRAM(pProHW,pProslic->channel,MADC_VBAT);
        if(monitor->vbat & 0x10000000L)
           monitor->vbat |= 0xf0000000L;
        monitor->vbat /= SCALE_V_MADC;

        monitor->itr  = ReadRAM(pProHW,pProslic->channel,MADC_ILOOP);
        if(monitor->itr & 0x10000000L)
           monitor->itr |= 0xf0000000L;
        monitor->itr /= SCALE_I_MADC;

        monitor->itip  = ReadRAM(pProHW,pProslic->channel,MADC_ITIP);
        if(monitor->itip & 0x10000000L)
           monitor->itip |= 0xf0000000L;
        monitor->itip /= SCALE_I_MADC;

        monitor->iring  = ReadRAM(pProHW,pProslic->channel,MADC_IRING);
        if(monitor->iring & 0x10000000L)
           monitor->iring |= 0xf0000000L;
        monitor->iring /= SCALE_I_MADC;

        monitor->ilong  = ReadRAM(pProHW,pProslic->channel,MADC_ILONG);
        if(monitor->ilong & 0x10000000L)
           monitor->ilong |= 0xf0000000L;
        monitor->ilong /= SCALE_I_MADC;

    }

    return 0;
}

/*
** Function: Si3226_PSTNCheck
**
** Description: 
** Continuous monitoring of longitudinal current.
** If an average of N samples exceed avgThresh or a
** single sample exceeds singleThresh, the linefeed 
** is forced into the open state.
**
** This protects the port from connecting to a live
** pstn line (faster than power alarm).
**
** TODO:  need error handling
*/
int Si3226_PSTNCheck (proslicChanType *pProslic,proslicPSTNCheckObjType *pPSTNCheck)
{
uInt8 i;
    /* Adjust buffer index */
    if(pPSTNCheck->count >= pPSTNCheck->samples)
    {
        pPSTNCheck->buffFull = TRUE;
        pPSTNCheck->count = 0;   /* reset buffer ptr */
    }

    /* Read next sample */
    pPSTNCheck->ilong[pPSTNCheck->count]  = ReadRAM(pProHW,pProslic->channel,MADC_ILONG);
    if(pPSTNCheck->ilong[pPSTNCheck->count] & 0x10000000L)
        pPSTNCheck->ilong[pPSTNCheck->count] |= 0xf0000000L;
    pPSTNCheck->ilong[pPSTNCheck->count] /= SCALE_I_MADC;

    /* Monitor magnitude only */
    if(pPSTNCheck->ilong[pPSTNCheck->count] < 0)
        pPSTNCheck->ilong[pPSTNCheck->count] = -pPSTNCheck->ilong[pPSTNCheck->count];

    /* Quickly test for single measurement violation */
    if(pPSTNCheck->ilong[pPSTNCheck->count] > pPSTNCheck->singleThresh)
        return 1;  /* fail */

    /* Average once buffer is full */
    if(pPSTNCheck->buffFull == TRUE)  
    {
        pPSTNCheck->avgIlong = 0;
        for(i=0;i<pPSTNCheck->samples; i++)
        {
            pPSTNCheck->avgIlong += pPSTNCheck->ilong[i];
        }
        pPSTNCheck->avgIlong /= pPSTNCheck->samples;

        if(pPSTNCheck->avgIlong > pPSTNCheck->avgThresh)    
        {
            /* reinit obj and return fail */
            pPSTNCheck->count = 0;
            pPSTNCheck->buffFull = FALSE;
            return 1;
        }
        else
        {
            pPSTNCheck->count++;
            return 0;
        }   
    }
    else
    {
        pPSTNCheck->count++;
        return 0;
    }
}

/*
** Function: Si3226_AudioGainSetup
**
** Description: 
** Set audio gain of RX and TX paths - presumed that
** all zsynth coefficient presets are 0dB
**
*/

int Si3226_AudioGainSetup(proslicChanType *pProslic, int32 rxgain, int32 txgain, int preset)
{

    Si3226_dbgSetTXGain(pProslic,rxgain,preset,0);
    Si3226_dbgSetRXGain(pProslic,txgain,preset,1);
    Si3226_TXAudioGainSetup(pProslic,0);
    Si3226_RXAudioGainSetup(pProslic,1);
   
    return 0;
}



