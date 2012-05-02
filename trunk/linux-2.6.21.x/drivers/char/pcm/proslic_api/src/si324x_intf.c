/*
** Copyright (c) 2007-2009 by Silicon Laboratories
**
** $Id: si324x_intf.c,v 1.1 2010-07-30 07:55:38 qwert Exp $
**
** Si324x_Intf.c
** Si324x ProSLIC interface implementation file
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
** Dependancies:
** proslic_Gcispi.c
**
*/

#include "si_voice_datatypes.h"
#include "si_voice_ctrl.h"
#include "si_voice_timer_intf.h"
#include "proslic.h"
#include "si324x_intf.h"
#include "si324x.h"
#include "si324x_registers.h"
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

#ifndef SI324X_DEFAULT_IMPEDANCE_RXGAIN
#define SI324X_DEFAULT_IMPEDANCE_RXGAIN 0
#endif

#ifndef SI324X_DEFAULT_IMPEDANCE_TXGAIN
#define SI324X_DEFAULT_IMPEDANCE_TXGAIN 0
#endif


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
	WriteReg(pProHW,pProslic->channel,126,2);
	WriteReg(pProHW,pProslic->channel,126,8);
	WriteReg(pProHW,pProslic->channel,126,0xe);
	WriteReg(pProHW,pProslic->channel,126,0);
	if (SetSemaphore != NULL)
		SetSemaphore(pProHW,0);
	return 0;
}

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
	data = ReadReg(pProHW,pProslic->channel,126);/*we check first channel. we assume all channels same user mode state*/
	if (((data&1) != 0) == on)
		return 0;
	WriteReg(pProHW,BROADCAST,126,2);
	WriteReg(pProHW,BROADCAST,126,8);
	WriteReg(pProHW,BROADCAST,126,0xe);
	WriteReg(pProHW,BROADCAST,126,0);
	if (SetSemaphore != NULL)
		SetSemaphore(pProHW,0);
	return 0;
}

static int probeDaisyChain (proslicChanType *pProslic){
	int i=0;
	WriteReg(pProHW,BROADCAST,RAM_ADDR_HI,0x40);
	while ((ReadReg(pProHW,(uInt8)i++,RAM_ADDR_HI) == 0x40) && (i<=32));
	WriteReg(pProHW,BROADCAST,RAM_ADDR_HI,0x0);
	return i-1;
}

static int cal_iteration (proslicChanType *pProslic,int iter,uInt8 *data2,uInt8 numChan){
	int i;
	uInt8 data,data3;
	int millisecs=0;
	for (i=0;i<numChan;i+=4){ /*we do each channel in each chip*/
		WriteReg(pProHW,(uInt8)(i+iter),CALR0,data2[0]);
		WriteReg(pProHW,(uInt8)(i+iter),CALR1,data2[1]);
		WriteReg(pProHW,(uInt8)(i+iter),CALR2,data2[2]);
		WriteReg(pProHW,(uInt8)(i+iter),CALR3,data2[3]);
	}
	do{
		data3=0;
		for (i=0;i<numChan;i+=4){
			data = ReadReg(pProHW,(uInt8)(i+iter),CALR3);
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

static int calibrate (proslicChanType *pProslic, int broadcast){
	/*
	** This function will perform the ProSLIC calibration sequence 
	*/
	int i;
	uInt8 data [] = {0x0, 0xff, 0xef, 0x80}; 
	uInt8 data3=0;
	uInt8 numChan=0;
	if (broadcast){
		WriteRAM(pProHW,BROADCAST,1012,0x1FFFFFFFL); /*ac dac gain cal en*/
		numChan = (uInt8)probeDaisyChain(pProslic);
		for (i=0;i<4;i++){
			data3 |= cal_iteration(pProslic,i,data,numChan);
		}
	}
	else {/*to do - add parallel cals on separate devices*/
		i=0;
		WriteRAM(pProHW,pProslic->channel,1012,0x1FFFFFFFL); /*ac dac gain cal en*/
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
		LOGPRINT("Calibration done. channel %d\n",pProslic->channel);
#endif
	return data3;
}

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
	while (pRamTable[i].address != 0xffff){
		WriteRAM(pProHW,channel,pRamTable[i].address,pRamTable[i].initValue); 
		i++;
	}
	
	i=0;
	while (pRegTable[i].address != 0xff){
		WriteReg(pProHW,channel,pRegTable[i].address,pRegTable[i].initValue);
		i++;
	}
	if (broadcast)
		setUserModeBroadcast(pProslic,FALSE);
	else
		setUserMode(pProslic,FALSE);

	return 0;
}
#ifdef DISABLE_LOOPMAN
#else
int enableSi324xLoopman (proslicChanType *pProslic, int broadcast){
	uInt8 channel;
	if (broadcast){
		setUserModeBroadcast(pProslic,TRUE);
		channel = BROADCAST;
	}
	else{
		setUserMode (pProslic,TRUE); /*make sure we are in user mode */
		channel = pProslic->channel;
	}

	WriteRAM(pProHW, channel,1445, 0);
	WriteRAM(pProHW, channel,1451, 0x200000L);
	Delay(pProTimer,1);  /* Wait for HVIC to exit standby */
	WriteRAM(pProHW, channel,1445, 0x300000L);
	WriteReg(pProHW, channel,98,0x80); /*enable MADC to prevent power alarm. this could also be done before going active*/
	if (broadcast){
		setUserModeBroadcast(pProslic,FALSE);
	}
	else {
		setUserMode(pProslic,FALSE); /*turn off user mode*/
	}
	return 0;
}
int disableSi324xLoopman (proslicChanType *pProslic, int broadcast){
	uInt8 channel;
	if (broadcast){
		setUserModeBroadcast(pProslic,TRUE);
		channel = BROADCAST;
	}
	else{
		setUserMode (pProslic,TRUE); /*make sure we are in user mode */
		channel = pProslic->channel;
	}

	WriteRAM(pProHW, channel,1445, 0x0);
	if (broadcast){
		setUserModeBroadcast(pProslic,FALSE);
	}
	else {
		setUserMode(pProslic,FALSE); /*turn off user mode*/
	}
	return 0;
}
#endif
static int LoadQuadPatch (proslicChanType *pProslic, const proslicPatch *pPatch,int broadcast){ 
	uInt32 loop;
	uInt8 jmp_table=82;
	uInt8 channel;
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

	for (loop=0;loop<8;loop++){
		/*zero out the jump table*/
		WriteReg (pProHW, channel, jmp_table,0);
		WriteReg (pProHW, channel, jmp_table+1,0);
	
		jmp_table+=2;
	}

	WriteRAM(pProHW, channel,PRAM_ADDR, 0); /*write patch ram address register*/
	/*If the data is all 0, you have hit the end of the programmed values and can stop loading.*/
    for (loop=0; loop<1024; loop++){
		if (pPatch->patchData[loop] != 0){
			if ((pProslic->deviceId->chipRev < 4) && broadcast)
				WriteRAM(pProHW, channel,PRAM_ADDR, loop<<19); /*write patch ram address register (only necessary for broadcast rev d)*/
            WriteRAM(pProHW, channel,PRAM_DATA,pPatch->patchData[loop]<<9); /*loading patch, note. *data is shifted**/
		}
        else
            loop = 1024;
    }

    /*zero out RAM_ADDR_HI*/
	WriteReg (pProHW, channel, RAM_ADDR_HI,0);

	jmp_table=82;
	for (loop=0;loop<8;loop++){
		/*Load the jump table with the new values.*/
		if (pPatch->patchEntries[loop] != 0){
			WriteReg (pProHW, channel, jmp_table,(uInt8)((pPatch->patchEntries[loop])&0xff));
			WriteReg (pProHW, channel, (uInt8)(jmp_table+1),(uInt8)(pPatch->patchEntries[loop]>>8));
		}
		jmp_table+=2;
	}

	WriteRAM(pProHW,channel,448,pPatch->patchSerial); /*write patch identifier*/
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
int Si324x_Reset (proslicChanType_ptr pProslic){
	/*
	** resets ProSLIC, wait 250ms, release reset, wait 250ms
	*/
	Reset(pProHW,1);
	Delay(pProTimer,250);
	Reset(pProHW,0);
	Delay(pProTimer,250);
	return 0;
}

int Si324x_VerifyControlInterface (proslicChanType_ptr pProslic)
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
	WriteRAM(pProHW,BROADCAST,448,0x12345678L);

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
		if (ReadRAM(pProHW,i,448) != 0x12345678L){
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
** Function: PROSLIC_Init
**
** Description: 
** Initializes the ProSLIC
*/
extern const proslicPatch RevDPatch;
extern Si324x_General_Cfg Si324x_General_Configuration;

int Si324x_Init (proslicChanType_ptr *pProslic, int size){
	/*
	** This function will initialize the chipRev and chipType members in pProslic
	** as well as load the initialization structures.
	*/
	uInt8 data; 
	int k;

	for (k=0;k<size;k++){
		data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,ID);
		pProslic[k]->deviceId->chipRev = data&0x7;
		pProslic[k]->deviceId->chipType= ((data&0x38)>>3) + SI3240;
	}
	for (k=0;k<size;k++){
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO,0x5a);
		if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO) != 0x5A){
			pProslic[k]->channelEnable = 0;
			pProslic[k]->error = RC_SPI_FAIL;
		}
	}
	
	for (k=0;k<size;k++){
		if (pProslic[k]->channelEnable){
			if (pProslic[k]->deviceId->chipRev == 3) {/*rev d*/				
				Si324x_LoadPatch(pProslic[k],&RevDPatch);
				data = (uInt8)Si324x_VerifyPatch(pProslic[k],&RevDPatch);
				if (data){
					pProslic[k]->channelEnable=0;
					pProslic[k]->error = RC_PATCH_ERR;	
				} else {
					setUserMode(pProslic[k],TRUE);
					pProslic[k]->WriteRegX (pProslic[k]->pProHWX, pProslic[k]->channel,JMPEN,1); /*enable the patch*/ 
					setUserMode(pProslic[k],FALSE);
				}
				
				

			}
		}

	}
	for (k=0;k<size;k++){ 
        if(pProslic[k]->channelEnable){
		setUserMode(pProslic[k],TRUE);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,VBATR_EXPECT,Si324x_General_Configuration.vbatr_expect);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,VBATH_EXPECT,Si324x_General_Configuration.vbath_expect);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,COEF_P_HVIC,Si324x_General_Configuration.coef_p_hvic);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,P_TH_HVIC,Si324x_General_Configuration.p_th_hvic);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,AUTO,Si324x_General_Configuration.autoRegister);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CM_CLAMP,Si324x_General_Configuration.cm_clamp);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,VBATL_EXPECT,Si324x_General_Configuration.vbatl_expect);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,BAT_DBI,Si324x_General_Configuration.bat_dbi);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,BAT_HYST,Si324x_General_Configuration.bat_hyst);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,BAT_SETTLE,Si324x_General_Configuration.bat_settle);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,BATSELMAP,Si324x_General_Configuration.batselmap);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,COEF_P_OFFLD,Si324x_General_Configuration.coef_p_offld);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,OFFLOAD,Si324x_General_Configuration.offld);
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,P_TH_OFFLD,Si324x_General_Configuration.p_th_offld);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,ZCAL_EN,0);
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,GPIO_CFG1,0x60); /*for coarse sensors (analog mode)*/
		pProslic[k]->WriteRAMX(pProslic[k]->pProHWX, pProslic[k]->channel,1417,0x200000L); /*PD_DC_COARSE_SNS enable*/
		setUserMode(pProslic[k],FALSE);
        }
	}

    Si324x_Cal(pProslic,size);

	for (k=0;k<size;k++){ 
		if (pProslic[k]->channelEnable){
#ifdef DISABLE_LOOPMAN
#else
				enableSi324xLoopman(pProslic[k],FALSE);
#endif
		}
	}
	return 0;
}


int Si324x_InitBroadcast (proslicChanType_ptr *pProslic){
	/*
	** This function will initialize the chipRev and chipType members in pProslic
	** as well as load the initialization structures.
	*/
	uInt8 data; 
	uInt8 k;
	uInt8 numOfChan;

    /*
    ** Probe daisy chain to see how many on chain
    */
    numOfChan = probeDaisyChain(pProslic[0]);
   	if (numOfChan == 0)
		return 0;

    /*
    ** Read channel id to estalish chipRev and chipType
    */
    for(k=0;k<numOfChan;k++)
    {
	    data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,ID);  
 	    pProslic[k]->deviceId->chipRev = data&0x7;
	    pProslic[k]->deviceId->chipType= ((data&0x38)>>3) + SI3240;
    }

    /*
    ** Probe each channel and enable all channels that respond
    */

	pProslic[0]->WriteRegX(pProslic[0]->pProHWX,BROADCAST,PCMRXLO,0x5a);
    for(k=0;k<numOfChan;k++)
    {
		pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO,0x5a);
		if (pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,PCMRXLO) != 0x5A){
			pProslic[k]->channelEnable = 0;
			pProslic[k]->error = RC_SPI_FAIL;	
        }
	}


    /* 
    ** Broadcast patch load, but must verify sequentially
    */
	if (pProslic[0]->deviceId->chipRev == 3) {/*rev d*/
		LoadQuadPatch(pProslic[0],&RevDPatch,1);
    }


    for(k=0;k<numOfChan;k++)
    {
        if(pProslic[k]->channelEnable) 
        {
            data = Si324x_VerifyPatch(pProslic[k],&RevDPatch);
            if(data)
            {
                pProslic[k]->channelEnable = 0;
                pProslic[k]->error=RC_PATCH_ERR;
            }
            else
            {
                setUserMode(pProslic[k],TRUE);
                pProslic[k]->WriteRegX(pProslic[k]->pProHWX,pProslic[k]->channel,JMPEN,1);
                setUserMode(pProslic[k],FALSE);
            }
        }
    }
  
 
    /*
    ** Load general parameters 
    */

	setUserModeBroadcast(pProslic[0],TRUE);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,VBATR_EXPECT,Si324x_General_Configuration.vbatr_expect);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,VBATH_EXPECT,Si324x_General_Configuration.vbath_expect);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,COEF_P_HVIC,Si324x_General_Configuration.coef_p_hvic);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,P_TH_HVIC,Si324x_General_Configuration.p_th_hvic);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,AUTO,Si324x_General_Configuration.autoRegister);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,CM_CLAMP,Si324x_General_Configuration.cm_clamp);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,VBATL_EXPECT,Si324x_General_Configuration.vbatl_expect);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,BAT_DBI,Si324x_General_Configuration.bat_dbi);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,BAT_HYST,Si324x_General_Configuration.bat_hyst);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,BAT_SETTLE,Si324x_General_Configuration.bat_settle);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,BATSELMAP,Si324x_General_Configuration.batselmap);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,COEF_P_OFFLD,Si324x_General_Configuration.coef_p_offld);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,OFFLOAD,Si324x_General_Configuration.offld);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,P_TH_OFFLD,Si324x_General_Configuration.p_th_offld);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,ZCAL_EN,0);
	pProslic[0]->WriteRegX(pProslic[0]->pProHWX, BROADCAST,GPIO_CFG1,0xF0); /*for coarse sensors (analog mode)*/
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,1417,0x200000L); /*PD_DC_COARSE_SNS enable*/
	/* DC_FEED */ 
	/*Ilim=20mA, Irfeed = 15mA, Slope_vlim=200 Ohms, Slope_rfeed=2 kOhms*/
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,V_VLIM,0x567609CL);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,V_RFEED,0x50D2839L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,V_ILIM,0x3E06C43L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,CONST_RFEED,0xA9D628L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_VLIM,0x1A10433FL);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,CONST_ILIM,0x5D0FA6L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_RFEED,0x1FA2311CL);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_ILIM,0x40A0E0L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_DELTA1,0x1E90517EL);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_DELTA2,0x1EB51625L);
	pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,I_VLIM,0x2BA7AFL);

	if (pProslic[0]->deviceId->chipRev == 3) {/*rev d*/
		pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,IRING_LIM,0xD17480L);
		pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,1023,0x1400000L);
		pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,872,0x1B4E81L);
		pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,873,0x3FB72EL);
		pProslic[0]->WriteRAMX(pProslic[0]->pProHWX, BROADCAST,SLOPE_RING,0x1B7B0000L);
	}

	setUserModeBroadcast(pProslic[0],FALSE);

	data = (uInt8)calibrate(pProslic[0],1);
	if (data != 0)	
		return RC_CAL_TIMEOUT;

#ifdef DISABLE_LOOPMAN
#else
	enableSi324xLoopman(pProslic[0],TRUE);
#endif

	return 0;
}


int Si324x_PrintDebugData (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
		int i;
		for (i=0;i<99;i++)
			LOGPRINT ("Si324x Register %d = %X\n",i,ReadReg(pProHW,pProslic->channel,i));
		for (i=0;i<1024;i++)
			LOGPRINT ("Si324x RAM %d = %X\n",i,ReadRAM(pProHW,pProslic->channel,i));
#endif
		return 0;
}



#ifdef DISABLE_PULSE_SETUP
#define LB_CAL_TIMEOUT 121 /*12 seconds*/
#else
#define LB_CAL_TIMEOUT 241 /*24 seconds*/
#endif
int Si324x_LBCal (proslicChanType_ptr *pProslic, int size)
{
	int k,i=0;
	uInt8 lf; uInt8 data;
	for (k=0;k<size;k++){
		if (pProslic[k]->channelEnable){
			lf = pProslic[k]->ReadRegX(pProslic[k]->pProHWX, pProslic[k]->channel,LINEFEED); 
			Si324x_SetLinefeedStatus(pProslic[k],LF_OPEN);
#ifdef DISABLE_LOOPMAN
#else
			disableSi324xLoopman(pProslic[k],FALSE);
#endif
			Si324x_SetLinefeedStatus(pProslic[k],LF_FWD_ACTIVE);
#ifdef DISABLE_PULSE_SETUP
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CALR0,0xC); /*enable LB cal*/
#else
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CALR0,0xF); /*enable LB cal*/
#endif
			pProslic[k]->WriteRegX(pProslic[k]->pProHWX, pProslic[k]->channel,CALR3,0x80); /*start cal*/
			i=0;
			do {
				data = pProslic[k]->ReadRegX(pProslic[k]->pProHWX,pProslic[k]->channel,CALR3);
				pProslic[k]->DelayX(pProslic[k]->pProTimerX,100);
				if (i++ >= LB_CAL_TIMEOUT){
#ifdef ENABLE_DEBUG
				if (pProslic[k]->debugMode)
					LOGPRINT("Calibration timeout channel %d\n",pProslic[k]->channel);
#endif
				pProslic[k]->error = RC_CAL_TIMEOUT; 
                Si324x_SetLinefeedStatus(pProslic[k],LF_OPEN);
				return RC_CAL_TIMEOUT;
			
			}
			} while (data&0x80 && i<=LB_CAL_TIMEOUT);
#ifdef DISABLE_LOOPMAN
#else
			enableSi324xLoopman(pProslic[k],FALSE);
#endif
			Si324x_SetLinefeedStatus(pProslic[k],lf);
		}
	}
	return 0;
}




int Si324x_LBCalBroadcast (proslicChanType *pProslic)
{
	uInt8 data;int k,j,i;
	int error=0;
	int numOfChan = probeDaisyChain(pProslic);
	Si324x_SetLinefeedStatusBroadcast(pProslic,LF_OPEN);
#ifdef DISABLE_LOOPMAN
#else
			disableSi324xLoopman(pProslic,TRUE);
#endif
	Si324x_SetLinefeedStatusBroadcast(pProslic,LF_FWD_ACTIVE);
#ifdef DISABLE_PULSE_SETUP
	WriteReg(pProHW, BROADCAST,CALR0,0xC); 
#else
	WriteReg(pProHW, BROADCAST,CALR0,0xF); 
#endif
	for (j=0;j<2;j++) {
		for (k=0;k<numOfChan;k+=2){
				WriteReg(pProHW, k+j,CALR3,0x80); /*start cal*/
		}
		i=0;
		do {
			data = 0;
			for (k=0;k<numOfChan;k+=2){
					data |= ReadReg(pProHW,k+j,CALR3);
			}	
			Delay(pProTimer,100);
			if (i++ >= LB_CAL_TIMEOUT){
#ifdef ENABLE_DEBUG
				if (pProslic->debugMode)
					LOGPRINT("Si324x LB Calibration timeout\n");
#endif
				pProslic->error = RC_CAL_TIMEOUT;
				error = RC_CAL_TIMEOUT;
				
			}
		} while (data&0x80 && i<=LB_CAL_TIMEOUT);
		
	}
	Si324x_SetLinefeedStatusBroadcast(pProslic,LF_OPEN);
#ifdef DISABLE_LOOPMAN
#else
			enableSi324xLoopman(pProslic,TRUE);
#endif
	Si324x_SetLinefeedStatusBroadcast(pProslic,LF_FWD_ACTIVE); 
	return error;
}
int Si324x_GetLBCalResult (proslicChanType *pProslic,int32*result1,int32*result2,int32*result3,int32*result4){
	setUserMode(pProslic,TRUE);
	*result1 = ReadRAM(pProHW,pProslic->channel,1476);
	*result2 = ReadRAM(pProHW,pProslic->channel,1477);
	*result3 = ReadRAM(pProHW,pProslic->channel,1458);
	*result4 = ReadRAM(pProHW,pProslic->channel,1459);
	setUserMode(pProslic,FALSE);
	return 0;
}

/*
** Function: Si324x_GetLBCalResultPacked
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
int Si324x_GetLBCalResultPacked (proslicChanType *pProslic,int32 *result){
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


int Si324x_LoadPreviousLBCal (proslicChanType *pProslic,int32 result1,int32 result2,int32 result3, int32 result4){
	setUserMode(pProslic,TRUE);
	WriteRAM(pProHW,pProslic->channel,1476,result1);
	WriteRAM(pProHW,pProslic->channel,1477,result2);
	WriteRAM(pProHW,pProslic->channel,1458,result3);
	WriteRAM(pProHW,pProslic->channel,1459,result4);
	setUserMode(pProslic,FALSE);
	return 0;
}

/*
** Function: Si324x_LoadPreviousLBCalPacked
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
int Si324x_LoadPreviousLBCalPacked (proslicChanType *pProslic,int32 *result){
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


/*
** Function: PROSLIC_Cal
**
** Description: 
** Calibrates the ProSLIC
*/
int Si324x_Cal (proslicChanType_ptr *pProslic, int size){
	/*
	** This function will perform the ProSLIC calibration sequence (for all channels)
	*/
	int i,j=0;
	for (i=0;i<size;i++){
		if (pProslic[i]->channelEnable)
			j |= calibrate(pProslic[i],0);
	}
	return j;
}



/*
** Function: PROSLIC_LoadRegTables
**
** Description: 
** Loads registers and ram in the ProSLIC
*/
int Si324x_LoadRegTables (proslicChanType_ptr *pProslic, ProslicRAMInit *pRamTable, ProslicRegInit *pRegTable, int size){
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
int Si324x_LoadPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
	LoadQuadPatch(pProslic,pPatch,0);
	return 0;
}

/*
** Function: PROSLIC_VerifyPatch
**
** Description: 
** Verifiess patch to the ProSLIC
*/
int Si324x_VerifyPatch (proslicChanType *pProslic, const proslicPatch *pPatch){ 
	uInt32 loop; int jmp_table=82;
	uInt8 data; uInt32 ramdata;
	int err = 0;
	if (pPatch == NULL)
		return 0;
	setUserMode (pProslic,TRUE); /*make sure we are in user mode to load patch*/

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
			data = ReadReg (pProHW, pProslic->channel, (uInt8)jmp_table);
			if (data != ((pPatch->patchEntries[loop])&0xff))
				err = 1;
			data = ReadReg (pProHW, pProslic->channel, (uInt8)(jmp_table+1));
			if (data != (pPatch->patchEntries[loop]>>8))
				err = 1;
		}
		jmp_table+=2;
	}
 
	
	
	if (err){
#ifdef ENABLE_DEBUG
		if (pProslic->debugMode)
			LOGPRINT("Si324x Patch data corrupted: channel %d\n",pProslic->channel);
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
int Si324x_EnableInterrupts (proslicChanType_ptr pProslic){

	WriteReg (pProHW,pProslic->channel,IRQEN1,Si324x_General_Configuration.irqen1);
	WriteReg (pProHW,pProslic->channel,IRQEN2,Si324x_General_Configuration.irqen2);
	WriteReg (pProHW,pProslic->channel,IRQEN3,Si324x_General_Configuration.irqen3);
	WriteReg (pProHW,pProslic->channel,IRQEN4,Si324x_General_Configuration.irqen4);
	return 0;
}




int Si324x_SetLoopbackMode (proslicChanType_ptr pProslic, ProslicLoopbackModes newMode){
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
int Si324x_SetMuteStatus (proslicChanType_ptr pProslic, ProslicMuteModes muteEn){
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
*/

/*
** Function: PROSLIC_RingSetup
**
** Description: 
** configure ringing
*/
#ifdef DISABLE_RING_SETUP
#else
extern Si324x_Ring_Cfg Si324x_Ring_Presets[];
int Si324x_RingSetup (proslicChanType *pProslic, int preset){
	WriteReg(pProHW,pProslic->channel,RINGTAHI,Si324x_Ring_Presets[preset].tahi);
	WriteRAM(pProHW,pProslic->channel,RINGFR,Si324x_Ring_Presets[preset].freq);
	WriteRAM(pProHW,pProslic->channel,RINGAMP,Si324x_Ring_Presets[preset].amp);
	WriteRAM(pProHW,pProslic->channel,RINGOF,Si324x_Ring_Presets[preset].offset);
	WriteRAM(pProHW,pProslic->channel,RINGPHAS,Si324x_Ring_Presets[preset].phas);
	WriteReg(pProHW,pProslic->channel,RINGTALO,Si324x_Ring_Presets[preset].talo);
	WriteReg(pProHW,pProslic->channel,RINGTIHI,(Si324x_Ring_Presets[preset].tihi));
	WriteReg(pProHW,pProslic->channel,RINGTILO,(Si324x_Ring_Presets[preset].tilo));
	WriteReg(pProHW,pProslic->channel,RINGCON,(Si324x_Ring_Presets[preset].ringcon));
	WriteRAM(pProHW,pProslic->channel,RTPER,Si324x_Ring_Presets[preset].rtper);
	WriteRAM(pProHW,pProslic->channel,RTCOUNT,Si324x_Ring_Presets[preset].rtcount);
	WriteRAM(pProHW,pProslic->channel,ADAP_RING_MIN_I,Si324x_Ring_Presets[preset].adap_ring_min_i);
	WriteRAM(pProHW,pProslic->channel,SLOPE_RING,Si324x_Ring_Presets[preset].slope_ring);
	WriteRAM(pProHW,pProslic->channel,COUNTER_VTR_VAL,Si324x_Ring_Presets[preset].counter_vtr_val);
	WriteRAM(pProHW,pProslic->channel,RTACDB,Si324x_Ring_Presets[preset].rtacdb);
	WriteRAM(pProHW,pProslic->channel,RTACTH,Si324x_Ring_Presets[preset].rtacth);
	WriteRAM(pProHW,pProslic->channel,RTDCDB,Si324x_Ring_Presets[preset].rtdcdb);
	WriteRAM(pProHW,pProslic->channel,RTDCTH,Si324x_Ring_Presets[preset].rtdcth);
	WriteRAM(pProHW,pProslic->channel,VOV_RING_BAT,Si324x_Ring_Presets[preset].vov_ring_bat);
	WriteRAM(pProHW,pProslic->channel,VOV_RING_GND,Si324x_Ring_Presets[preset].vov_ring_gnd);
	WriteRAM(pProHW,pProslic->channel,VCM_RING,Si324x_Ring_Presets[preset].vcm_ring);
	
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
extern Si324x_Tone_Cfg Si324x_Tone_Presets[];
int Si324x_ToneGenSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,OSC1FREQ,Si324x_Tone_Presets[preset].osc1.freq);
	WriteRAM(pProHW,pProslic->channel,OSC1AMP,Si324x_Tone_Presets[preset].osc1.amp);
	WriteRAM(pProHW,pProslic->channel,OSC1PHAS,Si324x_Tone_Presets[preset].osc1.phas);
	WriteReg(pProHW,pProslic->channel,O1TAHI,(Si324x_Tone_Presets[preset].osc1.tahi));
	WriteReg(pProHW,pProslic->channel,O1TALO,(Si324x_Tone_Presets[preset].osc1.talo));
	WriteReg(pProHW,pProslic->channel,O1TIHI,(Si324x_Tone_Presets[preset].osc1.tihi));
	WriteReg(pProHW,pProslic->channel,O1TILO,(Si324x_Tone_Presets[preset].osc1.tilo));
	WriteRAM(pProHW,pProslic->channel,OSC2FREQ,Si324x_Tone_Presets[preset].osc2.freq);
	WriteRAM(pProHW,pProslic->channel,OSC2AMP,Si324x_Tone_Presets[preset].osc2.amp);
	WriteRAM(pProHW,pProslic->channel,OSC2PHAS,Si324x_Tone_Presets[preset].osc2.phas);
	WriteReg(pProHW,pProslic->channel,O2TAHI,(Si324x_Tone_Presets[preset].osc2.tahi));
	WriteReg(pProHW,pProslic->channel,O2TALO,(Si324x_Tone_Presets[preset].osc2.talo));
	WriteReg(pProHW,pProslic->channel,O2TIHI,(Si324x_Tone_Presets[preset].osc2.tihi));
	WriteReg(pProHW,pProslic->channel,O2TILO,(Si324x_Tone_Presets[preset].osc2.tilo));
	WriteReg(pProHW,pProslic->channel,OMODE,(Si324x_Tone_Presets[preset].omode));
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
extern Si324x_FSK_Cfg Si324x_FSK_Presets[];
int Si324x_FSKSetup (proslicChanType *pProslic, int preset){
	uInt8 data; 


	WriteReg(pProHW,pProslic->channel,O1TAHI,0);
	WriteReg(pProHW,pProslic->channel,O1TIHI,0);
	WriteReg(pProHW,pProslic->channel,O1TILO,0);
	WriteReg(pProHW,pProslic->channel,O1TALO,0x13);

	data = ReadReg(pProHW,pProslic->channel,OMODE);
	if (Si324x_FSK_Presets[preset].eightBit)
		data |= 0x80;
	else 
		data &= ~(0x80);
	WriteReg(pProHW,pProslic->channel,FSKDEPTH,Si324x_FSK_Presets[preset].fskdepth);
	WriteReg(pProHW,pProslic->channel,OMODE,data);
	WriteRAM(pProHW,pProslic->channel,FSK01,Si324x_FSK_Presets[preset].fsk01);
	WriteRAM(pProHW,pProslic->channel,FSK10,Si324x_FSK_Presets[preset].fsk10);
	WriteRAM(pProHW,pProslic->channel,FSKAMP0,Si324x_FSK_Presets[preset].fskamp0);
	WriteRAM(pProHW,pProslic->channel,FSKAMP1,Si324x_FSK_Presets[preset].fskamp1);
	WriteRAM(pProHW,pProslic->channel,FSKFREQ0,Si324x_FSK_Presets[preset].fskfreq0);
	WriteRAM(pProHW,pProslic->channel,FSKFREQ1,Si324x_FSK_Presets[preset].fskfreq1);
	return 0;
}
#endif

int Si324x_CheckCIDBuffer (proslicChanType *pProslic, uInt8 *fsk_buf_avail){
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
extern Si324x_DTMFDec_Cfg Si324x_DTMFDec_Presets[];
int Si324x_DTMFDecodeSetup (proslicChanType *pProslic, int preset){
	
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_1,Si324x_DTMFDec_Presets[preset].dtmfdtf_b0_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_1,Si324x_DTMFDec_Presets[preset].dtmfdtf_b1_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_1,Si324x_DTMFDec_Presets[preset].dtmfdtf_b2_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_1,Si324x_DTMFDec_Presets[preset].dtmfdtf_a1_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_1,Si324x_DTMFDec_Presets[preset].dtmfdtf_a2_1);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_2,Si324x_DTMFDec_Presets[preset].dtmfdtf_b0_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_2,Si324x_DTMFDec_Presets[preset].dtmfdtf_b1_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_2,Si324x_DTMFDec_Presets[preset].dtmfdtf_b2_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_2,Si324x_DTMFDec_Presets[preset].dtmfdtf_a1_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_2,Si324x_DTMFDec_Presets[preset].dtmfdtf_a2_2);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B0_3,Si324x_DTMFDec_Presets[preset].dtmfdtf_b0_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B1_3,Si324x_DTMFDec_Presets[preset].dtmfdtf_b1_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_B2_3,Si324x_DTMFDec_Presets[preset].dtmfdtf_b2_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A1_3,Si324x_DTMFDec_Presets[preset].dtmfdtf_a1_3);
	WriteRAM(pProHW,pProslic->channel,DTMFDTF_A2_3,Si324x_DTMFDec_Presets[preset].dtmfdtf_a2_3);
	return 0;
}
#endif
/*
** Function: PROSLIC_SetProfile
**
** Description: 
** set country profile of the proslic
*/
int Si324x_SetProfile (proslicChanType *pProslic, int preset){
	/*TO DO
	**Will be filled in at a later date
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
extern Si324x_Impedance_Cfg Si324x_Impedance_Presets [];
int Si324x_ZsynthSetup (proslicChanType *pProslic, int preset){
	uInt8 lf;
	lf = ReadReg(pProHW,pProslic->channel,LINEFEED);
	Si324x_SetLinefeedStatus(pProslic,LF_OPEN);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C0,Si324x_Impedance_Presets[preset].audioEQ.txaceq_c0);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C1,Si324x_Impedance_Presets[preset].audioEQ.txaceq_c1);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C2,Si324x_Impedance_Presets[preset].audioEQ.txaceq_c2);
	WriteRAM(pProHW,pProslic->channel,TXACEQ_C3,Si324x_Impedance_Presets[preset].audioEQ.txaceq_c3);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C0,Si324x_Impedance_Presets[preset].audioEQ.rxaceq_c0);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C1,Si324x_Impedance_Presets[preset].audioEQ.rxaceq_c1);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C2,Si324x_Impedance_Presets[preset].audioEQ.rxaceq_c2);
	WriteRAM(pProHW,pProslic->channel,RXACEQ_C3,Si324x_Impedance_Presets[preset].audioEQ.rxaceq_c3);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C2,Si324x_Impedance_Presets[preset].hybrid.ecfir_c2);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C3,Si324x_Impedance_Presets[preset].hybrid.ecfir_c3);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C4,Si324x_Impedance_Presets[preset].hybrid.ecfir_c4);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C5,Si324x_Impedance_Presets[preset].hybrid.ecfir_c5);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C6,Si324x_Impedance_Presets[preset].hybrid.ecfir_c6);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C7,Si324x_Impedance_Presets[preset].hybrid.ecfir_c7);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C8,Si324x_Impedance_Presets[preset].hybrid.ecfir_c8);
	WriteRAM(pProHW,pProslic->channel,ECFIR_C9,Si324x_Impedance_Presets[preset].hybrid.ecfir_c9);
	WriteRAM(pProHW,pProslic->channel,ECIIR_B0,Si324x_Impedance_Presets[preset].hybrid.eciir_b0);
	WriteRAM(pProHW,pProslic->channel,ECIIR_B1,Si324x_Impedance_Presets[preset].hybrid.eciir_b1);
	WriteRAM(pProHW,pProslic->channel,ECIIR_A1,Si324x_Impedance_Presets[preset].hybrid.eciir_a1);
	WriteRAM(pProHW,pProslic->channel,ECIIR_A2,Si324x_Impedance_Presets[preset].hybrid.eciir_a2);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A1,Si324x_Impedance_Presets[preset].zsynth.zsynth_a1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_A2,Si324x_Impedance_Presets[preset].zsynth.zsynth_a2);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B1,Si324x_Impedance_Presets[preset].zsynth.zsynth_b1);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B0,Si324x_Impedance_Presets[preset].zsynth.zsynth_b0);
	WriteRAM(pProHW,pProslic->channel,ZSYNTH_B2,Si324x_Impedance_Presets[preset].zsynth.zsynth_b2);
	WriteReg(pProHW,pProslic->channel,RA,Si324x_Impedance_Presets[preset].zsynth.ra);
	WriteRAM(pProHW,pProslic->channel,TXACGAIN,Si324x_Impedance_Presets[preset].txgain.acgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN,Si324x_Impedance_Presets[preset].rxgain.acgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN_SAVE,Si324x_Impedance_Presets[preset].rxgain.acgain);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_B0_1,Si324x_Impedance_Presets[preset].rxachpf_b0_1);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_B1_1,Si324x_Impedance_Presets[preset].rxachpf_b1_1);
	WriteRAM(pProHW,pProslic->channel,RXACHPF_A1_1,Si324x_Impedance_Presets[preset].rxachpf_a1_1);
	Si324x_SetLinefeedStatus(pProslic,lf);
	return 0;
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
extern Si324x_CI_Cfg Si324x_CI_Presets [];
int Si324x_GciCISetup (proslicChanType *pProslic, int preset){
	WriteReg(pProHW,pProslic->channel,GCI_CI,Si324x_CI_Presets[preset].gci_ci);
	return 0;
}
#endif
/*
** Function: PROSLIC_ModemDetSetup
**
** Description: 
** configure modem detector
*/
int Si324x_ModemDetSetup (proslicChanType *pProslic, int preset){
	/*TO DO
	**Will be filled in at a later date
	*/
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
extern Si324x_audioGain_Cfg Si324x_audioGain_Presets[];
int Si324x_TXAudioGainSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,TXACGAIN,Si324x_audioGain_Presets[preset].acgain);
	return 0;
}

/*
** Function: PROSLIC_AudioGainSetup
**
** Description: 
** configure audio gains
*/
int Si324x_RXAudioGainSetup (proslicChanType *pProslic, int preset){
	WriteRAM(pProHW,pProslic->channel,RXACGAIN,Si324x_audioGain_Presets[preset].acgain);
	WriteRAM(pProHW,pProslic->channel,RXACGAIN_SAVE,Si324x_audioGain_Presets[preset].acgain);
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
extern Si324x_DCfeed_Cfg Si324x_DCfeed_Presets[];
int Si324x_DCFeedSetup (proslicChanType *pProslic, int preset){
	uInt8 lf;
	lf = ReadReg(pProHW,pProslic->channel,LINEFEED);
	Si324x_SetLinefeedStatus(pProslic,LF_OPEN);
	WriteRAM(pProHW,pProslic->channel,SLOPE_VLIM,Si324x_DCfeed_Presets[preset].slope_vlim);
	WriteRAM(pProHW,pProslic->channel,SLOPE_RFEED,Si324x_DCfeed_Presets[preset].slope_rfeed);
	WriteRAM(pProHW,pProslic->channel,SLOPE_ILIM,Si324x_DCfeed_Presets[preset].slope_ilim);
	WriteRAM(pProHW,pProslic->channel,SLOPE_DELTA1,Si324x_DCfeed_Presets[preset].delta1);
	WriteRAM(pProHW,pProslic->channel,SLOPE_DELTA2,Si324x_DCfeed_Presets[preset].delta2);
	WriteRAM(pProHW,pProslic->channel,V_VLIM,Si324x_DCfeed_Presets[preset].v_vlim);
	WriteRAM(pProHW,pProslic->channel,V_RFEED,Si324x_DCfeed_Presets[preset].v_rfeed);
	WriteRAM(pProHW,pProslic->channel,V_ILIM,Si324x_DCfeed_Presets[preset].v_ilim);
	WriteRAM(pProHW,pProslic->channel,CONST_RFEED,Si324x_DCfeed_Presets[preset].const_rfeed);
	WriteRAM(pProHW,pProslic->channel,CONST_ILIM,Si324x_DCfeed_Presets[preset].const_ilim);
	WriteRAM(pProHW,pProslic->channel,I_VLIM,Si324x_DCfeed_Presets[preset].i_vlim);
	WriteRAM(pProHW,pProslic->channel,LCRONHK,Si324x_DCfeed_Presets[preset].lcronhk);
	WriteRAM(pProHW,pProslic->channel,LCROFFHK,Si324x_DCfeed_Presets[preset].lcroffhk);
	WriteRAM(pProHW,pProslic->channel,LCRDBI,Si324x_DCfeed_Presets[preset].lcrdbi);
	WriteRAM(pProHW,pProslic->channel,LONGHITH,Si324x_DCfeed_Presets[preset].longhith);
	WriteRAM(pProHW,pProslic->channel,LONGLOTH,Si324x_DCfeed_Presets[preset].longloth);
	WriteRAM(pProHW,pProslic->channel,LONGDBI,Si324x_DCfeed_Presets[preset].longdbi);
	WriteRAM(pProHW,pProslic->channel,LCRMASK,Si324x_DCfeed_Presets[preset].lcrmask);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_POLREV,Si324x_DCfeed_Presets[preset].lcrmask_polrev);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_STATE,Si324x_DCfeed_Presets[preset].lcrmask_state);
	WriteRAM(pProHW,pProslic->channel,LCRMASK_LINECAP,Si324x_DCfeed_Presets[preset].lcrmask_linecap);
	WriteRAM(pProHW,pProslic->channel,VCM_OH,Si324x_DCfeed_Presets[preset].vcm_oh);
	WriteRAM(pProHW,pProslic->channel,VOV_BAT,Si324x_DCfeed_Presets[preset].vov_bat);
	WriteRAM(pProHW,pProslic->channel,VOV_GND,Si324x_DCfeed_Presets[preset].vov_gnd);
	Si324x_SetLinefeedStatus(pProslic,lf);
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
extern Si324x_GPIO_Cfg Si324x_GPIO_Configuration ;
int Si324x_GPIOSetup (proslicChanType *pProslic){
	uInt8 data;
	data = ReadReg(pProHW,pProslic->channel,GPIO);
	data |= Si324x_GPIO_Configuration.outputEn << 4;
	WriteReg(pProHW,pProslic->channel,GPIO,data);
	data = Si324x_GPIO_Configuration.analog << 4;
	data |= Si324x_GPIO_Configuration.direction;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG1,data);
	data = Si324x_GPIO_Configuration.manual << 4;
	data |= Si324x_GPIO_Configuration.polarity;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG2,data);
	data |= Si324x_GPIO_Configuration.openDrain;
	WriteReg(pProHW,pProslic->channel,GPIO_CFG3,data);
	
	return 0;
}
#endif

/*
** Function: PROSLIC_PulseMeterSetup
**
** Description: 
** configure pulse metering
*/
#ifdef DISABLE_PULSE_SETUP
#else
extern Si324x_PulseMeter_Cfg Si324x_PulseMeter_Presets [];
int Si324x_PulseMeterSetup (proslicChanType *pProslic, int preset){
	uInt8 reg;
	WriteRAM(pProHW,pProslic->channel,PM_AMP_THRESH,Si324x_PulseMeter_Presets[preset].pm_amp_thresh);
	reg = (Si324x_PulseMeter_Presets[preset].pmFreq<<1) | (Si324x_PulseMeter_Presets[preset].pmRampRate<<4);
	WriteReg(pProHW,pProslic->channel,PMCON,reg);

	return 0;
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
extern Si324x_PCM_Cfg Si324x_PCM_Presets [];
int Si324x_PCMSetup (proslicChanType *pProslic, int preset){
	uInt8 regTemp;

	regTemp = Si324x_PCM_Presets[preset].pcmFormat;
	regTemp |= Si324x_PCM_Presets[preset].pcm_tri << 5;
	WriteReg(pProHW,pProslic->channel,PCMMODE,regTemp);
	regTemp = ReadReg(pProHW,pProslic->channel,PCMTXHI);
	regTemp &= 3;
	regTemp |= Si324x_PCM_Presets[preset].tx_edge<<4;
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
int Si324x_PCMTimeSlotSetup (proslicChanType *pProslic, uInt16 rxcount, uInt16 txcount){
	uInt8 data;
	data = (uInt8)(txcount & 0xff);
	WriteReg(pProHW,pProslic->channel,PCMTXLO,data);
	data = (uInt8)(txcount >> 8) ;
	WriteReg(pProHW,pProslic->channel,PCMTXHI,data);
	data = (uInt8)(rxcount & 0xff);
	WriteReg(pProHW,pProslic->channel,PCMRXLO,data);
	data = (uInt8)(rxcount >> 8);
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
int Si324x_GetInterrupts (proslicChanType *pProslic,proslicIntType *pIntData){
	/*Reading the interrupt registers and will clear any bits which are set (SPI mode only)
	**Multiple interrupts may occur at once so bear that in mind when
	**writing an interrupt handling routine
	*/
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
					case OSC1_T1_SI324X:
						k=IRQ_OSC1_T1;
						break;
					case OSC1_T2_SI324X:
						k=IRQ_OSC1_T2;
						break;
					case OSC2_T1_SI324X:
						k=IRQ_OSC2_T1;
						break;
					case OSC2_T2_SI324X:
						k=IRQ_OSC2_T2;
						break;
					case RING_T1_SI324X:
						k=IRQ_RING_T1;
						break;
					case RING_T2_SI324X:
						k=IRQ_RING_T2;
						break;
					case LONG_STAT_SI324X:
						k=IRQ_LONG_STAT;
						break;
					case VBAT_SI324X:
						k=IRQ_VBAT;
						break;
					case RING_TRIP_SI324X:
						k=IRQ_RING_TRIP;
						break;
					case LOOP_STAT_SI324X:
						k=IRQ_LOOP_STATUS;
						break;
					case PQ1_SI324X:
						k=IRQ_PQ1;
						break;
					case PQ2_SI324X:
						k=IRQ_PQ2;
						break;
					case PQ3_SI324X:
						k=IRQ_PQ3;
						break;
					case PQ4_SI324X:
						k=IRQ_PQ4;
						break;
					case PQ5_SI324X:
						k=IRQ_PQ5;
						break;
					case PQ6_SI324X:
						k=IRQ_PQ6;
						break;
					case DTMF_SI324X:
						k=IRQ_DTMF;
						break;
					case INDIRECT_SI324X:
						k=IRQ_INDIRECT;
						break;
					case CM_BAL_SI324X:
						k=IRQ_CM_BAL;
						break;
					case FSKBUF_AVAIL_SI324X:
						k = IRQ_FSKBUF_AVAIL;
						break;
					case VOC_TRACK_SI324X:
						k = IRQ_VOC_TRACK;
						break;
					case TXMDM_SI324X:
						k = IRQ_TXMDM;
						break;
					case RXMDM_SI324X:
						k = IRQ_RXMDM;
						break;
					case RING_FAIL_SI324X:
						k = IRQ_RING_FAIL;
						break;
					case USER_IRQ0_SI324X:
						k = IRQ_USER_0;
						break;
					case USER_IRQ1_SI324X:
						k = IRQ_USER_1;
						break;
					case USER_IRQ2_SI324X:
						k = IRQ_USER_2;
						break;
					case USER_IRQ3_SI324X:
						k = IRQ_USER_3;
						break;
					case USER_IRQ4_SI324X:
						k = IRQ_USER_4;
						break;
					case USER_IRQ5_SI324X:
						k = IRQ_USER_5;
						break;
					case USER_IRQ6_SI324X:
						k = IRQ_USER_6;
						break;
					case USER_IRQ7_SI324X:
						k = IRQ_USER_7;
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
int Si324x_ReadHookStatus (proslicChanType *pProslic,uInt8 *pHookStat){
	if (ReadReg(pProHW,pProslic->channel,LCRRTP) & 2)
		*pHookStat=OFFHOOK;
	else
		*pHookStat=ONHOOK;
	return 0;
}

/*
** Function: Si324x_SetLinefeedStatus
**
** Description: 
** Sets linefeed state
*/
int Si324x_SetLinefeedStatus (proslicChanType *pProslic, uInt8 newLinefeed){
#ifdef NO_VBATL /* This is set when there is no VBATL connection so the   */
                /* VBAT pin does not float in the open state, which could */
                /* be a problem when using tracking voltage protection.   */
	setUserMode(pProslic,TRUE);
	if (newLinefeed == LF_OPEN) 
	{
		WriteRAM(pProHW,pProslic->channel,1430,0x200000L); /*PD_HVIC*/
		WriteReg (pProHW, pProslic->channel, LINEFEED,LF_OPEN);
		WriteRAM(pProHW,pProslic->channel,1449,0x10000000L); /*HVIC_STATE_MAN*/
		WriteRAM(pProHW,pProslic->channel,1447,0x400000L); /*Close S1*/
		WriteReg (pProHW,pProslic->channel, 98, 0x80); /*PU_MADC*/
	} else
	{
		WriteRAM(pProHW,pProslic->channel,1430,0x400000L); /*PD_HVIC*/
		WriteRAM(pProHW,pProslic->channel,1447,0xF0000L); /*Set normal mode so we don't transistion to active state w/ linefeed tristated */
		WriteRAM(pProHW,pProslic->channel,1449,0x0L); /*HVIC_STATE_MAN*/
		WriteReg (pProHW, pProslic->channel, LINEFEED,newLinefeed);
	}
#else
	WriteReg (pProHW, pProslic->channel, LINEFEED,newLinefeed);
#endif

	return 0;
}

/*
** Function: Si324x_SetLinefeedStatusBroadcast
**
** Description: 
** Sets linefeed state
*/
int Si324x_SetLinefeedStatusBroadcast (proslicChanType *pProslic, uInt8 newLinefeed){
#ifdef NO_VBATL /* This is set when there is no VBATL connection so the   */
                /* VBAT pin does not float in the open state, which could */
                /* be a problem when using tracking voltage protection.   */
	setUserMode(pProslic,TRUE);
	if (newLinefeed == LF_OPEN) 
	{
		WriteRAM(pProHW,BROADCAST,1430,0x200000L); /*PD_HVIC*/
		WriteReg (pProHW, BROADCAST, LINEFEED,LF_OPEN);
		WriteRAM(pProHW,BROADCAST,1449,0x10000000L); /*HVIC_STATE_MAN*/
		WriteRAM(pProHW,BROADCAST,1447,0x400000L); /*Close S1*/
		WriteReg (pProHW,BROADCAST, 98, 0x80); /*PU_MADC*/
	} else
	{
		WriteRAM(pProHW,BROADCAST,1430,0x400000L); /*PD_HVIC*/
		WriteRAM(pProHW,BROADCAST,1447,0xF0000L); /*Set normal mode so we don't transistion to active state w/ linefeed tristated */
		WriteRAM(pProHW,BROADCAST,1449,0x0L); /*HVIC_STATE_MAN*/
		WriteReg (pProHW, BROADCAST, LINEFEED,newLinefeed);
	}
#else
	WriteReg (pProHW, BROADCAST, LINEFEED,newLinefeed);
#endif

	return 0;
}

/*
** Function: PROSLIC_PolRev
**
** Description: 
** Sets polarity reversal state
*/
int Si324x_PolRev (proslicChanType *pProslic,uInt8 abrupt, uInt8 newPolRevState){
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
int Si324x_GPIOControl (proslicChanType *pProslic,uInt8 *pGpioData, uInt8 read){
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
int Si324x_MWI (proslicChanType *pProslic,uInt8 lampOn){
	/*TO DO
	**Will be filled in at a later date
	*/
	return 0;
}

/*
** Function: PROSLIC_StartGenericTone
**
** Description: 
** start tone generators
*/
int Si324x_ToneGenStart (proslicChanType *pProslic,uInt8 timerEn){
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
int Si324x_ToneGenStop (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT("Si324x ToneGenStop\n");
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
int Si324x_RingStart (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_RingStart\n");
#endif
	Si324x_SetLinefeedStatus(pProslic,LF_RINGING);
	return 0;
}


/*
** Function: PROSLIC_StopRing
**
** Description: 
** Stops ring generator
*/
int Si324x_RingStop (proslicChanType *pProslic){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_RingStop\n");
#endif
	Si324x_SetLinefeedStatus(pProslic,LF_FWD_ACTIVE);
	return 0;
}

/*
** Function: PROSLIC_EnableCID
**
** Description: 
** enable fsk
*/
int Si324x_EnableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_EnableCID\n");
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
int Si324x_DisableCID (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_DisableCID\n");
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
int Si324x_SendCID (proslicChanType *pProslic, uInt8 *buffer, uInt8 numBytes){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_SendCID\n");
#endif
	while (numBytes-- > 0){
		WriteReg(pProHW,pProslic->channel,FSKDAT,*(buffer++));
	}
	return 0;
}

/*
** Function: PROSLIC_StartPCM
**
** Description: 
** Starts PCM
*/
int Si324x_PCMStart (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_PCMStart\n");
#endif
	data = ReadReg(pProHW,pProslic->channel,PCMMODE);
	data |= 0x10;
	WriteReg(pProHW,pProslic->channel,PCMMODE,data);
	return 0;
}


/*
** Function: PROSLIC_StopPCM
**
** Description: 
** Disables PCM
*/
int Si324x_PCMStop (proslicChanType *pProslic){
	uInt8 data;
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_PCMStop\n");
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
int Si324x_DTMFReadDigit (proslicChanType *pProslic,uInt8 *pDigit){
#ifdef ENABLE_DEBUG
	if (pProslic->debugMode)
		LOGPRINT ("Si324x_DTMFReadDigit\n");
#endif
	*pDigit = ReadReg(pProHW,pProslic->channel,TONDTMF) & 0xf;
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStart
**
** Description: 
** initiates pll free run mode
*/
int Si324x_PLLFreeRunStart (proslicChanType *pProslic){
	/*not applicable for this part number*/
	return 0;
}

/*
** Function: PROSLIC_PLLFreeRunStop
**
** Description: 
** exit pll free run mode
*/
int Si324x_PLLFreeRunStop (proslicChanType *pProslic){
	/*not applicable for this part number*/
	return 0;
}

/*
** Function: PROSLIC_PulseMeterStart
**
** Description: 
** start pulse meter tone
*/
int Si324x_PulseMeterStart (proslicChanType *pProslic){
	WriteReg(pProHW,pProslic->channel,PMCON,ReadReg(pProHW,pProslic->channel,PMCON) | (0x5));
	return 0;	
}

/*
** Function: PROSLIC_PulseMeterStop
**
** Description: 
** stop pulse meter tone
*/
int Si324x_PulseMeterStop (proslicChanType *pProslic){
	WriteReg(pProHW,pProslic->channel,PMCON,ReadReg(pProHW,pProslic->channel,PMCON) & ~(0x5));
	return 0;
}
typedef struct
{
    int32   gain;
    uInt32 scale;
} ProSLIC_GainScaleLookup;


static	const ProSLIC_GainScaleLookup relGainTable[] =
/*  gain, scale=10^(gain/20)*262144 (RXGAIN), for TXGain, this needs to be shifted by 2 bits 
    see AN248, equations 69 & 70
*/
    {
        {-12,0x809BCC},
        {-11,0x904D1B},
        {-10,0xA1E89B},
        {-9,0xB5AA19},
        {-8,0xCBD4B3},
        {-7,0xE4B3B6},
        {-6,0x1009B9C},
        {-5,0x11FEB33},
        {-4,0x1430CD7},
        {-3,0x16A77DE},
        {-2,0x196B230},
        {-1,0x1C8520A},
        {0,0x2000000},
        {1,0x23E7933},
        {2,0x28491DF},
        {3,0x2D3381E},
        {4,0x32B771E},
        {5,0x38E7AA3},
        {6,0x3FD9305},
        {0xff,0}/* terminator */
    };
static int Si324x_dbgSetGain(int32 requested_gain, int32 impedance_gain, int scale_multiplier, Si324x_audioGain_Cfg *cfg)
{
	int errVal = 0;
	int32 i;
	int gain_delta;

    /* What relative gain do we need? */
	gain_delta = requested_gain - impedance_gain;

    if( gain_delta < relGainTable[0].gain )
    {
        errVal = RC_GAIN_DELTA_TOO_LARGE;
        gain_delta = relGainTable[0].gain;
    }

    if( requested_gain < (relGainTable[0].gain + impedance_gain) )
    {
        errVal = RC_GAIN_OUT_OF_RANGE;
        requested_gain = relGainTable[0].gain + impedance_gain;
    }
   
    for(i = 0 ; relGainTable[i].gain != 0xFF; i++)
    {
        if( gain_delta == relGainTable[i].gain )
        {
            cfg->acgain = (relGainTable[i].scale)*scale_multiplier;
            return errVal;
        }
    }
    
    /* We exceeded the maximum gain delta, did we do so because we've exceeded 
       the max gain or just the delta?
    */
    if( requested_gain > (relGainTable[i-1].gain+impedance_gain))
    {
        errVal = RC_GAIN_OUT_OF_RANGE;
    }
    else
    {
        errVal = RC_GAIN_DELTA_TOO_LARGE;
    }

    return errVal;
}

/*
** Function: Si3224_dbgSetTXGain
**
** Description: 
** Provisionary function for setting up
** TX gain
*/
int Si324x_dbgSetTXGain (proslicChanType *pProslic, int32 gain, 
    int impedance_preset, int audio_gain_preset)
{
    return(Si324x_dbgSetGain(gain, 
        SI324X_DEFAULT_IMPEDANCE_TXGAIN,4,
        &(Si324x_audioGain_Presets[audio_gain_preset])));
}

/*
** Function: Si3224_dbgSetRXGain
**
** Description: 
** Provisionary function for setting up
** RX gain
*/
int Si324x_dbgSetRXGain(proslicChanType *pProslic, int32 gain, 
    int impedance_preset, int audio_gain_preset)
{
    return(Si324x_dbgSetGain(gain, SI324X_DEFAULT_IMPEDANCE_RXGAIN,1,
        &(Si324x_audioGain_Presets[audio_gain_preset])));
}


/*
** Function: Si324x_AudioGainSetup
**
** Description: 
** Set audio gain of RX and TX paths - presumed that
** all zsynth coefficient presets are 0dB
**
*/

int Si324x_AudioGainSetup(proslicChanType *pProslic, int32 rxgain, int32 txgain, int preset)
{

    Si324x_dbgSetTXGain(pProslic,txgain,preset,0);
    Si324x_dbgSetRXGain(pProslic,rxgain,preset,1);
    Si324x_TXAudioGainSetup(pProslic,0);
    Si324x_RXAudioGainSetup(pProslic,1);
   
    return 0;
}

/*
** Function: Si324x_LineMonitor
**
** Description: 
** Monitor line voltages and currents
*/
int Si324x_LineMonitor(proslicChanType *pProslic, proslicMonitorType *monitor)
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

