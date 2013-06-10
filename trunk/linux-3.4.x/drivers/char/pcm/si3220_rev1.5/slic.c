/*************************************************************************************
					SLIC.c Copyright 2002 Silicon Laboratories

  This is the primary driver that performs initlization and operation.  Silicon Laboratories
  recommends that an application refers to this when writing code.
**************************************************************************************/

#include "../pcm_ctrl.h"
#include "dual_io.h"
#include "dual.h"
//#include "countries.h"
#include "slic.h"

unsigned char externalring=0;
#define Intring 1
#define Extring 2

extern Oscillator worldSingleTones [NUMSPECIALTONE] [NUMCOUNTRY] ;

#define initIRQEN1 0xFF
#define initIRQEN2 0x3F // Turn off Modem Detect
#define initIRQEN3 0xFF



//private functions.  
void RevBcode(void);
void RevEcode(void);
unsigned char initializeSRAM(void);
void initalizedirect(void);
int waitForTheChannel_limit(void);

extern void slic_sleep( unsigned long wait );
extern void spi_si3220_master_init(int spi_ch);

void unlock_lock_protectedRegister(void);

int si3220_init(void)
{
	int result;
	
	int irq_status[3];
	int data;
	extern unsigned char currentChannel;
	currentChannel = 0;
	while(currentChannel < ppcm_config->pcm_ch_num)
	{
		
		changeCID(currentChannel);
		pCCData= &channelData[currentChannel];
	
		result = dualProSLIC_channelInit();
		if(result)
		{
			printk("***** dualProSLIC_channelInit failed *****\n");
			return -1;;
		}
		initilaize_ringing();	
		
		currentChannel++;
		if(currentChannel==ppcm_config->pcm_ch_num)
			break;
	}
			
		
	data = readReg(DIGCON);
#ifdef PCM_SLIC_LOOP		
	data |= 0x040;
#else
	data &= 0x1F;	
#endif		
	writeReg(DIGCON, data);	

 	irq_status[0] = irq_status[1] = irq_status[2] = 0;
	irq_status[0] = readReg(IRQ1);
	irq_status[1] = readReg(IRQ2);
	irq_status[2] = readReg(IRQ3);
	if(irq_status[0]||irq_status[1]||irq_status[2])
	{
		MSG("IRQ1=%X, IRQ2=%X, IRQ3=%X\n",irq_status[0],irq_status[1],irq_status[2]);
		return 0;
	}

	return 0;
}
/*************************************************************
Returns the Chips ID register.  Gives the product being used and 
what silicon revision.
**************************************************************/

unsigned char getChipID(void)
{
	return readReg(ID);
}

/*******************************************************************
This is the silicon Laboratories initlization procedure.  This function
needs to be ran individually for each channel.  There are sections of code
for paticular silicon revisions, if the latest silicon is being used, these 
sections of code can be omitted.  This code follows the initlization procedure  
in AN58.  Plese consult AN58 and AN71 for software related questions.  In order to
understand the code below, some familarity with the Si3220/25 is required.
********************************************************************/

int dualProSLIC_channelInit()
{
	int i;
	unsigned char ChipID;
	
	changeCID(currentChannel);
	
	
	spi_si3220_master_init(CONFIG_RALINK_PCM_SPICH);
	
	/* wait for PLL and FSYNC signal ready */
	//waitForTheChannel();
	if(waitForTheChannel_limit()!=0)
		return -1;
	//ensure SLIC is in open state
	goOpen();
	
	ChipID = getChipID();
	MSG("Si3220 ChipID=0x%08X\n",ChipID);
	//decide internal or external ringing.
	if((ChipID&0x70)==0)
	{
		externalring=Intring;
		MSG("SLIC is Intring\n");
	}
	else if((ChipID &0x70)==0x20)
	{
		externalring=Extring;
		MSG("SLIC is Extring\n");
	}
	else
	{ 
		MSG("Chip ID %X wrong.\n",ChipID);
		return -1;
	}
	//initialize registers
	if(initializeSRAM())
	{
		MSG("SLIC ERR : initializeSRAM Error\n");
		return -1;
	}
	initalizedirect();
	
	//code for REV B silicon only
	if ((ChipID & 0x0f) == 2)
	{
		RevBcode();		
	}
		
	//code for REV E silicon only
	if ((ChipID & 0x0f) == 5)
	{
		RevEcode();	
	}
	
	//initial timeslot assignments
	writeReg(RXSTLO, (unsigned char) (currentChannel*16+1));
	writeReg(RXSTHI, (unsigned char) 0);
	writeReg(TXSTLO, (unsigned char) (currentChannel*16+1));
	writeReg(TXSTHI, (unsigned char) 0);
	 
#if defined(PCM_LINEAR)||defined(PCM_L2U2L)||defined(PCM_L2A2L)
	writeReg(PCMMODE,0x13);
#endif    
#if defined(PCM_ULAW)||defined(PCM_U2L2U)
	writeReg(PCMMODE,0x11);
#endif
#if defined(PCM_ALAW)
	writeReg(PCMMODE,0x14);
#endif
#if defined(PCM_A2L2A)
	writeReg(PCMMODE,0x1C);
#endif

	//Initial Calibration
	writeReg(CALR2,0x38);
	writeReg(CALR1,0xBF);
	while(readReg(CALR1));//wait for calibration
	
	MSG("calibration done\n");
	
	slic_sleep(100); 
	
	goOpen();
	
	//write protected register for correct Power settings
	unlock_lock_protectedRegister();
	writeReg(THERM,0x05);
	unlock_lock_protectedRegister();
	
	slic_sleep(100);

	//checking phone loop state.
	goActive();
	slic_sleep(10);
#ifdef PCM_SLIC_LOOP
#else	
	while(getLoopStatus());//ensure phone is on-hook
	MSG("phone is on-hook\n");
#endif	
	//this section is a critical period.  The phone cannot be taken off hook during this section.
	
	writeRam(VOC, 0);//let phone discharge
	writeRam(VCM,0);
	//wait for phone discharge.
#ifdef PCM_SLIC_LOOP
#else	
	while ((readRam(VTIP)!=0) || (readRam(VRING)!=0));
#endif
	goOpen();//go back to open for final calibration
	
	writeRam(VOC,initVOC);
	writeRam(VCM,initVCM);

	writeReg(IRQEN2,0x20);  //this interrupt if there was a state change before common mode balance calibration.
	writeReg(IRQEN3,0x80);//this interrupt tells if there was an error during common mode balance calibration

	//Longitudial Calibration
	writeReg(CALR2,0x01);
	writeReg(CALR1,0x80);
	while(readReg(CALR1));//wait for calibration
	MSG("Longitudial calibration done\n");
	//this ends the critical section defined above.
	for(i=0;i<10000;i++);
	if(1)
	{	
	 	int irq_status[3];
	 	irq_status[0] = irq_status[1] = irq_status[2] = 0;
		irq_status[1] = readReg(IRQ2);
		irq_status[2] = readReg(IRQ3);
		if(irq_status[1]||irq_status[2])
		{
			MSG("IRQ2=%X, IRQ3=%X\n",irq_status[1],irq_status[2]);
		}
		
	}

	slic_sleep(50);
   
	//intialize the Relay Driver on the Si3225.
  	if(externalring ==Extring)
	{
		writeReg(RLYCON,0xA8);
	}
	
	// Initialize interrupts 
	writeReg(IRQEN1,initIRQEN1);
	writeReg(IRQEN2,initIRQEN2); 
	writeReg(IRQEN3,initIRQEN3);
	
	goActive();
	
	return 0;
	
}



//special code for revision B silicon
void RevBcode(void)
{
	do
	{
	writeReg(TESTCON,0x2);
	writeReg(TESTCON,0x8);
	writeReg(TESTCON,0xE);
	writeReg(TESTCON,0x0);
	}while(readReg(TESTCON) != 2);
}
		
//special code for revision E silicon
void RevEcode(void)
{
	unlock_lock_protectedRegister();
	writeReg(AUTOCON, 0xBF);
	unlock_lock_protectedRegister();
}

/************************************************************************
Certain register are proteted from accidental writes due to writing to them 
accidently could have adverse effect.  Below is the sequence of writes that 
must be performed in order for on of these registers to be written too.  
Silicon Labs highly recommends locking the register immideately after writing the
protected register.
************************************************************************/

void unlock_lock_protectedRegister()
{
	writeReg(TESTCON,0x2);
	writeReg(TESTCON,0x10);
	writeReg(TESTCON,0x12);
	writeReg(TESTCON,0x0);
}

/********************************************************************
Below are functions used for initlization.
*********************************************************************/

//SRAM initialization 
unsigned char initializeSRAM(void)
{	
	unsigned char i;
	unsigned short ramTemp;
	
	for (i=0;i<LASTSRAM;i++)
	{
		if (initSramValues[i] != 0)
		{
			writeRam (i,initSramValues[i]);
		}
	}

	for (i=0;i<LASTSRAM;i++)
	{
		if (initSramValues[i] != 0)
		{
			ramTemp= readRam(i);
			if ( ramTemp != initSramValues[i])
			{
				MSG("SLIC RAM Write[%d] %X vs. %X\n",i, ramTemp, initSramValues[i]);
				return 1;			
			}
				
		
		}
	}

	if(externalring ==Extring)
	{
		i=0;
		while(initextSRAMValues[i][0])
		{
			writeRam ((unsigned char)initextSRAMValues[i][0],initextSRAMValues[i][1]);
			i++;
		}
	}
	
	return 0;
}

//Direct registers initlization
void initalizedirect(void)
{
	unsigned char i=0;

	while(direct_init_table[i][0])//write direct values 
	{
		writeReg (direct_init_table[i][0], direct_init_table[i][1]);
		i++;
	}
}


//wait for PLL to lock
void waitForTheChannel(void)
{ 
	int data;
	//data = readReg(MSTRSTAT);
	//while ( (readReg(MSTRSTAT)&0xf) != 0x0F);
	while(1)
	{
	data = readReg(MSTRSTAT);	
	if((data&0xf)!=0x0F)
		printk("SLIC is not supported\n");
	else
		break;
	}
	return ;
}

int waitForTheChannel_limit(void)
{ 
	int data, count=0;
	while(count<100)
	{
	data = readReg(MSTRSTAT);	
	if((data&0xf)!=0x0F){
		printk("SLIC is not supported\n");
		count++;
	}	
	else
		return 0;
	}
	return -1;
}
/**********************************************************************
Below are functions used for setting the linefeed states for operation.
***********************************************************************/

//Set LINEFEED to ACTIVE
void goActive(void)

{
#ifdef PCM_SLIC_LOOP
#else
	writeReg(LINEFEED,FORWARD_ACTIVE);	/* LOOP STATE REGISTER SET TO ACTIVE */
							/* Active works for on-hook and off-hook see spec. */
							/* The phone hook-switch sets the off-hook and on-hook substate*/
	slic_sleep(100);
#endif
}

//set LINEFEED to OPEN
void goOpen(void)

{
#ifdef PCM_SLIC_LOOP
#else
	writeReg(LINEFEED,OPEN);	/* LOOP STATE REGISTER SET TO ACTIVE */
							/* Active works for on-hook and off-hook see spec. */
							/* The phone hook-switch sets the off-hook and on-hook substate*/
	slic_sleep(100);
#endif
}


//set Line Feed to Ringing
void activateRinging(void){
	

	writeReg( LINEFEED, RING);
	
}

/*******************************************************************************
Below are functions used for operation
********************************************************************************/

unsigned char checkForGroundShorts (void)
{

	if ((readRam(VTIP) < 2) || readRam(VRING ) < 2) 
		return 1;
	else return 0;
	//	exception(TIPoRrINGgROUNDsHORT); /* Check for grounded Tip or Ring Leads*/

}

//disable oscillators and tones

void disableOscillators() { 
	
	writeReg(OCON,0);	
	writeReg(OMODE,0);  // disable both oscillators

}


//enables connection of two channels

void makeConnection( unsigned char c1, unsigned char c2)
{
	unsigned char timeslot1,timeslot2,  oldCid=Cid;
	timeslot1=c1*8;
	timeslot2=c2*8;
	
	
	changeCID(c1);
	
	writeReg(TXSTLO, timeslot2);
	connectionTable[c1].source=c2;
	
	connectionTable[c2].source=c1;
	
	disableOscillators();
	writeReg(PCMMODE, 0x15);
	
	changeCID(c2);
	
	writeReg(TXSTLO, timeslot1);
	connectionTable[c1].destination=timeslot2;
	
	connectionTable[c2].destination=timeslot1;
	
	disableOscillators();
	writeReg(PCMMODE, 0x15);
	
	Cid=oldCid;	

}

//disables connection of two channels

void breakConnection( unsigned char c1, unsigned char c2)
{

unsigned char	oldCid=Cid;


unsigned char timeslot1,timeslot2;

	timeslot1=c1*8;
	timeslot2=c2*8;
	

	changeCID(c2);
		writeReg(PCMMODE, 0);
		writeReg(TXSTLO, 0);
		writeReg(1, 0);
			
	changeCID(c1);
		writeReg(PCMMODE, 0);
		writeReg(TXSTLO, 0);
		connectionTable[c1].source=c2;


		connectionTable[c1].destination=c1;
		connectionTable[c2].destination=c2;
		connectionTable[c1].source=c1;
		connectionTable[c2].source=c2;
		writeReg(1, 0);
	
	Cid=oldCid;
}

//return offhook or onhook

unsigned char getLoopStatus(void)
{
#ifdef PCM_SLIC_LOOP
	return 1;
#else	
	return readReg(LCRRTP)&1;
#endif
}

//return DTMF digit

unsigned char getDTMFdigit(void)
{
	unsigned long data = 0;
	data = readReg(DTMF);
	if(data&0x30)
		return data & 0x0f;
	else
		return 0;	
	
	//return readReg(DTMF) & 0x0f;
}

//cadence ringing setup

void cadenceRingPhone(ringStruct ringRegs) 
{ 
	writeReg( RINGTALO,  ringRegs.onLowByte); // lo reg 48
	writeReg( RINGTAHI,   ringRegs.onHiByte); // hi reg 49
	// Inactive Timer
	writeReg( RINGTILO, ringRegs.offLowByte); // low reg 50
	writeReg( RINGTIHI, ringRegs.offHiByte); // hi reg 51
	// Enable timers for ringing oscillator
	writeReg( RINGCON, 0x18);
}

//generates tones necessary for operation, i.e dial tone, reorder tone, busy tone, etc.
void genTone(tone_struct *tone) 
{ 
	
	unsigned char  oConTemp = 0; // this is the place holder for the enable bits for Oscillator control
	unsigned char  oModeTemp = 0 ;//readReg(OMODE); // preserve old mode
	//loopBackOff();
	//if (worldTones[country][toneType].next != NULL)
	disableOscillators(); // Make sure the oscillators are not already on.

	if ((*tone).osc1.coeff == 0 || tone->osc1.x == 0) 
	{
		// Error!
		MSG("an invalid struct passed!\n");
		return;
	}
	// Setup osc 1
	writeRam( OSC1FREQ	, tone->osc1.coeff);
	writeRam( OSC1AMP, tone->osc1.x);
	writeRam( OSC1PHAS, tone->osc1.y);
	// Active Timer
    oConTemp |= 0x10;
	oModeTemp |=0x20;

	if ((*tone).osc1.on_hi_byte != 0) {
		writeReg( O1TALO, tone->osc1.on_low_byte);
		writeReg( O1TAHI, tone->osc1.on_hi_byte);
		oConTemp |= 0x40;
	}
	// Inactive Timer
	if ((*tone).osc1.off_hi_byte != 0) {
		writeReg( O1TILO, tone->osc1.off_low_byte);
		writeReg( O1TIHI, tone->osc1.off_hi_byte);
		oConTemp |= 0x20;
	}
	
	if ((*tone).osc2.coeff != 0 ) {
		// Setup OSC 2
		writeRam( OSC2FREQ, tone->osc2.coeff);
		writeRam( OSC2AMP, tone->osc2.x);
		writeRam( OSC2PHAS, tone->osc2.y);
		oConTemp |= 0x1;
		oModeTemp |= 0x2;
		// Active Timer
		if ((*tone).osc1.on_hi_byte != 0) {
			writeReg( O2TALO, tone->osc2.on_low_byte);
			writeReg( O2TAHI, tone->osc2.on_hi_byte);
			oConTemp |= 0x4;
		}
		// Inactive Timer
		if ((*tone).osc1.off_hi_byte != 0) {
			writeReg( O2TILO, tone->osc2.off_low_byte);
			writeReg( O2TIHI, tone->osc2.off_hi_byte);
			oConTemp |= 0x02;
			
		}
		
	}
	writeReg(OCON,oConTemp);
	writeReg( OMODE,oModeTemp); // Both oscillators will be sent to the phone
	
		
	return;
}


//initlizies rining
void initilaize_ringing(void)
{	
	// Active Timer
	writeReg(RINGTALO, worldSingleTones [RINGtYPE][pCCData->country].on_low_byte); // low reg 48
	writeReg(RINGTAHI, worldSingleTones [RINGtYPE][pCCData->country].on_hi_byte); // hi reg 49
	// Inactive Timer
	writeReg(RINGTILO, worldSingleTones [RINGtYPE][pCCData->country].off_low_byte); // low reg 50
	writeReg(RINGTIHI, worldSingleTones [RINGtYPE][pCCData->country].off_hi_byte); // hi reg 51
	// Enable timers for ringing oscillator
	writeReg( RINGCON, 0x18);
}


//enables Pulse metering.
void enable_PulseMetering(void)
{
	writeRam(PMFREQ, worldSingleTones [PULSEtYPE][pCCData->country].coeff);
	writeRam(RINGPHAS, worldSingleTones [PULSEtYPE][pCCData->country].x);
	writeRam(RINGAMP, worldSingleTones [PULSEtYPE][pCCData->country].y);
	//active timer
	writeReg( PMTALO, worldSingleTones [PULSEtYPE][pCCData->country].on_low_byte);
	writeReg( PMTAHI, worldSingleTones [PULSEtYPE][pCCData->country].on_hi_byte); 
	// Inactive Timer
	writeReg( PMTILO, worldSingleTones [PULSEtYPE][pCCData->country].off_low_byte); 
	writeReg( PMTIHI, worldSingleTones [PULSEtYPE][pCCData->country].off_hi_byte); 
	writeReg (PMCON,0x1C);//enable timers & osc
}
