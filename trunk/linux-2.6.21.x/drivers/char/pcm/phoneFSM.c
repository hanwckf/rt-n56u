
#include "dual_io.h"
#include "gr909.h"
#include "dual.h"
#include "slic.h"
#include "isr.h"
#include "pcm_ctrl.h"
#include <linux/time.h>
#include <linux/kernel.h>	

#define atoi(str) simple_strtoul(((str != NULL) ? str : ""), NULL, 0)

extern tone_struct UKcongest ;
extern tone_struct UKring ;
extern Oscillator UKring2 ;
extern Oscillator worldSingleTones [NUMSPECIALTONE] [NUMCOUNTRY] ;
extern tone_struct worldTones [NUMCOUNTRY][NUMTONEtype];

void nextdigit(unsigned char newdigit);
void runDiag(void);
void sendProSLICID (void);

int count=0;
tone_struct *ptr;
Oscillator *optr;
int perLine[2];


char * filename[2] = {"channel1.log","channel2.log"};
	
/* defintions */	
#define printf printk
#define scanf //scanf
#define exit	//exit
void getchar(){}
#define sleep slic_sleep

#define false 0
#define true  1
#define CLOCKS_PER_SEC	4000


    

static char * icause[]={
	"Osc1 Inactive",  
	"Osc1 Active",  
	"Osc2 Inactive",  
	"Osc2 Active",  
	"Ring Inactive", 
	"Ring Active" ,
	"Pulse Metering Inactive",
	"Pulse Metering Active",
	"Ring Trip",
	"Loop Status Change",
	"Ground Key",
	"VOC Tracking",
	"DTMF Decode",
	"Ram Access Interrupt",
	"Transmit Path Modem Tone Detct",
	"Receive Path Modem Tone Detect",
	"							Pwr Alarm Q1",
	"                           Pwr Alarm Q2",
	"                           Pwr Alarm Q3",
	"                           Pwr Alarm Q4",
	"                           Pwr Alarm Q5",
	"                           Pwr Alarm Q6",
	"Common mode balance fault",
};
	
int phone_task(unsigned long pData)
{

	for (currentChannel=0; currentChannel<2; currentChannel++)
	{
		changeCID(currentChannel);
	
		pCCData= &channelData[currentChannel];
	
		phoneSystemDemo();  

		

	}

	tasklet_hi_schedule(&phone_tasklet);
	

}


/**************************** Tone Definitions *********************************************/

/*

These structures are use to hold the values of the group of registers which control the two oscillators
on the ProSLIC.  These oscillators are implemented with an amplitude parameter called the frequency 
parameter called the x parameter and an amplitude parameter called the y parameter.  Each frequency and
amplitude map to an x and y parameter.  This mapping is not linear.  Also included in this structure
is the value of cadence timer periods.  Each field in this structure contains the value of one ProSLIC
register.

typedef struct {
	unsigned short coeff;
	unsigned short x;
	unsigned short y;
	unsigned char on_hi_byte;
	unsigned char on_low_byte;
	unsigned char off_hi_byte;
	unsigned char off_low_byte;
} Oscillator;




typedef struct {
	Oscillator osc1;
	Oscillator osc2;
} tone_struct;  

typedef struct {
unsigned short frequency;
unsigned short coeff; 
unsigned short x; 
unsigned short y; 
} coeffData;

*/

//extern tone_struct worldTones [NUMCOUNTRY][NUMTONEtype];


tone_struct *ptr;
Oscillator *optr;

struct  {

	int next;
	ringStruct ringRegs;
}  ringCadenceTables [] [5] =



//{	20,	"RING_OSC",			0x7EF0	},
//{	21,	"RING_X",			0x0160	},
//{	22,	"RING_Y",			0x0000	},


{


{
	
	{0, {0x7EF0,0x160,0, 1000*8,2000*8}},//1F40=8000 on,3E80=16000 off
	
	
},

{
	{1, {0x7EF0,0x160,0,   200*8,400*8}},//1F40=8000 on,3E80=16000 off
	{0, {0x7EF0,0x160,0,   400*8,1000*8}},//1F40=8000 on,3E80=16000 off
	
},
{
	{1, {0x7EF0,0x160,0,  400*8,600*8}},
	{0, {0x7EF0,0x160,0,  400*8,1600*8}},
	
	
		
},
{
	{1, {0x7EF0,0x160,0,  600*8,600*8}},
	{0, {0x7EF0,0x160,0,  1200*8,600*8}},
	
		
},

{
	
	{1, {0x7EF0,0x160,0,  800*8,400*8}},
	{0, {0x7EF0,0x160,0,  200*8,1600*8}},
		
},
{
	
	{0, {0x7EF0,0x160,0,  200*8,300*8}},
	
		
}




};


char *version= "1.0c Single"; // Version STRING!

char * exceptionStrings[] = 
{ 
	"ProSLIC not communicating", 
	"Time out durring Power Up", 
	"Time out durring Power Down",
	"Power is Leaking; might be a short",
	"Tip or Ring Ground Short",
	"Too Many Q1 Power Alarms" ,
	"Too Many Q2 Power Alarms" ,
	"Too Many Q3 Power Alarms" 
	"Too Many Q4 Power Alarms" ,
	"Too Many Q5 Power Alarms" ,
	"Too Many Q6 Power Alarms" 
};


void exception (enum exceptions e)
/* This is where an embedded system would call its exception handler */
/* This code runs a print out routine */
{
	printf( "\n                 E X C E P T I O N: %s\n",exceptionStrings[e] );
	printf( " Terminating the program\n");
	getchar();
	exit(e);
}



void calibrate(void) {setState(CALIBRATE);}



void key (void){
	printf ("Press any key to continue");  
	getchar();
}




/*******************************************************************
This function combined with the function is ISR.c accomplish the entire
Interrupt Service Routine of the program.  Here the  program finds what caused
the interupt and changes the state of that channel as necessary.
*********************************************************************/
 
void defered_isr(void) 
{
	// Interrupt service  routine
	/*
	15 IRQ1 Interrupt Status 1 PULSTIS PULSTAS RINGTIS RINGTAS OS2TIS OS2TAS OS1TIS OS1TAS Oper R/W 0x00
	16 IRQ2 Interrupt Status 2 RXMDMS TXMDMS RAMIRS DTMFS VOCTRKS LONGS LOOPS RTRIPS Oper R/W 0x00
	17 IRQ3 Interrupt Status 3 CMBALS PQ6S PQ5S PQ4S PQ3S PQ2S PQ1S Oper R/W 0x00
	18 IRQEN1 Interrupt Enable 1 PULSTIE PULSTAE RINGTIE RINGTAE OS2TIE OS2TAE OS1TIE
	*/
	
	unsigned long shiftMask=1, original_vec;
	
	enum              // Declare enum type Days
	{
		OSC1_T1,  
		OSC1_T2,
		OSC2_T1,
		OSC2_T2,
		RING_T1,
		RING_T2,
		PULSE_T1,
		PULSE_T2,
		RING_TRIP,
		LOOP__STAT,
		LONGS,
		VOCTRKS,
		DTMF,
		RAMIRS,
		TXMDMS,
		RXMDMS,
		PQ1,
		PQ2,
		PQ3,
		PQ4,
		PQ5,
		PQ6,
		CMBALS,
	} interruptCause;
	           
	pCCData->eventNumber++;
	original_vec=pCCData->interrupt;


	for ( interruptCause=OSC1_T1 ; interruptCause <= CMBALS ; interruptCause++)
	{
		if (shiftMask & pCCData->interrupt)
		{
			pCCData->interrupt &= ~shiftMask;   // clear interrupt cause

			printf((( interruptCause >=10) && (interruptCause<=11))?"\n %s""\t\t\t\t Channel #%d  \n":"\n(%s)  " "\t\t\t\t Channel #%d  \n", icause[interruptCause], currentChannel+1);
			switch (interruptCause) 
			{
				// Figure out what todo based on which one occured
				case RING_TRIP:
				case LOOP__STAT:
					//checkForGroundShorts(); /* Check for grounded Tip or Ring Leads*/
					setState(LOOPtRANSITION);
					break;
				default:
					printf("interruptCause:Error\n");
					break;

				case RING_T1:
					pCCData->ringCount++;
					if (pCCData->state==FIRSTrING)//run caller ID after first ring.
					{ 
						pCCData->ringCount=1;
						if (pCCData->version >2 && pCCData->country==USA   )
							setState(CALLERiD);
						else
							setState(RINGING);
					}
					if (((worldSingleTones[RING][pCCData->country]).next) != NULL)  
					{
						//if ((count%2) == 0)
						{
							if (optr->next != NULL)
							{
								optr = optr->next;
							}
							else
							{
								optr = &(worldSingleTones[RING][pCCData->country]);
							}
				
						}
						writeReg(RINGTALO, optr->on_low_byte); // low reg 48
						writeReg(RINGTAHI, optr->on_hi_byte); // hi reg 49	
					}
					break;
				case RING_T2:
					if (pCCData->state==PROGRAMMEDrING)
					{
						//nextCadence();
					}
					if (((worldSingleTones[RING][pCCData->country]).next) != NULL)  
					{
						//Inactive Timer
						writeReg( RINGTILO, optr->off_low_byte); // low reg 50
						writeReg( RINGTIHI, optr->off_hi_byte); // hi reg 51
					}
					break;
				case PULSE_T1:
					break;
				case PULSE_T2:
					break;
				case PQ1:
				case PQ2:
				case PQ3:
				case PQ4:
				case PQ5:
				case PQ6:
					{	
						static unsigned long lastEventNumber[8];
						{
							int lcv;	
							for (lcv=0;lcv<8;lcv++)lastEventNumber[lcv] =1;  
						}
						if (lastEventNumber[currentChannel] != pCCData->eventNumber)  /*  We allow only one alarm per alarm event */
						{
							int i = interruptCause - PQ1;
							lastEventNumber[currentChannel] = pCCData->eventNumber;
							printf( "  %d time",pCCData->qLog[i]);
							if (pCCData->qLog[i]++>2) 
								exception(POWERaLARMQ1+i);
							if(pCCData->qLog[i] >1) printf( "s");
							goActive();
							setState(ONHOOK);
						} 
					}	
					break;
				case DTMF:
					setState(DTMFtRANISTION); 
					break;	
				case RAMIRS:
					break;
				case OSC1_T1:
					{
						if (count==20)
							count=2; //reset counter
						if (((worldTones[pCCData->country][pCCData->toneType]).next) != NULL)  
						{
							//if ((count%2) == 0)
							{
								if (ptr->next != NULL)
								{
									ptr = ptr->next;
								}
								else
								{
									ptr = &(worldTones[pCCData->country][pCCData->toneType]);
								}
							}
							writeReg( O1TALO, ptr->osc1.on_low_byte);
							writeReg( O1TAHI, ptr->osc1.on_hi_byte);
							//osc1_ontimer_enable = 0x10;
							writeReg( O2TALO, ptr->osc2.on_low_byte);
							writeReg( O2TALO, ptr->osc2.on_hi_byte);
							//osc2_ontimer_enable = 0x10;
					
							writeRam( OSC1FREQ, ptr->osc1.coeff);
							writeRam( OSC2FREQ, ptr->osc2.coeff);
							writeRam( OSC1AMP, ptr->osc1.x);
							writeRam( OSC2AMP, ptr->osc2.x);
							writeRam( OSC1PHAS, ptr->osc1.y);
							writeRam( OSC2PHAS, ptr->osc2.y);
							count++;
						}
		
					}
					break;
				case OSC1_T2:
					{
	
						if (((worldTones[pCCData->country][pCCData->toneType]).next) != NULL )  
							//if (count%2 ==1)
							{
								//disableOscillators();
								//genTone(ptr);
							
								//Inactive Timer
								writeReg( O1TILO, ptr->osc1.off_low_byte);
								writeReg( O1TIHI, ptr->osc1.off_hi_byte);
								//osc1_offtimer_enable = 0x08;
								// Inactive Timer
								writeReg( O2TILO, ptr->osc2.off_low_byte);
								writeReg( O2TIHI, ptr->osc2.off_hi_byte);
								//osc2_offtimer_enable = 0x08;
	
							}				
					
					
					}
					break;
				case OSC2_T1:
					break;
				case OSC2_T2:
					break;
		
			} //switch

	
		} //if
		shiftMask<<=1;
	}//for

} // function interruptCause






void pulseMeter (void)
{
	optr = &worldSingleTones[PULSEtYPE][pCCData->country];

	enable_PulseMetering();

}



void standardRinging(void) 
{ 	
// Enables ringing mode on ProSlic for standard 
	optr = &worldSingleTones[RINGtYPE][pCCData->country];
	initilaize_ringing();
}


/*****************************************************
This function combined with caller.c illustrates how to send caller ID data.
*************************************************************/



void callerid( void)
{
	unsigned char irqmask[3];

	//writeReg(LINEFEED,2);
	irqmask[0]=readReg(IRQEN1);
	irqmask[1]=readReg(IRQEN2);
	irqmask[2]=readReg(IRQEN3);


	writeReg(IRQEN3, 0);
	readReg(IRQ3);
	writeReg(IRQEN2, 0);
	readReg(IRQ2);
	writeReg(IRQEN1, 1);
	readReg(IRQ1);
   
	//sleep (150);  // 250 millisecond spacing between the ringing and the caller id

	writeReg(OMODE,0); //disable oscillators
	sendProSLICID();



	writeReg(OMODE,0); //disable oscillators
	//readReg(IRQ1);
	   
	writeReg(IRQEN1, irqmask[0]);
	writeReg(IRQEN2, irqmask[1]);
	writeReg(IRQEN3, irqmask[2]);

}







/***********************************************
Ran for each channel, checks interrupt and correct
state
***********************************************/


void phoneSystemDemo(void) 
{
	int demo=1,i=0, channel=0;
	
 //	pCCData->interrupt = 0;
 //	printf("phoneSystemDemo...\n");
	pCCData->eventEnable =1;
	
	/* Basic Touch Tone Phone system demo */
	
 
	if (pCCData->eventEnable) 
	{
		interruptChannelData();
		if (pCCData->interrupt)
			defered_isr();   // isr = interrrupt service routine
	}
	
	stateMachine();

}


void setState(int newState)
{
	pCCData->previousState=pCCData->state;
	pCCData->newState= newState;
	pCCData->state=STATEcHANGE;
	switch (newState)
	{
		 case CALLERiD:
			 pCCData->eventEnable=0;
			 break;
		 case RINGING:
			 pCCData->eventEnable=1;
			 break;
	}
}
/*
double elapsedtime (clock_t timer)
{
	clock_t currenttime;
	currenttime= clock();
//	printf("\n\n currenttime = %0.4f", currenttime/CLOCKS_PER_SEC);
	return (double)(currenttime - timer )/ CLOCKS_PER_SEC;
}
*/
#include <asm/mipsregs.h>
static __u64 clock()
{
	__u64 clk = 0;
	return read_c0_count();//cycle();
	//return clk;
}

int elapsedtime_fixed (__u64 timer)
{
	__u64 currenttime;
	currenttime = clock();
	//return (currenttime - timer )/40000;
	return (currenttime - timer )>>4;
}

/************************************************
This function illustrates how to collet on-hook/off-hook data
in the event of pulse dialing
***********************************************/
void dialPulseDigit(void)
{
#ifdef PCM_SLIC_FLOAT	
	double breaktime;
#else
	int breaktime_fixed;	
#endif	
	if(!(getLoopStatus()))
	{
#ifdef PCM_SLIC_CLOCK	 	
		pCCData->OnHtime=clock();
#endif		
		pCCData->hook_status = 1;

	}
	else
	{
		
#ifdef PCM_SLIC_FLOAT		
		breaktime =elapsedtime(pCCData->OnHtime);
#else
		//breaktime_fixed = elapsedtime_fixed(pCCData->OnHtime);
#endif		
		//printf("\n\n breaktime = %0.4f", breaktime);
		pCCData->hook_status = 0;
#ifdef PCM_SLIC_FLOAT		
		if ( (breaktime > ((0.040))) && (breaktime < ((0.08))))
#else
		//if ( (breaktime_fixed > 400) && (breaktime_fixed < 800))
		if ( (breaktime_fixed > (400*40000>>4)) && (breaktime_fixed < (800*40000>>4)) )
#endif
		{
#ifdef PCM_SLIC_CLOCK			
			pCCData->OffHtime=clock();
#endif			
			pCCData->currentPulsedigit++;
		}
	
	}
}


/****************************************************************
performs necessary routines of each state and changes to next state
******************************************************************/
void stateMachine(void){

	static int cnt = 0;
	int j;
	//if(cnt%100000==0)
	if(pCCData->newState!=pCCData->state)
		printf("st=[%d]%s<=[%d]%s\n",pCCData->newState,szStates[pCCData->newState],pCCData->state,szStates[pCCData->state]);
	cnt++;
	switch (pCCData->state)
	{
		

	/*  Below are only the meta/transient states (transitions).
	The actual precursor states are changed durring the event/interrupt handler.
	I know this is unconventional but it does shrink down the code considerably.  	
	Latter when I fold in the wait states and the delays I will put the stable states back here.	
		*/
	

	case STATEcHANGE:
		 pCCData->state=pCCData->newState;
		  break;			
	case CALLbACK :
		disableOscillators();
		standardRinging();
		activateRinging();
		pCCData->state = FIRSTrING;
		break; 
	case MAKEpULSE:
		disableOscillators();
		pulseMeter ();
		setState (PULSING);
		break;
	case CALLBACKpROGRAMMED:
		disableOscillators();
 		activateRinging();
		pCCData->state = PROGRAMMEDrING;
		break; 
	case MAKEoFFHOOK: // Make dialtone 
		pCCData->toneType = DIALTONEtype;
		ptr = &(worldTones[pCCData->country][pCCData->toneType]);
		if ((worldTones [pCCData->country][pCCData->toneType]).next != NULL)
		{
			//complexCadence = true;
			count = 0;
		}
		//else
			//complexCadence = false;
		genTone(ptr); //********
		printMenu();
		setState(DIALtONE);
		pCCData->digit_count=0;
		break;
	case LOOPtRANSITION://handles loop transitions.
		switch (pCCData->previousState)
		{
				
			case RINGING:
			case FIRSTrING:
			case CALLERiD:
				setState(OFFHOOK);
				break;
			case DEFEREDcALLBACKpROGRAMMED:
				setState (CALLBACKpROGRAMMED);
				break;
			case DIALtONE:
				pCCData->currentPulsedigit=0;
				disableOscillators();
			case DIALpULSE://checks if in on-hook transition was really a pulse digit.
			case DIGITDECODING:
				dialPulseDigit();
				setState(DIALpULSE);
				break;
			case PULSING:
				writeReg (PMCON,0x00); //disable pulse metering
				break;
			default:
				loopAction();
		}
		break;


	case DIALpULSE:
		if(pCCData->hook_status)
		{
#ifdef PCM_SLIC_FLOAT
			if(elapsedtime(pCCData->OnHtime)>.08)
			{
				loopAction();
			}
#else
			if(elapsedtime_fixed(pCCData->OnHtime)>(80*40000>>4))
			{
				loopAction();
			}			
#endif			
		}
		else
		{
#ifdef PCM_SLIC_FLOAT
			if(elapsedtime(pCCData->OffHtime)>.052)
				nextdigit((unsigned char)('0' + pCCData->currentPulsedigit));
#else
			if(elapsedtime_fixed(pCCData->OffHtime)> (52*40000>>4))
				nextdigit((unsigned char)('0' + pCCData->currentPulsedigit));
#endif
		}
		break;

	case DTMFtRANISTION:
		switch (pCCData->previousState){
			case OFFHOOK:
			case BUSY:
			case RINGbACK:
			break;
			case DIALtONE:
				disableOscillators();
				dtmfAction();
				break;
			case DIGITDECODING:
				dtmfAction();
				break;
			default:
				//printf("DTMFtRANISTION:Error[%d]\n",pCCData->previousState);
				break;	
		}
		break;

	case CALLERiD:
		callerid();
		setState(RINGING);
		break;

	
	case FIRSTrING:
	case RINGING:
		{int k = ONHOOK;}
		if (pCCData->connectionWith != currentChannel )
			j=channelData[pCCData->connectionWith].state;
		else
			j=0xFF;
		
		if ((pCCData->ringCount>6)|| (j==ONHOOK))
		{ 
			breakConnection(currentChannel,pCCData->connectionWith );
			stopRinging();
			pCCData->ringCount=0;
			setState(ONHOOK);
		}
		break;

	case ONHOOK:

		/*  The marker for a non-connection is that connectionWidth element is connection to its self */

		if (pCCData->connectionWith != currentChannel )
		{
			breakConnection(currentChannel,pCCData->connectionWith );
			channelData[pCCData->connectionWith].connectionWith=pCCData->connectionWith;
			if(channelData[pCCData->connectionWith].state == OFFHOOK2) channelData[pCCData->connectionWith].state = MAKEcONGESTION;
				pCCData->connectionWith =currentChannel;
		}
		break;
	case MAKEbUSY:
		pCCData->toneType = BUSYtype;
		ptr = &(worldTones[pCCData->country][pCCData->toneType]);
		if ((worldTones [pCCData->country][pCCData->toneType]).next != NULL)
		{
			//complexCadence = true;
			count = 0;
		}
		//else
			//complexCadence = false;
		genTone(ptr);  //********
		setState(BUSY);
		pCCData->digit_count=0;
		break;
	case MAKErINGbACK:
		pCCData->toneType = RINGBACKtype;
		ptr = &(worldTones[pCCData->country][pCCData->toneType]);
		if ((worldTones [pCCData->country][pCCData->toneType]).next != NULL)
		{
			//complexCadence = true;
			//count = 0;
		}
		//else
			//complexCadence = false;
		genTone(ptr); //**********
		setState(RINGbACK);
		pCCData->digit_count=0;
		break;

	case RINGbACK:
	if (pCCData->connectionWith != currentChannel )
		{
		if (channelData[pCCData->connectionWith].state==OFFHOOK)
		
			setState(OFFHOOK);
		}
		break;

	case OFFHOOK:

		disableOscillators();
		
		if (connectionTable[currentChannel].destination== currentChannel )
			makeConnection(currentChannel,pCCData->connectionWith);
		
		setState(OFFHOOK2);
		break;
/*	case MAKEbUSYjAPAN:
		genTone( BusyJapan);
		setState(RINGbACKjAPAN);
		pCCData->digit_count=0;
		break;
    case MAKErINGbACKjAPAN:
		genTone( RingbackJapan);
		setState(RINGbACKjAPAN);
		pCCData->digit_count=0;
		break;*/
	case MAKErEORDER:
		pCCData->toneType = REORDERtype;
		ptr = &(worldTones[pCCData->country][pCCData->toneType]);
		if ((worldTones [pCCData->country][pCCData->toneType]).next != NULL)
		{
			//complexCadence = true;
			count = 0;
		}
		//else
		//	complexCadence = false;
		genTone(ptr); //**********
		setState(REORDER);
		pCCData->digit_count=0;
		break;
	case MAKEcONGESTION:
		pCCData->toneType = CONGESTIONtype;
		ptr = &(worldTones[pCCData->country][pCCData->toneType]);
		if ((worldTones [pCCData->country][pCCData->toneType]).next != NULL)
		{
			//complexCadence = true;
			count = 0;
		}
		//else
		//	complexCadence = false;
		//printf ("%d  ",complexCadence);
		genTone(ptr); //*********
		setState(CONGESTION);
		pCCData->digit_count=0;
		break;
	case DIALtONE:
		break;
	case REORDER:
		break;
	case BUSY:
		break;		 	
	default:
		if((pCCData->state>DIALpULSE)||(pCCData->state<MAKEbUSY))
			printf("stateMachine:Error[%d]\n",pCCData->state);

		break;	
	}

}

/*****************************************************
This function handles loop transition interrupt
*****************************************************/

void loopAction(void)
{
	unsigned char loopstate;

	loopstate =getLoopStatus();
	//printf(loopstate?"Off hook":"On hook");

	if(loopstate)//if off-hook
	{
		if (pCCData->previousState==OFFHOOK || pCCData->previousState==OFFHOOK2 ) // This avoids the loop closure after ringtrip
		{	
			setState(pCCData->previousState);  // backup to previous state		
			return;
		}
		
		if ( pCCData->state != MAKEoFFHOOK)  
		{
				setState(MAKEoFFHOOK);
#ifdef PCM_SLIC_CLOCK				
				//pCCData->Off_Hook_time = clock();
#endif				
				return;		
		}
	}

	if ((pCCData->state != ONHOOK) && (loopstate)==0 )//if on hook
	{		

		setState(ONHOOK);
#ifdef PCM_SLIC_CLOCK
		//pCCData->On_Hook_time = clock();
		//	if ((pCCData->On_Hook_time - pCCData->Off_Hook_time  ) < 2000 )
		//	{
		//		setState(CALLbACK);
		//		return;
		//	}
#endif			
		disableOscillators();
		return;
	}

}



/******************************************************
Handles DTMF interrupt
******************************************************/

void dtmfAction()
{  
	
	unsigned char digit;
	char asciiChar;
	
		
	digit = getDTMFdigit();
	

	switch (digit){
	case 0x0:
			return;	
	case 0xC:
			asciiChar = '#';
			printk("ending\n");
			return;

	default:
		asciiChar= '0' + digit;
		break;
	}

	printk("KEY=%c [%d]\n", asciiChar,digit);
	
}


void nextdigit(unsigned char newdigit)//finds called number whether it be dtmf or pulse digit.
{  
	void (*funct)();
	
	setState(DIGITDECODING);
	if (pCCData->digit_count < 20)
	{
		pCCData->DTMF_digits[pCCData->digit_count] = newdigit; 
		pCCData->digit_count++;
		pCCData->DTMF_digits[pCCData->digit_count]= 0;
		pCCData->currentPulsedigit=0;
	
		printf("Value= 0x%01x  String collected \"%s\" ", newdigit, &pCCData->DTMF_digits );	
		funct =		findNumber();
		if (funct) funct();
	}
}
/************************************************************
The blow function sets the state machine to make various tones
******************************************************************/

void busy() 
{ 
	printf("\n 3210 match");
	setState(MAKEbUSY);
}

void pulse () 
{
	printf ("\n 3215 match");
	setState (MAKEpULSE);
}

void congestion() { printf("\n 5555 match");
				setState(MAKEcONGESTION);
}
void ringBack() { printf("\n 3211 match");
				setState(MAKErINGbACK);}

void ringBackJapan() { printf("\n 4444 match");
				setState(MAKErINGbACKjAPAN);}



void busyJapan() { printf("\n 4445 match");
				setState(MAKEbUSYjAPAN);}

void emergency() { printf("\n 911 match");}
void reOrder() { printf("\n no match");
				setState(MAKErEORDER);}

void quickNeon(void) { printf("\n Neon"); setState( PRENEON); }

void callbackProgrammed(int type) 
{
	printf("\n Ringing Type %i",type);
	setState(DEFEREDcALLBACKpROGRAMMED);
	pCCData->ringCadenceCordinates.nextCadenceEntryIndex=0;
	pCCData->ringCadenceCordinates.ringType = type;
}


void cb0() { callbackProgrammed (0); }
void cb1() { callbackProgrammed (1); }
void cb2() { callbackProgrammed (2); }
void cb3() { callbackProgrammed (3); }
void cb4() { callbackProgrammed (4); }
void cb5() { callbackProgrammed (5); }
void cleanExit(void) { exit (0);}

void callSetState ()   // **************ADDED*************
{
	//calls setState to change dial tone
	printf ("#_ match. Switching to Country Selected.");
	setState(MAKEoFFHOOK);
}
/*
typedef struct  {
	char * phoneNumber ;
	char * functionName;
	void (*action)();
}tNUMBER;
*/


tNUMBER phoneNumbers[] = {
	{ "61", "Dial 1st Phone",callNumber},
	{ "62", "Dial 2nd Phone",callNumber},
	{ "63", "Dial 3rd Phone",callNumber},
	{ "64", "Dial 4th Phone",callNumber},
	{ "65", "Dial 5th Phone",callNumber},
	{ "66", "Dial 6th Phone",callNumber},
	{ "67", "Dial 7th Phone",callNumber},
	{ "68", "Dial 8th Phone",callNumber},
	{"#0", "USA Tones", callSetState},     /*ADDED*********** */
	{"#1","Japanese Tones",callSetState  }, 
	{"#2", "Taiwan   ",callSetState }, /*ADDED ********* */
	{"#3","Austria",callSetState  }, 
	{"#4","Belgium    ",callSetState  }, 
	{"#5","Bulgaria",callSetState  }, 
	{"#6","Czech Republic",callSetState  }, 
	{"#7","Denmark  ",callSetState  }, 
	{"#8","Finland  ",callSetState  }, 
	{"#9","France   ",callSetState  }, 
	{"*0","Hungary  ",callSetState  }, 
	{"*1","Iceland  ",callSetState  }, 
	{"*2","Italy     ",callSetState  }, 
	{"*3","Luxembourg",callSetState  }, 
	{"*4","Netherlands",callSetState  }, 
	{"*5","Norway     ",callSetState  }, 
	{"*6","Poland     ",callSetState  }, 
	{"*7","Portugal     ",callSetState  }, 
	{"*8","Slovakia   ",callSetState  }, 
	{"*9","Spain     ",callSetState  }, 
	{"40","Sweden   ",callSetState  }, 
	{"41","Switzerland",callSetState  }, 
	{"42","UK         ",callSetState  },
	{ "3210","Busy Tone", busy},
	{ "***", "Quit Program", cleanExit},
	{ "3211", "Ring Back Tone",ringBack},
	{ "333", "Neon          ",quickNeon},
	{ "50#", "Type 0 Ringing",cb0},
	{ "51#", "Type 1 Ringing",cb1},
	{ "52#", "Type 2 Ringing",cb2},
	{ "53#", "Type 3 Ringing",cb3},
	{ "54#", "Type 4 Ringing",cb4},
	{ "55#", "Type 5 Ringing",cb5},
	{ "555", "Congestion",congestion},
	{"911", "Busy Tone",busy},
	{"3215","Pulse Metering",pulse},
	{"", " Any Other Number=>Reorder Tone", reOrder},
};
	


void printMenux(void)
{
	int	i=0;
	printf("\n\n\t \t \t P H O N E  D I R E C T O R Y\n ");
	while (*phoneNumbers[i].phoneNumber)
	{
		//if ( (int) (i/3.0) == (i/3.0) )
		if ( (int) (i/3) == (i/3) )
			printf ("\n");
		else
			printf ("\t");
		printf("%s\t%s",phoneNumbers[i].phoneNumber,phoneNumbers[i].functionName);
		i++;
	}
	printf("\n\n\t \t \t P H O N E  D I R E C T O R Y\n ");
}
	
void printMenu(void)
{
	
}

void chooseCountry (char *cOUNTRY) /* *************ADDED *****************/
{
	//	int i;
	pCCData->country = atoi (cOUNTRY+1);
	if (cOUNTRY[0] == '*')
		pCCData->country = pCCData->country + 10;
	if (cOUNTRY[0] == '4')
		pCCData->country = pCCData->country + 20;
	//printf ("%d",country);
	//scanf ("%d",&i);
	if (pCCData->country >= NUMCOUNTRY)
	{
		printf ("Error - Invalid country\n");
		pCCData->country = USA;
	}
	//country = i;
}

/*********************************************************************
Finds number in phone directory.
**********************************************************************/

void  (*findNumber(void))(void)
{
	static  int currentNumber;
	int 
	tableR, // Row
	tableC; // Column

	/*
		To find the number a while loop is start at the last place the program left off 
		in the phone number table and continues until either there is a match with fewer digits 
		than a table entry, or there is a match with an entire entry, or there is no match  and the 
		end of the table is reached.
	*/

	tableR=currentNumber;
	tableC=pCCData->digit_count-1;
	
	while (phoneNumbers[tableR].phoneNumber[tableC] !=0)
	{
		if (phoneNumbers[tableR].phoneNumber[tableC] == pCCData->DTMF_digits[tableC])
		{
			tableC++;
		
			if (tableC == pCCData->digit_count)
			{
				if (phoneNumbers[tableR].phoneNumber[tableC]== 0)
				{
					currentNumber=0;
					tableC= 0;
					pCCData->digit_count=0;
					if (*(phoneNumbers[tableR].phoneNumber) == '#' || *(phoneNumbers[tableR].phoneNumber) == '*'  || *(phoneNumbers[tableR].phoneNumber) == '4')				// ADDED ***********
						chooseCountry (phoneNumbers[tableR].phoneNumber);	//ADDED ********
		
					return  phoneNumbers[tableR].action;
		
				}
				else 
					return NULL ;
			}
		
		} 
		else
		{
			tableR=++currentNumber;
			tableC= 0;
		}
	}

	currentNumber=0;
	pCCData->digit_count=0;
	
	return &reOrder ;
}

void nextCadence(void) 
{
	int x = pCCData->ringCadenceCordinates.nextCadenceEntryIndex;
	int y =pCCData->ringCadenceCordinates.ringType;
	
	/* Start current coordinate in the table */
	cadenceRingPhone(ringCadenceTables[y][x].ringRegs);
	
	/* change coordinate */
	pCCData->ringCadenceCordinates.nextCadenceEntryIndex= ringCadenceTables[y][x].next;
}

void stopRinging(void)
{
	goActive();
}


void callNumber()
{
	int  calledChannelNumber;

	calledChannelNumber= pCCData->DTMF_digits[1]-'1';

	/* check called channel state */
	if (channelData[calledChannelNumber].state == ONHOOK)
	{
	
		pCCData->connectionWith = calledChannelNumber;
		channelData[calledChannelNumber].connectionWith = currentChannel;
		channelData[calledChannelNumber].state = CALLbACK;
		/*
		fprintf(logfile[0],"\nState = %s"
			"\t\t\t Channel  %i",szStates[pCCData->state],currentChannel);
			fflush(logfile[0]);
		*/
	  	ringBack();
	}
	else
		busy();
}



