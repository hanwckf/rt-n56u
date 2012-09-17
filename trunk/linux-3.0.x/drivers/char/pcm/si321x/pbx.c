/*
** $Id: pbx.c,v 1.6 2010-03-22 02:10:11 qwert Exp $
**
** pbx.c
** PBX state machine demo
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This file implements a call state machine to demonstrate the features of the ProSLIC
**
** Dependancies:
** ProSLIC.c,system.c
**
*/
//#include "stdio.h"
#include <linux/delay.h>
#include "../pcm_ctrl.h"
#include "proslic_api/ralink/proslic_ppt_mb/spi.h"
#include "proslic_api/proslic.h"
#include "pbx.h"
#include "si321x_constants.h"

#define VERSIONSTR "4.6"
#define NOCONNECT 0xff

#define PRINTF_IS_OK	1
#define SLIC_MSG(fmt, args...) printk("SLIC_API: " fmt, ## args)
#define printf printk

typedef void(*pt2Func)(chanState*);

int widebandEn[NUMBER_OF_CHAN];
/*
** struct used to represent phone numbers in directory
*/
typedef struct  {			
	char * phoneNumber ; //number to dial
	char * functionName; //function displayed in the menu
	pt2Func action; //pointer to actual function that is called
}phoneNumberType;

/*
** Local function prototypes
*/
static void InitStateMachine (chanState *pState); 
static void channelDemo (chanState *pState, systemTimer_S *timerObj, BOOLEAN *quit_b); 
static void handleInterrupts (chanState *pState,proslicIntType *irqs); 
static void newDigit (chanState *pState, uInt8 asciiChar);
static pt2Func findNumber(chanState *pState); 
static void processDTMF(chanState *pState);
static void processDigit(chanState *pState,uInt8 rawDigit);
static void connectionManager (chanState *pthisChan,chanState *pOtherChan);
static void printMenu();
//these are the states in the state machine
static void OnHook (chanState *pState, ProslicInt eInput);
static void OffHook (chanState *pState, ProslicInt eInput);
static void DialTone (chanState *pState, ProslicInt eInput);
static void RingbackTone (chanState *pState, ProslicInt eInput);
static void Ringing (chanState *pState, ProslicInt eInput);
static void SignalingTone (chanState *pState, ProslicInt eInput);
static void DigitDecoding (chanState *pState, ProslicInt eInput);
static void FirstRing (chanState *pState,ProslicInt eInput);
static void pulseDialOnHk (chanState *pState, ProslicInt eInput);
static void pulseDialOffHk (chanState *pState, ProslicInt eInput);
static void callBack (chanState *pState, ProslicInt eInput);
static void exit (chanState *pState,ProslicInt eInput);
//these are transistions between states
static void goOnHookState (chanState *pState);
static void goOffHookState (chanState *pState);
static void powerAlarm (chanState *pState);
static void stopRingingGoOnHk (chanState *pState);
static void goToPulseDialOnHkState (chanState *pState);
static void goToPulseDialOffHkState (chanState *pState);
//various telephony functions
static void callDoBusy(chanState *pState);
static void callDoRingback (chanState *pState);
static void callDoRinging (chanState *pState);
static void callDoCongestion (chanState *pState);
static void callDoReorder (chanState *pState);
//setup pbx call
static void callDoCall (chanState *pState);
static void callDoCallBack (chanState *pState);
//exits the demo
static void callDoExit (chanState *pState);
static void DoCallerID (proslicChanType *pProslic);

static void callDoWideband (chanState *pState);
static void callDoNarrowband (chanState *pState);


/*
** this is the phonebook: a set of possible number to dial for this demo
*/
static const phoneNumberType phoneNumbers[] = {		
#if (NUMBER_OF_CHAN > 1)	
	{ "11\0","Call phone 1", callDoCall},
	{ "12\0","Call phone 2", callDoCall},
#endif
	{ "19\0","Call back", callDoCallBack},
	{ "3210\0","Busy Tone", callDoBusy},
	{ "222\0", "Quit Program", callDoExit},
	{ "3211\0", "Ring Back Tone",callDoRingback},
	{ "555\0", "Congestion Tone",callDoCongestion},
	//{ "411", "GR909 Menu",GR909menu},

	{"911\0", "Busy Tone",callDoBusy},
	{"81\0", "Turn on wideband (Si3216)", callDoWideband},
	{"82\0", "Turn off wideband (Si3216)", callDoNarrowband},
	{"\0", " Any Other Number=>Reorder Tone", callDoReorder},
};
/*
** Function: printChipDetails
**
** Description: 
** prints details of EVB
**
** Input Parameters: 
** pProslic: pointer to proslic data structure 
**
** Return:
** none
*/
static void printChipDetails(proslicChanType *pProslic)
{
	char* name = "";

	uInt8 data;
//	char* freqs[ ] = {"8192","4028","2048","1024","512","256","1536","768","32768"};

	//--------------------- NAME -----------------------------
	data = pProslic->deviceId->chipType;
	switch(data){
		case SI3210:
			printf("Si3210");
			break;
		case SI3215:
			printf("Si3215-16");
			break;
		case SI3211:
			printf("Si3211");
			break;
		case SI3212:
			printf("Si3212");
			break;
		case SI3210M:
			printf("Si3210 M");
			break;
		case SI3215M:
			printf("Si3215 M");
			break;
		case SI3216M:
			printf("Si3216 M");
			break;
	}
	//-------------------END OF NAME -------------------------

	//---------------------FREQUENCY--------------------------
//	ReadReg(pProHW, pProslic->channel, 1, 13, 0, &data);  /* Read the frequency */
//	data = data>>4;
	//-----------------END OF FREQUENCY-----------------------

	printf("%s Rev %c\n",name, pProslic->deviceId->chipRev);
	//printf("(%s KHz)\n", freqs[data]);
}

/*
** Function: printMenu
**
** Description: 
** prints phone directory for user
**
** Input Parameters: 
** 
** Return:
** none
*/

static void printMenu(){
	int	i=0;
#if(PRINTF_IS_OK)
	
	printf("\n\n\t \t \t P H O N E  D I R E C T O R Y\n ");
	while (*phoneNumbers[i].phoneNumber){
		printf("\n \t %s \t %s",phoneNumbers[i].phoneNumber,phoneNumbers[i].functionName);
		i++;
	}
	printf("\n\n\t \t \t P H O N E  D I R E C T O R Y\n\n ");
	for (i=0;i<NUMBER_OF_CHAN;i++){
		printf("\t\t\tChannel %d Wideband = %d\n",i,widebandEn[i]);
	}
	printf("\n");
#endif
}

static void processDigit(chanState *pState,uInt8 rawDigit){  
	//calls newDigit to insert into buffer after converting to ascii
	
	uInt8 asciiChar;
	switch (rawDigit){
	case 0xA :
		asciiChar = '0';
	break;
	case 0xB:
		asciiChar = '*';
	break;
	case 0xC:
		asciiChar = '#';
	break;

	default:
		asciiChar= '0' + rawDigit;
	break;
	}
	newDigit (pState,asciiChar);
}

/*
** Function: processDTMF
**
** Description: 
** Called whenever new DTMF digit is expected. Reads digit from Proslic.
**
**
** Input Parameters: 
** pState: pointer to channel state data structure 
**
** Return:
** none
*/
static void processDTMF(chanState *pState){  
	//collect dtmf digit and calls newDigit to insert into buffer
	
	uInt8 rawDigit;
	ProSLIC_DTMFReadDigit(pState->ProObj,&rawDigit);
	
	processDigit(pState,rawDigit); //convert to ascii and insert into buffer
}


/*
** Function: newDigit
**
** Description: 
** Called whenever new pulse digit or DTMF digit is collected
** Digit is added to digit buffer and findNumber is called to search for a match
** in the 'phonebook'
**
** Input Parameters: 
** pState: pointer to channel state data structure 
** asciiChar: digit pressed on the phone
**
** Return:
** none
*/
static void newDigit (chanState *pState, uInt8 asciiChar){  
pt2Func funct;
funct = NULL;
	if (pState->digitCount < 19)
		 {
			pState->digits[pState->digitCount] = asciiChar; 
			pState->digitCount++;
			pState->digits[pState->digitCount]= 0;
			#if(PRINTF_IS_OK)
			printf("Value= %c  String collected \"%s\" ", asciiChar, pState->digits );	
            #endif
			funct =	findNumber(pState);
			if (funct) funct(pState);
		 }
}


/*
** Function: findNumber
**
** Description: 
** Checks the digit buffer to see if it matches anything in the 'phonebook'
**
** Input Parameters: 
** pState: pointer to channel state data structure 
**
** Return:
** pointer to function assigned to tthe phone number (if found)
** NULL if phone number is not found
*/
static pt2Func findNumber(chanState *pState){
	//checks to see if there is a match for entered number in the directory above
	//if there is returns function pointer to next action
	//if there is partial match NULL is returned
	//if no match possible returns next action reorder tone
	uInt8 * checkDigit; 
	const phoneNumberType *menuRow;
	uInt8 * bookDigit;

	menuRow = &(phoneNumbers[0]);	

	while (menuRow->phoneNumber[0] != 0 ){ //go through rows of phonebook

		bookDigit = &(menuRow->phoneNumber[0]);
		checkDigit = &(pState->digits[0]);
		while ((*checkDigit == *bookDigit) && (*bookDigit!=0)){
			checkDigit++;
			bookDigit++;
		}
		if (*bookDigit == 0){		//if we get to the end of the phonebook string we must have match
			pState->digitCount=0;
			return (menuRow->action);
		}
		if (*checkDigit == 0) //ran out of DTMF digits (partial match)
			return NULL;

		menuRow++;
	}

	pState->digitCount=0;
	return &callDoReorder ;
}


/*
** Function: PBX_Demo
**
** Description: 
** PBX State Machine
**
** Input Parameters: 
** 
** Return:
** none
*/
uInt16 si3210_init (void){
	ctrl_S spiGciObj; //spi interface object
	systemTimer_S timerObj;    //timer object
	controlInterfaceType *ProHWIntf; //hw interface
	ProslicDeviceType *ProSLICDevices[NUMBER_OF_PROSLIC]; //proslic device object
	proslicChanType_ptr arrayOfProslicChans[NUMBER_OF_CHAN]; //used for initialization only
	chanState ports[NUMBER_OF_CHAN];  //declare channel state structures
	uInt8 i=0;
	BOOLEAN keepRunning = TRUE;
	proslicChanType *pSlic;

#if (PRINTF_IS_OK)
	printf("Hang up phone!\n");
#endif
	//initialize timer
	TimerInit(&timerObj);  
	ProSLIC_createControlInterface(&ProHWIntf);
	for (i=0;i<NUMBER_OF_PROSLIC;i++)
		ProSLIC_createDevice (&(ProSLICDevices[i]));
	for (i=0;i<NUMBER_OF_CHAN;i++){
		widebandEn[i]=0;
		ProSLIC_createChannel(&(ports[i].ProObj));
		ProSLIC_SWInitChan (ports[i].ProObj,i,SI321X_TYPE, ProSLICDevices[i], ProHWIntf);
	}
	ProSLIC_setControlInterfaceCtrlObj (ProHWIntf, &spiGciObj);
	ProSLIC_setControlInterfaceReset (ProHWIntf, ctrl_ResetWrapper);
	ProSLIC_setControlInterfaceWriteRegister (ProHWIntf, ctrl_WriteRegisterWrapper);
	ProSLIC_setControlInterfaceReadRegister (ProHWIntf, ctrl_ReadRegisterWrapper);		
	ProSLIC_setControlInterfaceWriteRAM (ProHWIntf, ctrl_WriteRAMWrapper);
	ProSLIC_setControlInterfaceReadRAM (ProHWIntf, ctrl_ReadRAMWrapper);
	ProSLIC_setControlInterfaceTimerObj (ProHWIntf, &timerObj);
	ProSLIC_setControlInterfaceDelay (ProHWIntf, time_DelayWrapper);
	ProSLIC_setControlInterfaceTimeElapsed (ProHWIntf, time_TimeElapsedWrapper);
	ProSLIC_setControlInterfaceGetTime (ProHWIntf, time_GetTimeWrapper);
	
	
#if (PRINTF_IS_OK)
	printf ("-------Si321x Rev ");
	printf (VERSIONSTR);
	printf (" PBX Demo Program------\n\n");
#endif


	ProSLIC_Reset((ports[0].ProObj));	//Reset the ProSLIC(s) before we begin
	
	//initialize SPI interface   
	if (SPI_Init (&spiGciObj) ==  FALSE){
		#if (PRINTF_IS_OK)
		printf ("Cannot connect\n");
		#endif
		return 0;	
	}
	
	//Initialize the channel state for each channel
	for (i=0;i<NUMBER_OF_CHAN;i++){
		arrayOfProslicChans[i] = (ports[i].ProObj); //create array of channel pointers (for broadcast init)
		ProSLIC_setSWDebugMode (ports[i].ProObj, 1);
	}
	ProSLIC_Init(arrayOfProslicChans,NUMBER_OF_CHAN);
	ProSLIC_RingSetup(ports[0].ProObj,USA_DEFAULT_RING);
	ProSLIC_SetLinefeedStatus(ports[0].ProObj,LF_FWD_ACTIVE);
	ProSLIC_EnableInterrupts(ports[0].ProObj);
	pSlic = (ports[0].ProObj);
	printChipDetails(pSlic);

	for (i=0;i<NUMBER_OF_CHAN;i++){
		pSlic = (ports[i].ProObj);
		InitStateMachine(&(ports[i])); //initialize the call state machine
		ProSLIC_InitializeDialPulseDetect(&(ports[i].pulseDialData),&(ports[i].offHookTime),&(ports[i].onHookTime));
		if(pSlic->error==SPIFAIL)
			return 0;
		pSlic->deviceId->ctrlInterface->getTime_fptr(pSlic->deviceId->ctrlInterface->hTimer,&(ports[i].offHookTime));
		pSlic->deviceId->ctrlInterface->getTime_fptr(pSlic->deviceId->ctrlInterface->hTimer,&(ports[i].onHookTime));
	}
	
		
	//printMenu();
	callDoRinging (&(ports[0]));
	{
		int nTry = 20;
		uInt8 hkStat;
		do{
			ProSLIC_ReadHookStatus(ports[0].ProObj,&hkStat);
			if (hkStat != ONHOOK){
				ProSLIC_ToneGenStop(ports[0].ProObj);
				break;
			}
			else {
				printf ("Waiting OnHOOK Event..%d\n", nTry);
			}
			nTry--;
			mdelay(300);
		}while(nTry>0);
	}

	
	ProSLIC_PCMTimeSlotSetup(ports[0].ProObj,1,1);
	
#if	defined(PCM_LINEAR)
	callDoWideband(&(ports[0]));
#else	
	callDoNarrowband(&(ports[0]));	
#endif	
	ProSLIC_PCMStart(ports[0].ProObj);
	ProSLIC_SetLinefeedStatus(ports[0].ProObj,LF_FWD_ACTIVE);
	//start the demo
/*
	while (keepRunning){
		for (i=0;i<NUMBER_OF_CHAN;i++){
			if (ports[i].ProObj->channelEnable){
				channelDemo(&(ports[i]),&timerObj,&keepRunning);  
				if (ports[i].connectionWith != NOCONNECT)
					connectionManager(&(ports[i]),&(ports[ports[i].connectionWith]));
			}
		}
	}
*/
	return 1;
}


/*
** Function: InitStateMachine 
**
** Description: 
** Initializes the state machine for each channel
**
** Input Parameters: 
** pState: pointer to channel state data structure 
**
** Return:
** none
*/
static void InitStateMachine (chanState *pState){	
	pState->currentState = OnHook;
	pState->pulseDialData.currentPulseDigit=0;
	pState->digitCount=0;
	pState->digits[0] = 0;
	pState->ringCount =0;
	pState->connectionWith=NOCONNECT;
	pState->eventEnable=TRUE;
	pState->powerAlarmCount=0;
}


/*
** Function: handleInterrupts
**
** Description: 
** Goes through all interrupt bits and inputs each interrupt to the state machine
**
** Input Parameters: 
** pState: pointer to channel state data structure 
** irqs: values in Proslic interrupt registers
**
** Return:
** none
*/
static void handleInterrupts (chanState *pState,proslicIntType *irqs){ 
	uInt8 i;
	int numIrq = irqs->number;
	if (pState->eventEnable == FALSE) //if channel is disabled ignore interrupts
		return;

	for (i=0; i<numIrq; i++){ //otherwise process each interrupt
			pState->currentState(pState,irqs->irqs[i]); //see ProSLIC.h for ProslicIntType
	}
	
		
}


/*
** Function: channelDemo
**
** Description: 
** called once per channel in a loop from PBX_Demo
** checks for interrupts in the ProSLIC and handles them by calling "handleInterrupts"
** also checks timers for pulse dial detection
**
** Input Parameters: 
** pState: pointer to channel state data structure 
**
** Return:
** none
** 
*/
pulseDial_Cfg pDCfg = {30, 150, 30, 150}; //relaxed pulse dial thresholds - pc timer is not accurate

static void channelDemo (chanState *pState, systemTimer_S *timerObj,BOOLEAN *quit_b){
	    uInt8 digit;
		proslicIntType irqs;
		ProslicInt arrayIrqs[MAX_PROSLIC_IRQS];
		irqs.irqs = arrayIrqs;

		if (pState->currentState == exit){
			*quit_b = FALSE;
			return;
		}

		if (ProSLIC_GetInterrupts (pState->ProObj,&irqs)) //check channel for pending interrupts
		{
			handleInterrupts (pState,&irqs); //deal with each interrupt
		}
        digit = ProSLIC_DialPulseDetectTimeout (pState->ProObj,&pDCfg,&(pState->pulseDialData));

		/*Pulse dial detect handling code start*/
		if (pState->currentState == pulseDialOnHk && digit==ON_HOOK_TIMEOUT){
			goOnHookState(pState);
			
		}
		if (pState->currentState == pulseDialOffHk && digit!=0){
				if (digit != ON_HOOK_TIMEOUT)
					processDigit(pState,digit);
		}
		/*Pulse dial detect handling code end*/
}


/*
** Function: connectionManager
**
** Description: 
** called if phone number match is to call another channel
** checks state of 2 channels trying to connect and sets up ringing/ringback/busy tone as needed
**
** Input Parameters: 
** pthisChan: pointer to channel state data structure for first channnel
** pOtherChan: pointer to channel state data structure for second channel
**
** Return:
** 
*/
static void connectionManager (chanState *pthisChan,chanState *pOtherChan){
	//this function manages PCM connections between two channels
	int rxcount,txcount;
	
	if (pthisChan->currentState == DigitDecoding || pthisChan->currentState == pulseDialOffHk){//dialed number found 
		if (pOtherChan->currentState == OnHook){ //if other phone not busy
			callDoRingback(pthisChan); //ring other phone and ring back this phone
			pOtherChan->connectionWith = pthisChan->ProObj->channel; //set connection variable for other phone
			callDoRinging (pOtherChan);			
		}
		else{
			pthisChan->connectionWith=NOCONNECT; //otherwise phone must be busy
			callDoBusy(pthisChan);
		}
	}
	else if (pOtherChan->currentState == RingbackTone){ //call answered
		if (pthisChan->currentState==OffHook){
			goOffHookState(pOtherChan); //stop ring back and set up PCM connection

			rxcount = 16 * pthisChan->ProObj->channel;
			txcount = 16 * pOtherChan->ProObj->channel;
			ProSLIC_PCMTimeSlotSetup(pOtherChan->ProObj,rxcount,txcount);

			rxcount = 16 * pOtherChan->ProObj->channel;
			txcount = 16 * pthisChan->ProObj->channel;
			ProSLIC_PCMTimeSlotSetup(pthisChan->ProObj,rxcount,txcount);

			ProSLIC_PCMStart(pOtherChan->ProObj);
			ProSLIC_PCMStart(pthisChan->ProObj);
		}
	}
	else if (pOtherChan->currentState==OnHook){ //other phone hung up
		pthisChan->connectionWith=NOCONNECT;
		ProSLIC_PCMStop(pthisChan->ProObj);
		ProSLIC_PCMStop(pOtherChan->ProObj);
		if (pthisChan->currentState == OffHook)
			callDoCongestion(pthisChan);
		else if (pthisChan->currentState == Ringing || pthisChan->currentState==FirstRing){
			stopRingingGoOnHk (pthisChan);
		}
	}

}

/*
* FUNCTIONS BELOW ARE CHANNEL STATES
*/

/*
** Function: onHook (state)
**
** Description: 
** called when current state is onHook and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void OnHook (chanState *pState, ProslicInt eInput){
	int tone = USA_DIAL_TONE;

	if (eInput==LOOP_STATUS){
		ProSLIC_ToneGenSetup(pState->ProObj,tone);
		ProSLIC_ToneGenStart(pState->ProObj,FALSE);
		printMenu();
		pState->digitCount=0;
		pState->currentState = DialTone;
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
}


/*
** Function: offHook (state)
**
** Description: 
** called when current state is offHook and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void OffHook (chanState *pState, ProslicInt eInput){
	//this is off hook as in on a call (phone picked up after ringing or call answered)
	if (eInput>=PQ1 && eInput<=PQ6 ){
		powerAlarm(pState);
	}
	else if (eInput==LOOP_STATUS){
		///pState->pulseDialData.currentPulseDigit = 0;
		//goOnHookState(pState);
		goToPulseDialOnHkState (pState);
	}
}


/*
** Function: dialtone (state)
**
** Description: 
** called when current state is dialtone and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void DialTone (chanState *pState, ProslicInt eInput){
	//this state is used when user picks up idle phone
	if (eInput==DTMF){ //user is dialing
		ProSLIC_ToneGenStop(pState->ProObj);
		pState->currentState = DigitDecoding;
		processDTMF(pState);
		
	}
	else if (eInput == LOOP_STATUS){
			ProSLIC_DialPulseDetect(pState->ProObj,&pDCfg,&(pState->pulseDialData));
			goToPulseDialOnHkState (pState);
			//goOnHookState(pState);
		
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState); 
	}

}


/*
** Function: pulseDialOnHk (state)
**
** Description: 
** called when current state is pulse dialing on hook and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/

static void pulseDialOnHk (chanState *pState, ProslicInt eInput){
   
   if (eInput == LOOP_STATUS){
		/*SYSTEM_TimeElapsed(pState->onHookTime,&breaktime);
       if (breaktime>PULSE_TH3 && breaktime<PULSE_TH1){
       		pState->currentPulseDigit++;*/
			ProSLIC_DialPulseDetect(pState->ProObj,&pDCfg,&(pState->pulseDialData));
       		goToPulseDialOffHkState(pState);
       		//}
       	
   }
   else if (eInput>=PQ1 && eInput<=PQ6){
      powerAlarm(pState); 
   } 
}


/*
** Function: pulseDialOffHk (state)
**
** Description: 
** called when current state is pulse dialing off hook and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void pulseDialOffHk (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS){
		ProSLIC_DialPulseDetect(pState->ProObj,&pDCfg,&(pState->pulseDialData));
	    goToPulseDialOnHkState(pState);
   }
   else if (eInput>=PQ1 && eInput<=PQ6){
      powerAlarm(pState); 
   } 
}

/*
** Function: callBack (state)
**
** Description: 
** called when current state is "call back is pending"
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void callBack (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS){
		callDoRinging (pState);	
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
      powerAlarm(pState); 
   } 
	
}
/*
** Function: ringbackTone (state)
**
** Description: 
** called when current state is ing back tone is being generated and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void RingbackTone (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS){
		goOnHookState(pState);
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
}



/*
** Function: FirstRing (state)
**
** Description: 
** called when current state is ring back tone is being generated and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void FirstRing (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS || eInput == RING_TRIP){
		goOffHookState(pState); //phone is answered
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
	else if (eInput == RING_T1){
		pState->ringCount=1;
		DoCallerID(pState->ProObj);
		pState->currentState = Ringing;
	}
}


/*
** Function: Ringing (state)
**
** Description: 
** called when current state is ringing and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void Ringing (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS || eInput == RING_TRIP ){
		goOffHookState(pState); //phone is answered
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
	else if (eInput == RING_T1){
		pState->ringCount++;
		if (pState->ringCount>6)
			stopRingingGoOnHk(pState);
	}
}


/*
** Function: SignalingTone (state)
**
** Description: 
** called when current state is a signaling tone is being generated and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void SignalingTone (chanState *pState, ProslicInt eInput){
	if (eInput == LOOP_STATUS){
		goOnHookState(pState);
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
	else if (eInput==OSC1_T1 || eInput==OSC1_T2 || eInput==OSC2_T1 || eInput==OSC2_T2){
#if (DEBUG)
		printf ("Oscillator timer interrupt\n");
#endif
	}
}


/*
** Function: DigitDecoding (state)
**
** Description: 
** called when current state is a DTMF being decoded and interrupt occurs
**
** Input Parameters: 
** pState: pointer to channel state data structure
** eInput: interrupt that has occured
**
** Return:
** 
*/
static void DigitDecoding (chanState *pState, ProslicInt eInput){
	//this is the state when user is pressing keys on phone (DTMF)
	if (eInput == DTMF){
		processDTMF(pState);
	}
	else if (eInput == LOOP_STATUS){
		goOnHookState(pState);
	}
	else if (eInput>=PQ1 && eInput<=PQ6){
		powerAlarm(pState);
	}
}

/*
* FUNCTIONS BELOW ARE TRANSITIONS BETWEEN STATES
*/

/*
** Function: goOnHookState (transition)
**
** Description: 
** called when changing to on hook state
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void goOnHookState (chanState *pState){
	uInt8 hkStat;
	#if (PRINTF_IS_OK)
	printf ("On Hook\n");
	#endif
	ProSLIC_ReadHookStatus(pState->ProObj,&hkStat);
	if (hkStat == ONHOOK){
		ProSLIC_ToneGenStop(pState->ProObj);
		ProSLIC_PCMStop(pState->ProObj);
		pState->connectionWith=0xff;
		pState->currentState = OnHook;
	}
	else {
		goOffHookState(pState);
	}
}


/*
** Function: goOffHookState (transition)
**
** Description: 
** called when changing to off hook state
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void goOffHookState (chanState *pState){
	#if (PRINTF_IS_OK)
	printf ("Off Hook\n");
	#endif
	ProSLIC_ToneGenStop(pState->ProObj);
	pState->currentState = OffHook;
}


/*
** Function: powerAlarm (transition)
**
** Description: 
** called when power alarm detected
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void powerAlarm (chanState *pState){
	//implement recovery from alarm
	#if (PRINTF_IS_OK)
	printf ("Power Alarm!\n");
	#endif
	pState->powerAlarmCount++;
	pState->currentState=OnHook;
	if (pState->powerAlarmCount < 6)
		ProSLIC_SetLinefeedStatus(pState->ProObj,LF_FWD_ACTIVE); //Go Forward active
	else {
		ProSLIC_SetLinefeedStatus(pState->ProObj,LF_OPEN);
#if(DEBUG)
			LOGPRINT ("Too many power alarms!\n");
#endif
	}
}


/*
** Function: stopRingingGoOnHk (transition)
**
** Description: 
** called when ringing to stop ringing and change state to on hook
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void stopRingingGoOnHk (chanState *pState){
#if (DEBUG)
		LOGPRINT ("Stop Ring\n");
#endif
	ProSLIC_RingStop(pState->ProObj);
	pState->currentState = OnHook;
}


/*
** Function: goToPulseDialOnHkState (transition)
**
** Description: 
** called when detecting pulse dialing and swithcing to pulse dial on hk state
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void goToPulseDialOnHkState (chanState *pState){
//#if (DEBUG)
	//	LOGPRINT ("\nPulse Dial On Hook\n");
//#endif
	ProSLIC_ToneGenStop(pState->ProObj);
//	SYSTEM_ReadClock(&pState->onHookTime);
	pState->currentState = pulseDialOnHk;
}

/*
** Function: goToPulseDialOffHkState (transition)
**
** Description: 
** called when detecting pulse dialing and swithcing to pulse dial off hk state
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void goToPulseDialOffHkState (chanState *pState){
	//#if (PRINTF_IS_OK)
	//printf ("\nPulse Dial Off Hook\n");
	//#endif
	//SYSTEM_ReadClock(&pState->offHookTime);
	pState->currentState = pulseDialOffHk;
}

/*
* FUNCTIONS BELOW ARE RESULTS OF USER REQUESTS
*/

/*
** Function: callDoBusy (phone number result)
**
** Description: 
** called when busy tone is needed
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoBusy(chanState *pState){
	int busyTone = USA_BUSY_TONE;
	ProSLIC_ToneGenStop(pState->ProObj);
	ProSLIC_ToneGenSetup(pState->ProObj,busyTone);
	ProSLIC_ToneGenStart(pState->ProObj,TRUE);
	pState->currentState = SignalingTone;
}


/*
** Function: callDoRingback (phone number result)
**
** Description: 
** called when ring back tone is needed
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoRingback (chanState *pState){
	int ringbackTone = USA_RINGBACK_TONE;
	ProSLIC_ToneGenSetup(pState->ProObj,ringbackTone);
	ProSLIC_ToneGenStart(pState->ProObj,TRUE);
	pState->currentState = RingbackTone;
}


/*
** Function: callDoRinging (phone number result)
**
** Description: 
** called when ringing is needed
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoRinging (chanState *pState){
	ProSLIC_ToneGenStop(pState->ProObj);
	ProSLIC_RingStart(pState->ProObj);
	pState->ringCount=0;
	pState->currentState=FirstRing;
}


/*
** Function: callDoCongestion (phone number result)
**
** Description: 
** called when congestion tone is needed
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoCongestion (chanState *pState){
	int congestionTone = USA_CONGESTION_TONE;
	ProSLIC_ToneGenSetup(pState->ProObj,congestionTone);
	ProSLIC_ToneGenStart(pState->ProObj,TRUE);
	pState->currentState = SignalingTone;
}

/*
** Function: callDoWideband(Si3216)
**
** Description: 
**
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoWideband (chanState *pState){
	ProSLIC_PCMSetup(pState->ProObj,WIDEBAND_PCM);
	widebandEn[pState->ProObj->channel] = 1;
	printf ("\n*** Set up wideband on other phone before making a call ***\nPlease hang up\n");
}

/*
** Function: callDoWideband(Si3216)
**
** Description: 
** 
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoNarrowband (chanState *pState){
#if defined(PCM_ALAW)||defined(PCM_A2L2A)
	ProSLIC_PCMSetup(pState->ProObj,ALAW_PCM); 
#endif	
#if defined(PCM_ULAW)||defined(PCM_U2L2U)
	ProSLIC_PCMSetup(pState->ProObj,ULAW_PCM); 
#endif	
	widebandEn[pState->ProObj->channel] = 0;
	printf ("\n*** Turn off wideband on other phone before making a call ***\nPlease hang up\n");
}

/*
** Function: callDoCall (phone number result)
**
** Description: 
** called when pbx phone connection is being set up
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoCall (chanState *pState){
	pState->connectionWith = pState->digits[1] - '1';
}

/*
** Function: callDoCallBack (phone number result)
**
** Description: 
** called when phone request a call back
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoCallBack (chanState *pState){
#if (PRINTF_IS_OK)
	printf ("\nCall back programmed\n");
#endif
	pState->currentState = callBack;
}

/*
** Function: callDoreorder (phone number result)
**
** Description: 
** called when reorder tone is needed
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoReorder (chanState *pState){
	int reorderTone = USA_REORDER_TONE;
	ProSLIC_ToneGenSetup(pState->ProObj,reorderTone);
	ProSLIC_ToneGenStart(pState->ProObj,TRUE);
	pState->currentState = SignalingTone;
}

/*
** Function: callDoExit (phone number result)
**
** Description: 
** called when user wants to exit
**
** Input Parameters: 
** pState: pointer to channel state data structure
**
** Return:
** 
*/
static void callDoExit (chanState *pState){
	ProSLIC_Reset(pState->ProObj);
	pState->currentState = exit;
}

/*
** Caller ID checksum
*/
uInt8 checkSum( int8 * string )
{
int i =0;
uInt8 sum=0;
while (string[i] !=0){
        sum += string[i++];
        }

return -sum ;
}

/*
** NOTE: Caller ID is very real time intensive and will probably not work on windows. This fsk code is for informational purposes.
*/
static void DoCallerID (proslicChanType *pProslic){
	uInt8 init=0;
	uInt8 preamble[] =
		{
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U',
			'U','U','U'
		} ;
	int8 message[] =
		"\x80\x27\x01\x08" "01010101"  "\x02\x0A"  "0123456789"  "\x07\x0f"
		"ProSLIC" "\x20" "CALLING" "\x00" ;
	uInt8 data;
	int FskCfg = USA_DEFAULT_FSK;
	ProSLIC_FSKSetup (pProslic, FskCfg);

	(pProslic->deviceId->ctrlInterface)->Delay_fptr((pProslic->deviceId->ctrlInterface)->hTimer,250);//caller id starts 250ms after ring burst
	if (pProslic->debugMode)
		LOGPRINT ("\nSending CID to channel %d\n",pProslic->channel);
	
	ProSLIC_EnableCID(pProslic);

	ProSLIC_SendCID(pProslic,preamble,30);

	(pProslic->deviceId->ctrlInterface)->Delay_fptr((pProslic->deviceId->ctrlInterface)->hTimer,130); //wait for 1 byte then 130ms +/- 25ms mark bits

	data = checkSum(message);
	message[message[1]+2] = data;
	ProSLIC_SendCID(pProslic,message,message[1]+3);
	
	ProSLIC_DisableCID(pProslic);
}


//place holder for exit state
static void exit (chanState *pState,ProslicInt eInput){
}

/*
** $Log: pbx.c,v $
** Revision 1.6  2010-03-22 02:10:11  qwert
** reduce compiler warning
**
** Revision 1.5  2010-02-22 03:53:21  qwert
** move spi interface reset after SLIC reset
**
** Revision 1.4  2009-07-28 10:59:20  qwert
** Change setting for A-law or U-law
**
** Revision 1.3  2009-02-13 07:06:02  qwert
** Add count down for waiting phone onhook.
**
** Revision 1.2  2009-01-20 12:59:59  qwert
** For 8-bit mode(u-law or a-law)
**
** Revision 1.1  2008-12-31 02:34:53  qwert
** Add si3210 driver
**
** Revision 1.34  2008/04/14 22:39:59  lajordan
** redefined LOGPRINT
**
** Revision 1.33  2008/03/28 19:03:22  lajordan
** updates for filename mixed case and configuration
**
** Revision 1.32  2007/05/30 22:20:28  lajordan
** added wideband feature
**
** Revision 1.31  2007/03/22 18:52:56  lajordan
** fixed warnings
**
** Revision 1.30  2007/02/26 16:51:57  lajordan
** cleaned up some warnings
**
** Revision 1.29  2007/02/26 16:46:30  lajordan
** cleaned up some warnings
**
** Revision 1.28  2007/02/16 23:54:25  lajordan
** no message
**
** Revision 1.27  2007/02/15 23:56:05  lajordan
** no message
**
** Revision 1.26  2007/02/15 23:33:08  lajordan
** no message
**
** Revision 1.25  2007/02/01 21:29:58  lajordan
** updated for broadcast init
**
** Revision 1.24  2006/11/29 20:04:02  lajordan
** fixed chipType reference
**
** Revision 1.23  2006/08/09 19:59:36  sasinha
** added printChipDetails
**
** Revision 1.22  2006/08/09 16:14:35  lajordan
** added versionstr
**
** Revision 1.21  2006/08/09 16:08:30  lajordan
** removed getch
**
** Revision 1.20  2006/08/07 21:52:21  sasinha
** relaxed pulse dial thresholds
**
** Revision 1.19  2006/08/02 22:30:36  lajordan
** added si3215m part type
**
** Revision 1.18  2006/08/02 21:41:15  lajordan
** pulse dial thresh relaxed just a little
**
** Revision 1.17  2006/08/02 21:22:21  lajordan
** pulse dial thresh relaxed just a little
**
** Revision 1.14  2006/07/21 04:18:28  lajordan
** changed the way dial tone is set up
**
** Revision 1.13  2006/07/20 19:09:58  lajordan
** pulse dial thresholds changes
**
** Revision 1.12  2006/07/20 16:29:57  lajordan
** fixed pulse dial thresholds
**
** Revision 1.11  2006/07/20 16:22:50  lajordan
** demo does not continue if communication error occurs
**
** Revision 1.10  2006/07/19 19:51:10  lajordan
** no message
**
** Revision 1.9  2006/07/19 19:46:15  lajordan
** added callback functionality
**
** Revision 1.8  2006/07/19 19:37:46  lajordan
** added 16khz tones
**
** Revision 1.7  2006/07/18 23:51:45  lajordan
** caller id edits
**
** Revision 1.7  2006/06/21 22:42:26  laj
** new api style
**
*/