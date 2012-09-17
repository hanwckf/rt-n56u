/*******************************************************************************
						Caller.c Copyright 2002 Silicon Laboratories

This file demonstrates how to send simple caller ID/FSK information.
************************************************************************************/


#include "dual_io.h"
#include "dual.h"


// Notice that the 0x24 is the length of the caller ID frame.
// You would need to change this if you wanted to change the length of the
// message


static unsigned char init ;// this will indicate if there is a missed interrupt

void fskInitialization (void)
{
	init =0 ;  // gloabal variable used to detect critical timing violation
	           // if init =2 => more than 1/1200th second passed between interrupt
	writeReg(PCMMODE,0);
	writeReg(O1TALO,19);  // 19 is twenty ticks  20/24000 sec = 1/1200 sec
	writeReg(O1TAHI,0); // 0 is zero MSB of timer
	writeReg(OMODE,0xA); // Start/Stop bit+ FSK enabled + Receive direction
	writeReg(OCON,0x5); // Osc1 Active Osc 1 + Active Timer enabled

	/*
	tones_params.exe" -f 1200 .2 2200
	Copyright 2002 Silicon Laboratories
	
	
	INFO: Settings for  2200 as 0 frequency are - FSK_COEFF0 = 0x35b0, FSK_X0 = 0x0222
	INFO: Settings for  1200 as 1 frequency are - FSK_COEFF1 = 0x3ce0, FSX_X1 = 0x0123
	INFO: Settings for 0 to 1 transition are - FSK_COEFF0 = 0x3ce0, FSK_X01H = 0x1118, FSK_X01L = 0x1d88
	INFO: Settings for 1 to 0 transition are - FSK_COEFF1 = 0x35b0, FSK_X10H = 0x3be0, FSK_X10L = 0x1330
	*/

}



char SiLabsID2[] =
 "\x80\x27\x01\x08" "01010101"  "\x02\x0A"  "0123456789"  "\x07\x0f"
 "ProSLIC" "\x20" "CALLING" ;//"\x2F" ;




void waitForInterrupt (void)
{
	/* Wait for an Interrupt from the ProSLIC => oscillator loaded */
	if (Interrupt() && init !=0){
	
	//printf(" %1.1x",init) ;
	init = 2;
	
	}
	if (init == 0) init=1; /* init has 3 states 0 => fsk initialized
	                 1 => fsk did first interrupt
					 2 => got premature interrupt
	*/

	while (!Interrupt());
		readReg(IRQ1);
}

void fskByte(unsigned char c)
{
	waitForInterrupt() ;   
	writeReg(FSKDAT,c);

}// fskByte()




unsigned char checkSum( char * string )
{
int i =0;
unsigned char sum=0;
while (string[i] !=0){
        sum += string[i++];
        }

return -sum ;
}


void sendProSLICID (void)
{   
	
	static char c ='0',modulo=0;
	int i; 
unsigned char sum;
//	time_t curtime_a;
//	char  sztime[10];
//	struct tm *loctime;
	/* Get the current time.  */
//	curtime_a = time (NULL);
	/* Convert it to local time representation.  */
//	loctime = localtime (&curtime_a);
//	sprintf(sztime,"%02.2i""%02.2i""%02.2i""%02.2i",loctime->tm_mon+1,loctime->tm_mday,loctime->tm_hour,loctime->tm_min);
//	memcpy((SiLabsID2+4),&sztime,8);
//	memset (&SiLabsID2[14], c+(modulo++%10),10);;

writeReg(LINEFEED,2);	
sum= checkSum(SiLabsID2);
//printf("sum=%x",sum);
fskInitialization ();
//sleep (200);
for ( i=0 ; i<15 ; i++ ) waitForInterrupt();
for ( i=0 ; i<30; i++) fskByte('U');
for ( i=0 ; i<15 ; i++ ) waitForInterrupt();

i=0;
//while  (SiLabsID2[i] != 0) fskByte(SiLabsID2[i++]);
while  (SiLabsID2[i]) fskByte(SiLabsID2[i++]);

fskByte(sum);
for ( i=0 ; i<15 ; i++ ) waitForInterrupt();
}










