/*
** Copyright © 2007 by Silicon Laboratories
**
** $Id: Gr909Demo.c,v 1.1 2008-12-31 02:34:53 qwert Exp $
**
** SI321x_Gr909Demo.c
** SI321x ProSLIC GR909 demonstration file. Example implementation
** of GR909 on Windows PC platform using parallel port SPI.
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
*/

#include "stdio.h"
#include "proslic.h"
#include "Si321x_gr909.h"
#include "time.h"
#include "math.h"
#include "proslic_api/example_drivers/win/proslic_ppt_mb/spi.h"
#include "proslic_api/example_drivers/win/timer/timer.h"

uInt8 RshInstalled[8]={0,0,0,0, 0,0,0,0}; //eight storage units.  One unit for each possible SLIC
uInt8 Calibrated[8] ={0,0,0,0, 0,0,0,0};
si321x_rencal_t renCalDataLV = {33,91,232,57,343,156,-9};
si321x_rencal_t renCalDataHV = {27,83,249,81,343,147,16};
si321x_rencal_t *renCalData = &renCalDataLV;

#define pProHW		pProslic->deviceId->ctrlInterface->hCtrl
#define Reset		pProslic->deviceId->ctrlInterface->Reset_fptr
#define Delay		pProslic->deviceId->ctrlInterface->Delay_fptr
#define pProTimer	pProslic->deviceId->ctrlInterface->hTimer
#define WriteReg	pProslic->deviceId->ctrlInterface->WriteRegister_fptr
#define ReadReg		pProslic->deviceId->ctrlInterface->ReadRegister_fptr

#define PRINTF_IS_OK TRUE

#define NUMBER_OF_CHAN 1

#define VERSIONSTR "4.6"

_int64 readTSC ();
_int64 time0,time1,ticksPerSecond;

typedef struct chanStatus chanState; //forward declaration

typedef void (*procState) (chanState *pState, ProslicInt eInput);



static void SetRenCalValues();
static void RenCalibration(ProSLICGR909Type *pProSLIC909);
static void SaveRenCal();

struct chanStatus { 
	proslicChanType *ProObj;
/*	procState currentState;
	uInt16 digitCount;
	uInt8 digits[20];
	uInt8 ringCount;
	uInt16 connectionWith;
	uInt16 powerAlarmCount;
	pulseDialType pulseDialData;
	BOOLEAN eventEnable;
*/
} ;

/*not part of Gr909 but provided as a reference*/
int32 ProSLIC_PSTN(ProSLICGR909Type *pProSLIC909);

/*foreign voltage test*/
static void ForeignVoltageDemo(ProSLICGR909Type *pProSLIC909){
//simulate polling environment
_int64 time1;
_int64 timeout; int done=0;
SI321x_VOLTAGES_STATE vState;
vState.State.stage=0; vState.samples=100;

	while (!done) {
		time1 = readTSC();   
		timeout = (__int64)(POLL_RATE*(ticksPerSecond/1000))+time1; // Set timeout 
		

		done = ProSLIC_GR909_FOREIGN_VOLTAGES(pProSLIC909,&vState);

		while (readTSC() < timeout);
	}
}
/*resistive fault test*/
static void ResistiveFaultDemo(ProSLICGR909Type *pProSLIC909){
//simulate polling environment
_int64 time1;
_int64 timeout; int done=0;
SI321x_RMEAS_STATE rMeas;
SI321x_RMEASCOARSE_STATE rMeasCoarse;
rMeasCoarse.DiffState.stage=0;
rMeasCoarse.LongState.stage=0;

rMeas.State.stage=0;

	while (!done) {
		time1 = readTSC();   
		timeout = (__int64)(POLL_RATE*(ticksPerSecond/1000))+time1; // Set timeout 
		if(RshInstalled[pProSLIC909->pProslic->channel]){   // External Resistor between TIP-RING installed
			done = ProSLIC_GR909_RESISTIVE_FAULT(pProSLIC909,&rMeas);
		}
		else {
			done = ProSLIC_GR909_RESISTIVE_FAULT_NOSH(pProSLIC909,&rMeasCoarse);
		}

		while (readTSC() < timeout);
	}
}
/*receiver offhook test*/
static void RohDemo (ProSLICGR909Type *pProSLIC909){
	SI321x_ROH_STATE rohState;

	int done=0;

	_int64 timeout; 
    _int64 time1;


	rohState.State.stage=0;

	while (!done) {
		time1 = readTSC();   
		timeout = (__int64)(POLL_RATE*(ticksPerSecond/1000))+time1; // Set timeout 
		

		done = ProSLIC_GR909_RECEIVER_OFFHOOK(pProSLIC909,&rohState);

		while (readTSC() < timeout);
	}
}
/*measure ren test*/
static void RenDemo (ProSLICGR909Type *pProSLIC909,int cal){
	//simulate polling environment
	int done=0;

	_int64 timeout; 
    _int64 time1;

    SI321x_REN_STATE renState;
	renState.State.stage=0;

	renState.RenCalFlag=cal;
	renState.renCal = renCalData;
	//printf("Press a key to abort\n");
	while (!done) {
		time1 = readTSC();   
		timeout = (__int64)(POLL_RATE*(ticksPerSecond/1000))+time1; // Set timeout 
		

		done = ProSLIC_GR909_REN(pProSLIC909,&renState);
		
		while (readTSC() < timeout);
	}
}

// ----------------------------------
// syssleep()
//
// Idle system delay
//
// ----------------------------------
void syssleep( unsigned long wait )
{
   int goal;
   goal = wait + clock();
   while( goal > clock() )
      ;
}
// ----------------------------------
// readTSC()
//
// Read host total cycle count (TSC)
// to be utilized for system timing
//
// ----------------------------------
_int64 readTSC (void)
{
	union {
 _int64 extralong;
 unsigned long longish[2];
	} t;
 unsigned long a,b;
 
 _asm {
 _emit 0x0f;
 _emit 0x31;
 mov a,eax;
 mov b,edx;
 }
 t.longish[0]=a;t.longish[1]=b;
 return t.extralong;
}







// ----------------------------------
// calibratePrecisionClock()
//
// Determine number of host cycles 
// in 1 second to calibrate elapsed
// cycles to elapsed time.
//
// ----------------------------------
_int64 calibratePrecisionClock(void)
{
syssleep(1);
time0= readTSC();
syssleep (1800);
time1 = readTSC();

ticksPerSecond=((time1-time0)/1800000)*1000000;
return ticksPerSecond;
}




static void GR909ClearResults(ProSLICGR909Type *pProSLIC909)
{

	//VOLTAGES
	pProSLIC909->tgAC = 0;
	pProSLIC909->trAC = 0;
	pProSLIC909->rgAC = 0;
	
	pProSLIC909->tgDC = 0;
	pProSLIC909->trDC = 0;
	pProSLIC909->rgDC = 0;

	//RESISTIVE FAULTS
	pProSLIC909->tg = 10000000;
	pProSLIC909->tr = 10000000;
	pProSLIC909->rg = 10000000;

	pProSLIC909->rohTrue=0;

	//REN
	pProSLIC909->renValue = 0;
}

static void GR909ClearSettings(ProSLICGR909Type *pProSLIC909)
{
	pProSLIC909->CalFlag = 0;
}
static void goOpen(proslicChanType *pProslic)
{
	uInt8 data;
	data = 0;
	WriteReg(pProHW, pProslic->channel, 72,  data);  // Voc=0
	WriteReg(pProHW, pProslic->channel, 73, data);  // Vcm=0
	
	data = 1;
	WriteReg(pProHW, pProslic->channel, 64, data);
	Delay(pProTimer, 200);
	
	data = 0;
    WriteReg(pProHW, pProslic->channel, 64, data);
	
	data = 0x2;
    WriteReg(pProHW, pProslic->channel,73, data);
	
	data = 0x20;
    WriteReg(pProHW, pProslic->channel, 72,data);
	Delay(pProTimer, 100);
}

static void channelSelect(ProSLICGR909Type *pProSLIC909)
{
	uInt8 channelSelect;
	do
	{
		printf("\nPlease enter channel: (0-->%d): ",(NUMBER_OF_CHAN-1));
		channelSelect=getch();
		putch(32);
		putch(channelSelect);	
		printf("\n");
		channelSelect = ((uInt8)channelSelect - 48); //convert from ASCII to integer
	}
	while(channelSelect<0 || channelSelect>(NUMBER_OF_CHAN-1));
	pProSLIC909->pProslic->channel = channelSelect; 


}


static void printChipDetails(proslicChanType *pProslic)
{
	char* name = "";

	uInt8 data;
	char* freqs[ ] = {"8192","4028","2048","1024","512","256","1536","768","32768"};

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
	data = ReadReg(pProHW, pProslic->channel, 13);  /* Read the frequency */
	data = data>>4;
	//-----------------END OF FREQUENCY-----------------------

	printf("%s Rev %c (%s KHz)\n",name, pProslic->deviceId->chipRev, freqs[data]);
}

static void gr909SetupMenu(ProSLICGR909Type *pProSLIC909)
{
uInt8 m=0;
SI321x_GR909_Config config;
config.rshValue = 68;

   while(toupper(m)!='Q')
   {
		printf("\n\n\n");
		printf("--------------------------------------------------------------------------\n");
		printf("----------------------- CALIBRATION AND SETUP ----------------------------\n");
		printf("--------------------------------------------------------------------------\n");
		printf("1.  Perform REN Test Calibration \n");
		printf("2.  Manually Set REN Calibration Values \n");
		printf("3.  Save REN Calibration Values  \n");
		if(RshInstalled[pProSLIC909->pProslic->channel])
			printf("4.  Shunt Resistor (Rsh):	Installed\n");
		else
			printf("4.  Shunt Resistor (Rsh):	Not Installed\n");
		printf("5.  Change BOM option \n");
		printf("--------------------------------------------------------------------------\n");
		printf("\nUse PC Keyboard to enter a number key or 'q' to quit: ");

		m=getch();
		putch(32);
		putch(m);

        switch (m)
        {
           case '1':
              RenCalibration(pProSLIC909); 
           break;
           case '2':
              SetRenCalValues();
           break;
           case '3':
              SaveRenCal();
           break;           
           case '4':
			   RshInstalled[pProSLIC909->pProslic->channel]=(RshInstalled[pProSLIC909->pProslic->channel])?0:1;
           break;
		   case '5':
			   config.rshValue = 0;
			   printf ("\nEnter shunt resistor value in k ohms (680k default):\n\n");
			   m = getch();
			   putch(m);
			   config.rshValue = (m-'0')*10;
			   m = getch();
			   putch(m);
			   config.rshValue += (m-'0');
			   m = getch();
			   putch(m);
			   printf ("\nChoose:\n0: Standard BOM \n1: High Voltage BOM\n");
			   m = getch();
			   putch(32);
			   putch(m);
			   config.bomOption = (m == '1')? 1 : 0;
			   ProSLIC_GR909_INIT(pProSLIC909,&config);
			   renCalData = &renCalDataLV;
			   if (config.bomOption)
				   renCalData = &renCalDataHV;
           break;
		}
	}
}

static void SetRenCalValues()
{
char m=0;
char val[32]={0};

	while(toupper(m)!='Q')
	{
		printf("\n\n");
		printf("Enter REN Calibration Parameter To Be Manually Changed\n");
		printf("or q when finished\n\n");
		printf("1.  lowRenSlope    (currently set to %d)\n", renCalData->lowRenSlope);
		printf("2.  lowRenOffs     (currently set to %d)\n", renCalData->lowRenOffs);
		printf("3.  highRenSlope   (currently set to %d)\n", renCalData->highRenSlope);
		printf("4.  highRenOffs    (currently set to %d)\n", renCalData->highRenOffs);
		printf("5.  renTrans       (currently set to %d)\n\n", renCalData->renTrans);
		printf("6.  extraLowOffset       (currently set to %d)\n\n", renCalData->extraLowRenOffset);
		printf("7.  extraLowSlope       (currently set to %d)\n\n", renCalData->extraLowRenSlope);
		printf("-> ");

		m=getch();
        printf("\n\n\n");

		switch(m)
		{
		   case '1':
			  printf("Enter New Value For lowRenSlope -> ");
			  gets(val);
			  renCalData->lowRenSlope = atoi(val);
		   break;
		   case '2':
			  printf("Enter New Value For lowRenOffs -> ");
			  gets(val);
			  renCalData->lowRenOffs = atoi(val);
		   break;
		   case '3':
			  printf("Enter New Value For highRenSlope -> ");
			  gets(val);
			  renCalData->highRenSlope = atoi(val);
		   break;
		   case '4':
			  printf("Enter New Value For highRenOffs -> ");
			  gets(val);
			 renCalData->highRenOffs = atoi(val);
		   break;
		   case '5':
			  printf("Enter New Value For renTrans -> ");
			  gets(val);
			  renCalData->renTrans = atoi(val);
		   break;

		}
	}
}
static void SaveRenCal()
{
FILE *fp,*fopen();

   if((fp=fopen("rencal.txt","w")) == NULL)
   {
	   printf("\nERROR - Cannot Open rencal.txt for writing!!\n");
   }
   else
   {

	
	{
	   printf("\nSaving REN Calibration values to rencal.txt...");
	   fprintf(fp,"lowRenSlope\n");
	   fprintf(fp,"%d\n", renCalData->lowRenSlope);
	   fprintf(fp,"lowRenOffs\n");
	   fprintf(fp,"%d\n", renCalData->lowRenOffs);
	   fprintf(fp,"highRenSlope\n");
	   fprintf(fp,"%d\n", renCalData->highRenSlope);
	   fprintf(fp,"highRenOffs\n");
	   fprintf(fp,"%d\n", renCalData->highRenOffs);
	   fprintf(fp,"renTrans\n");
	   fprintf(fp,"%d\n", renCalData->renTrans);
	   fprintf(fp,"extraLowRenOff\n");
	   fprintf(fp,"%d\n", renCalData->extraLowRenOffset);
	   fprintf(fp,"extraLowRenSlope\n");
	   fprintf(fp,"%d\n", renCalData->extraLowRenSlope);
	   fclose(fp);
	   printf("done!\n\n");
	   fflush(stdout);
	}
   }
}
static void RenCalibration(ProSLICGR909Type *pProSLIC909)
{


int32 oneRenQ2Pwr;
int32 threeRenQ2Pwr;
int32 fiveRenQ2Pwr;
int32 zeroRenQ2Pwr;
uInt8 m;

        printf("\n\n"); fflush(stdout);
		printf("Step 1:  Connect Known 0 REN Load and Hit Any Key ->  ");fflush(stdout);
    	m=getch();
		putch(32);
		putch(m);
		printf("\nCalibrating...\n");fflush(stdout);
		pProSLIC909->CalFlag = 1;
		RenDemo (pProSLIC909,1);
		zeroRenQ2Pwr = pProSLIC909->renValue;
		//pProSLIC909->Delay(pProSLIC909->pProTimer,500);
		printf("Step 1:  Connect Known 1 REN Load and Hit Any Key ->  ");fflush(stdout);
    	m=getch();
		putch(32);
		putch(m);
		printf("\nCalibrating...\n");fflush(stdout);
		pProSLIC909->CalFlag = 1;
		RenDemo (pProSLIC909,1);
		oneRenQ2Pwr = pProSLIC909->renValue;
		//pProSLIC909->Delay(pProSLIC909->pProTimer,500);
		printf("\nStep 2:  Connect Known 3 REN Load and Hit Any Key ->  ");fflush(stdout);
        m=getch();
		putch(32);
		putch(m);
		printf("\nCalibrating...\n");fflush(stdout);
		//pProSLIC909->Delay(pProSLIC909->pProTimer,500);
		pProSLIC909->CalFlag = 1;
		RenDemo (pProSLIC909,1);
		threeRenQ2Pwr = pProSLIC909->renValue;
		//pProSLIC909->Delay(pProSLIC909->pProTimer,500);
		printf("\nStep 3:  Connect Known 5 REN Load and Hit Any Key ->  ");fflush(stdout);
        m=getch();
		putch(32);
		putch(m);
		printf("\nCalibrating...\n");fflush(stdout);
		pProSLIC909->Delay(pProSLIC909->pProTimer,500);
		pProSLIC909->CalFlag = 1;
		RenDemo (pProSLIC909,1);
		fiveRenQ2Pwr = pProSLIC909->renValue;
		renCalData->extraLowRenSlope = (oneRenQ2Pwr - zeroRenQ2Pwr) ;
		renCalData->highRenSlope = (fiveRenQ2Pwr-threeRenQ2Pwr)/2;
		renCalData->lowRenSlope = (threeRenQ2Pwr-oneRenQ2Pwr)/2;
		renCalData->renTrans = threeRenQ2Pwr*104;    // Transition just above 3 REN
		renCalData->renTrans = renCalData->renTrans/100;
		renCalData->highRenOffs = fiveRenQ2Pwr - (5*renCalData->highRenSlope);
		renCalData->lowRenOffs = threeRenQ2Pwr - (3*renCalData->lowRenSlope);
		renCalData->extraLowRenOffset = oneRenQ2Pwr - (renCalData->extraLowRenSlope);
		//renSlope = (fiveRenQ2Pwr-oneRenQ2Pwr)/4;
		//renOffs = fiveRenQ2Pwr - (5*renSlope);

		Calibrated[pProSLIC909->pProslic->channel] = 1;

}


static void GR909menu(proslicChanType *pProslic)
{ 
uInt8 m=0, saveResults = 0;
int32 temp;
ProSLICGR909Type pSLIC909;
	
	pSLIC909.pProslic = pProslic;

	GR909ClearSettings(&pSLIC909);

	if(NUMBER_OF_CHAN != 1)
        channelSelect(&pSLIC909); //choose which SLIC to test
	

    while (toupper(m)!='Q')
	{
        if(!saveResults)
			GR909ClearResults(&pSLIC909);
		
		goOpen(pSLIC909.pProslic);
		
		printf("\n\n");
		printf("--------------------------------------------------------------------------\n");
		printf("------------------------------  Rev ");    
		printf(VERSIONSTR);
		printf(" ----------------------------------\n");
		printf("----------------------------  GR-909 TESTS -------------------------------\n");
		printf("----------------------------   CHANNEL: %d  -------------------------------\n",pSLIC909.pProslic->channel);
		printf("--------------------------------------------------------------------------\n");
		printf("1.  GR-909 Tests 1&2 [fixed point]:        Hazardous and Foreign EMF Test \n");
		printf("3.  GR-909 Test 3 [fixed point]:           Resistive Fault Test\n");
		printf("4.  GR-909 Test 4 [fixed point]:           Receiver Offhook Test \n");
		printf("5.  GR-909 Test 5 [fixed point]:           REN Test\n");
		printf("6.  GR-909 Test 6 [fixed point]:           PSTN Line Check Demo\n");
		printf("\n");
		printf("7.  Run Entire GR-909 Test Suite (no output)\n");
		printf("8.  Print Last Results\n");
		printf("9.  GR-909 Test Setup Menu\n");
		printf("\n");
		printf("0.  Change Channel\n\n");
		printf("--------------------------------------------------------------------------\n");
		printf("CHANNEL %d STATUS:\n\n",pSLIC909.pProslic->channel);
		if(RshInstalled[pSLIC909.pProslic->channel])
			printf("Shunt Resistor:		Installed\n");
		else
			printf("Shunt Resistor:		NOT Installed\n");
		if(Calibrated[pSLIC909.pProslic->channel])
			printf("ProSLIC (REN):		Calibrated\n");
		else
			printf("ProSLIC (REN):		NOT Calibrated\n");
		printf("--------------------------------------------------------------------------\n");
		printf("\nUse PC Keyboard to enter a number key or 'q' to quit: ");
		
		m=getch();
		putch(32);
		putch(m);
		pSLIC909.Delay(pSLIC909.pProTimer,100);
		/*while (1){
			pSLIC909.pProslic->debugMode=1;
			ResistiveFaultDemo(&pSLIC909);
					
					
					temp = pSLIC909.tr;
					printf("tr = %8d ohm\n",temp);
					temp = pSLIC909.rg;
					printf("rg = %8d ohm\n",temp);
					temp = pSLIC909.tg;
					printf("tg = %8d ohm\n",temp);
					if (pSLIC909.rg < 150000 || pSLIC909.tr < 150000 || pSLIC909.tg < 150000){
						printf("stop\n");					
						getch();
					}
		}*/
		switch (m)
		{
			case '1':
			case '2':
					printf("\nEntering: Voltages Test....\n\n");
					ForeignVoltageDemo(&pSLIC909);
					
					
					temp = (int32)sqrt(pSLIC909.tgAC);
					printf("tgAC = %d.%0.1d\n",temp/10, abs(temp - temp/10*10)); //result is multiplied by 10
					temp = (int32)sqrt(pSLIC909.trAC);
					printf("trAC = %d.%0.1d\n",temp/10, abs(temp - temp/10*10));
					temp = (int32)sqrt(pSLIC909.rgAC);
					printf("rgAC = %d.%0.1d\n",temp/10, abs(temp - temp/10*10));
					printf("\n");
					temp = pSLIC909.tgDC;
					printf("tgDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000)); //result is multiplied by 1000
					temp = pSLIC909.trDC;
					printf("trDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
					temp = pSLIC909.rgDC;
					printf("rgDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
			break;
			case '3':
					printf("\nEntering: Resistive Fault Test....\n\n"); //result in the form of xxx.x k Ohm
					ResistiveFaultDemo(&pSLIC909);
					
					
					temp = pSLIC909.tr;
					printf("tr = %8d ohm\n",temp);
					temp = pSLIC909.rg;
					printf("rg = %8d ohm\n",temp);
					temp = pSLIC909.tg;
					printf("tg = %8d ohm\n",temp);
			break;
			case '4':
					printf("\nEntering: Receiver Offhook Test....\n\n"); //result is multiplied by 100 (except default 100MOhm)
					RohDemo(&pSLIC909);
					switch (pSLIC909.rohTrue){
						case SI321x_FAIL_ROH_DETECTED:
							printf ("Phone Offhook\n");
							break;
						case SI321x_FAIL_RESFAULT_INC:
							printf("Resistive fault detected\n");
							break;
						case 0:
							printf ("Phone onhook\n");
							break;
					}
					
			break;
			case '5':
					printf("\nEntering: REN Test....\n\n"); //result is multiplied by 1000
					pSLIC909.CalFlag=0;
					RenDemo(&pSLIC909,0);
					temp = pSLIC909.renValue;
					printf("renValue = %d.%0.3d \n",temp/1000, abs(temp - temp/1000*1000));
			break;	
			case '6':
					printf("\nEntering: PSTN Test....\n\n"); //result is multiplied by 1000
					printf("\nConnect test source and press any key\n");
					getch();
					temp = ProSLIC_PSTN(&pSLIC909);
					if(temp)
						printf("\nPSTN line check PASSED");
					else
						printf("\nPSTN live line detected!");

				break;
			case '7':
					saveResults = 1;
					printf("\n\nEntering: GR-909 Test Suite....This may take a while\n\n"); //result is multiplied by 1000
					printf("TEST 1&2:");
					ForeignVoltageDemo(&pSLIC909);
					printf(".....COMPLETE\n");
					printf("TEST 3");
					ResistiveFaultDemo(&pSLIC909);
					printf("........COMPLETE\n");
					printf("TEST 4");
					RohDemo(&pSLIC909);
					printf("........COMPLETE\n");
					printf("Remove any external voltage sources on TIP and RING.  Press any key to continue: ");
					getch();
					printf("\nTEST 5");
					RenDemo(&pSLIC909,0);
					printf("........COMPLETE\n");

			break;
			case '8':
					printf("\n\nVOLTAGE TEST RESULTS: \n");
					printf("-------------------------------\n");
					temp = pSLIC909.tgAC;
					printf("tgAC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
					temp = pSLIC909.trAC;
					printf("trAC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
					temp = pSLIC909.rgAC;
					printf("rgAC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));

					temp = pSLIC909.tgDC;
					printf("tgDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
					temp = pSLIC909.trDC;
					printf("trDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));
					temp = pSLIC909.rgDC;
					printf("rgDC = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));

					printf("\n\n\nRESISTIVE FAULTS TEST RESULTS: \n");
					printf("-------------------------------\n");
					temp = pSLIC909.tg;
					printf("tg = %8d.%d kOhm\n",temp/10, abs(temp - temp/10*10));
					temp = pSLIC909.tr;
					printf("tr = %8d.%d kOhm\n",temp/10, abs(temp - temp/10*10));
					temp = pSLIC909.rg;
					printf("rg = %8d.%d kOhm\n",temp/10, abs(temp - temp/10*10));

					printf("\n\n\nRECEIVER OFFHOOK TEST RESULTS: \n");
					printf("-------------------------------\n");
					
					switch (pSLIC909.rohTrue){
						case SI321x_FAIL_ROH_DETECTED:
							printf ("Phone Offhook\n");
							break;
						case SI321x_FAIL_RESFAULT_INC:
							printf("Resistive fault detected\n");
							break;
						case 0:
							printf ("Phone onhook\n");
							break;
					}

					printf("\n\n\nRREN TEST RESULTS: \n");
					printf("-------------------------------\n");
					temp = pSLIC909.renValue;
					printf("renValue = %d.%0.3d\n",temp/1000, abs(temp - temp/1000*1000));


			break;
			case '9':
					gr909SetupMenu(&pSLIC909);
			break;
			case '0':
				if(NUMBER_OF_CHAN!= 1)
					channelSelect(&pSLIC909);
				else
					printf("\n**** Cannot change channel. Only 1 channel detected ***\n\n");
			break;
		}	
	}
	ProSLIC_Reset(pSLIC909.pProslic); //reset at exit
}
#define NUMBER_OF_PROSLIC NUMBER_OF_CHAN
uInt8 GR909Demo()
{
	ctrl_S spiGciObj; //spi interface object
	systemTimer_S timerObj;    //timer object
	controlInterfaceType *ProHWIntf; //hw interface
	ProslicDeviceType *ProSLICDevices[NUMBER_OF_PROSLIC]; //proslic device object
	proslicChanType_ptr arrayOfProslicChans[NUMBER_OF_CHAN]; //used for initialization only
	chanState ports[NUMBER_OF_CHAN];  //declare channel state structures
	uInt8 i=0;
	BOOLEAN keepRunning = TRUE;
	proslicChanType *pSlic;


	//initialize SPI interface   
	if (SPI_Init (&spiGciObj) ==  FALSE){
		#if (PRINTF_IS_OK)
		printf ("Cannot connect\n");
		#endif
		return 0;	
	}
#if (PRINTF_IS_OK)
	printf("Hang up phone!\n");
#endif
	//initialize timer
	TimerInit(&timerObj); 
	calibratePrecisionClock();
	ProSLIC_createControlInterface(&ProHWIntf);
	for (i=0;i<NUMBER_OF_PROSLIC;i++)
		ProSLIC_createDevice (&(ProSLICDevices[i]));
	for (i=0;i<NUMBER_OF_CHAN;i++){
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
	printf ("-------Si321x Gr909 ------\n\n");
	#endif
	
	ProSLIC_Reset((ports[0].ProObj));	//Reset the ProSLIC(s) before we begin

	//Initialize the channel state for each channel
	for (i=0;i<NUMBER_OF_CHAN;i++){
		arrayOfProslicChans[i] = (ports[i].ProObj); //create array of channel pointers (for broadcast init)
		ProSLIC_setSWDebugMode (ports[i].ProObj, 0);
	}
	ProSLIC_Init(arrayOfProslicChans,NUMBER_OF_CHAN);
	//ProSLIC_InitBroadcast(ports[0].ProObj);
	for (i=0;i<NUMBER_OF_CHAN;i++){
		pSlic = (ports[i].ProObj);
		ProSLIC_SetLinefeedStatus(pSlic, LF_OPEN);
		#if (PRINTF_IS_OK)
			printf("CHANNEL %d: ",i); 
			printChipDetails(pSlic);
		#endif
    }
	

	GR909menu(pSlic);
	return 1;
}

void main()
{
	GR909Demo(); //tests do not work properly when different board types are stacked eg si3210 with si3215??
}

/*CVS LOG FOLLOWS:*/
//$Log: Gr909Demo.c,v $
//Revision 1.1  2008-12-31 02:34:53  qwert
//Add si3210 driver
//
//Revision 1.31  2008/04/16 16:39:42  lajordan
//no message
//
//Revision 1.30  2007/06/05 15:40:00  lajordan
//no message
//
//Revision 1.29  2007/06/05 15:31:07  lajordan
//no message
//
//Revision 1.28  2007/06/04 18:33:49  lajordan
//no message
//
//Revision 1.27  2007/05/29 19:49:58  lajordan
//changed to current sense ren test
//
//Revision 1.26  2007/05/22 22:46:47  lajordan
//no message
//
//Revision 1.25  2007/05/22 21:57:06  lajordan
//updated for HV BOM
//
//Revision 1.24  2007/04/03 20:46:37  lajordan
//no message
//
//Revision 1.23  2007/04/03 20:26:19  lajordan
//improved ren test
//
//Revision 1.22  2007/04/02 19:49:31  lajordan
//no message
//
//Revision 1.21  2007/02/16 00:34:54  lajordan
//no message
//
//Revision 1.20  2007/02/01 21:34:28  lajordan
//updated for parallel initialization
//
//Revision 1.19  2007/02/01 17:47:06  lajordan
//cosmetic
//
//Revision 1.18  2007/02/01 04:08:38  lajordan
//updated to polling
//
//Revision 1.17  2006/11/29 20:07:53  lajordan
//updated versionstr
//
//Revision 1.16  2006/11/29 20:06:31  lajordan
//updated versionstr
//
//Revision 1.15  2006/11/29 19:59:48  lajordan
//removed unused code
//
//Revision 1.13  2006/08/09 19:45:45  sasinha
//modified printChipDetails
//
//Revision 1.12  2006/08/09 18:30:47  sasinha
//removed sqrt_approx from AC test
//
//Revision 1.11  2006/08/09 18:22:14  sasinha
//added PSTN
//
//Revision 1.10  2006/08/02 18:48:27  lajordan
//moved ui out of gr909.c
//
//Revision 1.9  2006/07/28 23:41:06  lajordan
//added log fields
//