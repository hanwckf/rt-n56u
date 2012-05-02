/*******************************************************************************
						GR909.c Copyright 2002 Silicon Laboratories

This file illustrates how to perfom gr909 line testing and other diagnostic capabalities.

************************************************************************************/


char * gr909Version = "0.5";


#include "gr909.h"
//#include "stdio.h"
#include "dual_io.h"
//#include "time.h"
//#include "math.h"
#include "dual.h"

#define printf printk
#define scanf	//scanf
static void getchar(){}
#define sleep slic_sleep

short currents[5];
unsigned short ren,	acVloop,   dcVloop, acVring,   dcVring, acVtip,   dcVtip;




__s64 readTSC(void);




void runDiag(void)
{
	char select[3];
	unsigned char channel [2];
	while(1)
	{
	printf("\nPress 'A' to run DC/AC voltage test,");
	printf("\nPress 'B' to measure loop  capicatiance using slope time.");
	printf("\nPress 'C' to measure loop  capicatiance using small level 20Hz signal.");
	printf("\nPress 'D' to measure loop  capicatiance using small level 40Hz signal.");
	printf("\nPress 'E' to measure REN,");
	printf("\nPress 'F' to measure Tip to ground resistance.");
	printf("\nPress 'G' to measure Ring to ground resistance.");
	printf("\nPress 'H' to measure loop resistance with tipopen.");
	printf("\nPress 'I' to measure loop resistance with ringopen.");
	printf("\nPress 'J' to measure loop resistance with V/I method.\n");
	
	scanf("%s", select);
	printf("\nEnter Channel number.\n");
	scanf("%s", channel);
	changeCID((unsigned char)(channel[0]-'0'));
	switch(select[0])
	{
	case 'A':
	case 'a':
		openVoltages (   &acVloop,   &dcVloop, &acVring,   &dcVring, &acVtip,   &dcVtip);
		break;
	case 'B':
	case 'b':
		lineCap();
		break;
	case 'C':
	case 'c':
		capUsingRinging();
		break;
	case 'D':
	case 'd':
		capUsingRinging40Hz();
		break;
	case 'E':
	case 'e':
		RENtest(&ren);
		break;
	case 'F':
	case 'f':
		RTG ( ) ;
		break;
	case 'G':
	case 'g':
		RRG ( ) ;
		break;
	case 'H':
	case 'h':
		RTRtipopen ( ) ;
		break;
	case 'I':
	case 'i':
		RTRringopen ( );
		break;
	case 'J':
	case 'j':
		RTRVImeth();
		break;
	
	}
	printf ("\n\n\n\nPress enter to continue\n\n\n");  
	getchar();
	getchar();
	}
}







/***********************************************************************************************************
The open voltage test measures dc and ac current by taking a series of samples and averging them.  The 
dc voltage is found first and is subtracted from the samples to find the AC current.  When implementing 
this function it may be a good idea to try different number of samples in case of many different frequencies 
present
************************************************************************************************************/ 


void openVoltages 
( 
	unsigned short *acVloop, unsigned short *dcVloop,				   
	unsigned short *acVring, unsigned short *dcVring,
	unsigned short *acVtip, unsigned short *dcVtip				   
) // in milivolts
{
	unsigned short i,vtip,vloop,vring;
	unsigned long sumVtip =0;
	unsigned long sumVring =0;
	unsigned long sumVloop =0;
#ifdef PCM_SLIC_FLOAT	
	double samplesVtip[SAMPLES],samplesVring[SAMPLES], samplesVloop[SAMPLES];


	writeReg(LINEFEED,0); 
	//The open state is used to prevent the Si3200 line driver from affecting tip/ring.
	sleep (100); // waiting 100 ms for capicatance settling.
	
	for (i=0;i<SAMPLES/*50 Nominally*/;i++)//aquires and sums samples of line voltage.  
	{
		vtip = 0x7fff & readRam(VTIP);
		samplesVtip[i] = vtip;
		sumVtip += vtip;
	
	
		vring =0x7fff & readRam(VRING);
		samplesVring[i] = vring;
		sumVring += vring;
	
		vloop = 0x7fff & readRam(VLOOP);
		samplesVloop[i] = vloop;
		sumVloop += vloop;
		
	}

	// The samples are then scaled and averaged for the dc value.  If different number of samples are taken, it is possible
	//to not be get different values due to AC frequencies on the line.  

	*dcVtip = (short)((VSCALE * (sumVtip))/SAMPLES);
	*dcVring =(short) ((VSCALE *(sumVring))/SAMPLES);
	*dcVloop = (short) ((VSCALE * (sumVloop))/SAMPLES);

	//output dc values
	printf("\n DC Tip voltage = %d", *dcVtip);
	printf("\n DC Ring voltage = %d", *dcVring);
	printf("\n DC Loop voltage = %d",  *dcVloop);

	sumVtip=0;
	sumVring=0;
	sumVloop=0;

	//The dc offset is then subtracted from the samples to remove the dc component.  Then by squareing the numbers, you can 
	//determine the AC rms value.  See AN71 for details
	for (i=0;i<SAMPLES/*50 Nominally*/;i++)
	{	
		samplesVtip[i] = (samplesVtip[i]*VSCALE) - (*dcVtip);
		samplesVtip[i] = (samplesVtip[i]*samplesVtip[i])/SAMPLES;
		sumVtip += (short) samplesVtip[i];

		samplesVring[i] = (samplesVring[i]*VSCALE) - (*dcVring);
		samplesVring[i] = (samplesVring[i]*samplesVring[i])/SAMPLES;
		sumVring += (short) samplesVring[i];

		samplesVloop[i] = (samplesVloop[i]*VSCALE) - (*dcVloop);
		samplesVloop[i] = (samplesVloop[i]*samplesVloop[i])/SAMPLES;
		sumVloop += (short) samplesVloop[i];
	}
	
	//calculate and output AC rms values
	*acVtip = (short)(sqrt(sumVtip));
	*acVring =(short)(sqrt(sumVring));
	*acVloop = (short)(sqrt(sumVloop));
	
	printf("\n AC Tip voltage = %d", *acVtip);
	printf("\n AC Ring voltage = %d", *acVring);
	printf("\n AC Loop voltage = %d",  *acVloop);
	printf("\n");
#endif	
}

/*********************************************************************************************************** 
This version of open volatge is a way to get the dc/ac vlotage without having to store the samples. It is very
similar, however it does not give a true rms value.
*************************************************************************************************************/

void openVoltages2 ( 
	unsigned short *acVloop, unsigned short *dcVloop,				   
	unsigned short *acVring, unsigned short *dcVring,
	unsigned short *acVtip, unsigned short *dcVtip				   
 ) // in milivolts
{
	unsigned short i,vtip,vring,vloop;
	unsigned long sumSquareVtip=0,sumVtip =0;
	unsigned long sumSquareVring=0,sumVring =0;
	unsigned long sumSquareVloop=0,sumVloop =0;


		writeReg(LINEFEED,0); // GO TO OPEN SO WE CAN READ THE OUTSIDE VOLTAGE
		sleep (100); // wait 100 ms for settling time
	
		//This version sums both the samples and the square of the samples.
		for (i=0;i<SAMPLES/*50 Nominally*/;i++)
		{
		vtip = 0x7fff & readRam(VTIP);
		sumSquareVtip += vtip * vtip; // used 
		sumVtip += vtip;


		vring = 0x7fff & readRam(VRING);
		sumSquareVring += vring * vring;
		sumVring += vring;

		vloop = 0x7fff & readRam(VLOOP);
		sumSquareVloop += vloop * vloop;
		sumVloop += vloop;
		
	}
#ifdef PCM_SLIC_FLOAT	
	//averages and scales DC/AC voltages
	*dcVtip = (short)(VSCALE * (sumVtip/SAMPLES));
	
	*acVtip = (short)( VSCALE * sqrt(sumSquareVtip/SAMPLES))-   *dcVtip;

	*dcVring =(short)(VSCALE * (sumVring/SAMPLES));
	
	*acVring =(short)(VSCALE * sqrt (sumSquareVring/SAMPLES))-  *dcVring;

	*dcVloop = (short)(VSCALE * (sumVloop/SAMPLES));
		
	*acVloop = (short)(VSCALE * sqrt (sumSquareVloop/SAMPLES))-  *dcVloop;
#endif
	//Output voltage
	printf("\n AC Tip  voltage = %d  DC Tip voltage = %d",  *acVloop/SAMPLES,*dcVtip/SAMPLES);
	printf("\n AC Ring voltage = %d  DC Ring voltage = %d",  *acVloop/SAMPLES,*dcVring/SAMPLES);
	printf("\n AC Loop voltage = %d  DC Loop voltage = %d",  *acVloop/SAMPLES,*dcVloop/SAMPLES);
			
}



/****************************************************************************************************************************
lineCap works be shifting the voltage between 60 and 55 volts quickly, and measures the time it takes to do so.  It gives very 
accurate measurements, but requires calibration on a known capicatnace.  Refer to AN71 for details.
*****************************************************************************************************************************/

//calibration values for lineCap see AN71
//#define beta 182535300							//332140256
//#define alpha 6.97206E+13						//1.20818656e8


int lineCap ( void)  

{

	// This function requires calibration per applicaton 
	const double constanttime = 8.30565e-10;
	__s64 time1,time2;
	unsigned short sum=0 ,   i , sixtyVolts, fiftyFiveVolts;
#ifdef PCM_SLIC_FLOAT		
	double finaltime,capValue;
#endif	
	
//	doReset();
//	changeCID(0);
//	initialize();
	
	writeRam(VCM,0);
	writeReg(ILIM,5);
	
	writeRam(SBIAS, 0x61);  // TURN OFF SLOW FEED

	writeReg(LINEFEED,5);  
	for (i=0;i<12;i++){
		writeRam(VOC,(unsigned short) ((i/2)*TENVOLTINC));
		delay(50);
	}
	writeRam(VOC,6*TENVOLTINC);
	delay(50);
	sixtyVolts=readRam(VTIP);
	writeRam(VOC,11*TENVOLTINC/2);
	delay(50);
	fiftyFiveVolts=readRam(VTIP);	

	time1=readTSC(); //record initial time 
	for(i=0;i<50;i++)
	{
		writeRam(VOC,6*TENVOLTINC);
		while(readRam(VTIP)!=sixtyVolts);
		writeRam(VOC,11*TENVOLTINC/2);
		while (readRam(VTIP)!=fiftyFiveVolts);
	}
	
	time2=readTSC();//record final time
	time2-=time1;
	
	writeReg(LINEFEED,0); 
	delay(50);
	printf(" \n Time = %I64i",time2);
#ifdef PCM_SLIC_FLOAT	
	time2-=beta; //subtract beta, see AN71
	
	finaltime = (double) time2;
	capValue = (finaltime/alpha)*1e6;//divide alpha, see AN71	
	//output capicatance
	printf("\n Cap = %f uF  ",  capValue);
#endif	
	return(0);		
	
}

/**********************************************************************************************************
capUsingRinging and capUsingRinging40Hz using a low level riging signal (17 Vrms ) and measures the AC current.  
GR-909 allows a ringing signal between 7Vrms and 17 Vrms to be used for diagnostics.  A lower level signal should be 
used if there is concern is the signal would cause a ring.  These functions can be used to determine values for 
both, REN and capicatance values.  To differentiate between the two, it is neccessary to try the following fuction at 
different frequencies.Here we try it at 20Hz and 40 Hz.  See AN71 for more details
*************************************************************************************************************/


int capUsingRinging( void)  // 0 to 50 volts in 5 volt increments
{ 
	unsigned short sample10ms, sample1sec;
#ifdef PCM_SLIC_FLOAT	
	double ratio ;
	double capValue;

	
	// It is necessary to lower VOC to allow the Ringing State Machine to proceed
	delay(200);
	writeRam( RINGOF	,	0);
	writeRam( RINGFRHI,0x3F78);
	writeRam( RINGFRLO,0x6CE8);
	writeRam( RINGAMP,0x4D);
	writeRam( RINGPHAS,0x0);
	writeRam( RTCOUNT,0x190);
	writeRam( RTDCTH,0x7FFF);
	writeRam( RTPER,0x28);
	writeRam( RTACTH,0x7FFF);
	writeRam( RTDCDB,3);
	writeRam( RTACDB,3);
	writeReg( RINGCON,0X0);


	writeRam(DIAGACCO,0X10);//see AN71 and Si3220 datasheet for info on how to use diagnostic registers
	writeRam(VOC,0x000); 
	writeRam(VCM,0);
	writeReg(LINEFEED,4);  // start ringing
	writeReg(DIAG,0Xcc) ; // High Preciscion current loop current
	delay(10); //wait milliseconds for it to take effect 
	sample10ms= readRam(DIAGAC);
	delay(1000);
	sample1sec= readRam(DIAGAC);
	

	ratio = 17/(sample1sec *.000003097 * 2.5);//1.11072);
	capValue = (1.25e-6*sqrt((double)(640512102400.01-(ratio*ratio))))/(125.663706*sqrt((double)((ratio*ratio)-102400)));
	//see AN71 for equation details.
	printf("\n Cap = %f uF  %d, %f ",  (capValue*1000000)-.16, sample1sec,ratio);

	writeReg(LINEFEED,1);
	writeRam(VOC,0x000); 
	
	delay(100);
#endif
	return(0);
}



int capUsingRinging40Hz( void)  
{ 
	unsigned short sample10ms, sample1sec ;
#ifdef PCM_SLIC_FLOAT	
	double capValue, ratio;
	

	// It is necessary to lower VOC to allow the Ringing State Machine to proceed
	delay(200);
	writeRam(RINGOF	,	0);
	writeRam( RINGFRHI,0x3DF8);
	writeRam( RINGFRLO,0x5440);
	writeRam( RINGAMP,0x4D);
	writeRam( RINGPHAS,0x0);
	writeRam( RTCOUNT,0x190);
	writeRam( RTDCTH,0x7FFF);
	writeRam( RTPER,0x28);
	writeRam( RTACTH,0x7FFF);
	writeRam( RTDCDB,3);
	writeRam( RTACDB,3);
	writeReg( RINGCON,0X0);

	writeRam(DIAGACCO,0X10);
	writeRam(VOC,0x000); 
	writeRam(VCM,0);

	writeReg(LINEFEED,4);  // start ringing
	writeReg(DIAG,0Xcc) ; // High Preciscion current loop current
	delay(10); //wait milliseconds for it to take effect 
	sample10ms= readRam(DIAGAC);
	delay(1000);
	sample1sec= readRam(DIAGAC);
	ratio=0;
	ratio = 17/(((double)sample1sec) *.000003097 * 5.5);

	
	//capValue = (((float)sample1sec-(float)sample10ms)*4.69)/ (float)(1346);	
	capValue = (1.25e-6*sqrt((double)(640512102400.01-(ratio*ratio))))/(251.3274*sqrt((double)((ratio*ratio)-102400)));
	//see AN71 for equation details.

	printf("\n Cap = %f uF  %d, %f ",  (capValue*1000000)-.16, sample1sec,ratio);

	writeReg(LINEFEED,1);
	writeRam(VOC,0x000); 
	
	delay(100);
#endif
	return(0);
}




/*************************************************************************************************
RENtest and LowREN work together to determine the REN loading on line.  RENtest slams the voltage from
20 volts to 10 volts. The result is a burst of current which coresponds to the the Ren Resistance for 
higher than 3 ren.  LowREN works by putting 30 volts on the line and suddenly goes to open.  The amount of 
it take for the current to die away is porprotional to the REN on the line.  Refer to AN71 for more details.
**************************************************************************************************/


void RENtest (  unsigned short *ren)
{  
	unsigned short max=0, i, readingILOOP;

//	doReset();
//	initialize(); 
//	changeCID(0);
	writeRam(VOC,0X1000);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,1);  // Go to Active
	
	delay (1000);
	writeRam(VOC, 0x800);

	delay(20);
	for (i=0;i<30	;i++)
	{
	
		readingILOOP =  0x7fff& readRam(ILOOP);
		max = (readingILOOP>max)?readingILOOP:max;

	}

	*ren = max /240;
	if (*ren >5)
		printf("\nRen >5");
	else if ((*ren) ==0)
		LowREN();
	else
		printf("\nRen = %i",*ren);
	delay (1000);

}



int LowREN( void)  
{
	unsigned short a[200], i ;
//	doReset();
//	initialize(); 
//	changeCID(0);

	writeRam(VOC,3*TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,1);  // Go to Active
	writeRam(DIAGDCCO,0X1fFF);


	delay(40);
	writeReg(LINEFEED,0);
	delay(100);
	for(i=0;i<200; i++)
	{
		delay (2);
		a[i] = readRam(VRING);
	}


	if  (((a[0]-a[4]) < 300)  && (a[4] >3410))
	{

		printf("\t\t\t\t\nREN between .175 and 1");
		return(-1);

	}

	printf("\t\t\t\t\nREN < .175");
	return(0);

}


/*************************************************************************************************
RRG/RTG work to determine the ring to ground and tip to ground resistance leakage. These functions rely 
on a 800k resistance accross tip and ring for a voltage division measurement.  At the point where a V/I 
measurement becomes more accurate, that resistance value is used instead.  It is good practice to run these
functions multiple times and over different voltages and average the results together for an accurate 
measurement. Refer to AN71 for more details.
******************************************************************************************************/



void RRG ( )  // 0 to 50 volts in 5 volt increments

{ 
#ifdef PCM_SLIC_FLOAT	
	double tipVoltage,ringVoltage, ringcurrent,rrgdiv, rrgImeth, rrgout, tipRingVoltage;
//	doReset();
//	changeCID(0);
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,5);  // Go to Active
//	while(1);
	writeRam(VOC,5*TENVOLTINC/3 ); // Write out voltages starting with zero and going to 50/3 ; this is the expected voltage on the high imp resistor
//	delay (100);
	writeReg(LINEFEED,7);
	writeRam(VOC,5*TENVOLTINC );
		delay(1000);// wait a long time for capassitance to settle
		ringVoltage = -1 *.00492 * (float) readRam(VRING);
		tipVoltage = -1 * .00492 * (float) readRam(VTIP);
		tipRingVoltage = tipVoltage-ringVoltage;
		rrgdiv = (ringVoltage*800)/((tipRingVoltage)-(2*ringVoltage)+3);
	writeRam(VOC,2 * TENVOLTINC); // step through voltage slowly to avoid ringing ringer
	writeReg(LINEFEED,3); // Activate reverse active (low resistance feed)
	writeRam(VOC,5 * TENVOLTINC);// step up other direction ( Foward active)
	delay (1000);
	ringcurrent = -1*(readRam(IRING)&0x7FFF) * .000003097;
	ringVoltage = -1*.00492 * (float) readRam(VRING);
	tipVoltage = -1 * .00492 * (float) readRam(VTIP);
	tipRingVoltage = tipVoltage-ringVoltage;
	rrgImeth =ringVoltage/(1000*ringcurrent-((ringVoltage-1.5)/402) - ((ringVoltage-tipVoltage)/800));
	if (rrgdiv<15 && rrgdiv>5){
		rrgout = rrgImeth;
	}
	else
	{
		rrgout = rrgdiv;
	}
	if (rrgout<0)
		rrgout =1000;

	printf("\nR R-G = %.0f ohms", rrgout*1000);
#endif	
}



void RTG ( )  // 0 to 50 volts in 5 volt increments

{ 
#ifdef PCM_SLIC_FLOAT		
	double tipVoltage,ringVoltage,tipcurrent, rrgdiv, rrgImeth, rrgout, RingtipVoltage;

//	doReset();
//	changeCID(0);
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,5);  // Go to Active
	writeRam(VOC,5*TENVOLTINC/3 ); // Write out voltages starting with zero and going to 50/3 ; this is the expected voltage on the high imp resistor
//	delay (100);
	writeReg(LINEFEED,3);
	writeRam(VOC,5*TENVOLTINC );
		delay(1000);// wait a long time for capassitance to settle
		ringVoltage = -1 *.00492 * (float) readRam(VRING);
		tipVoltage = -1 * .00492 * (float) readRam(VTIP);
		tipcurrent = readRam(ILONG);//*.0000031;
		RingtipVoltage = ringVoltage-tipVoltage;
		rrgdiv = (tipVoltage*800)/((RingtipVoltage)-(2*tipVoltage)+3);
	writeRam(VOC,2 * TENVOLTINC); // step through voltage slowly to avoid ringing ringer
	writeReg(LINEFEED,7); // Activate reverse active (low resistance feed)
	writeRam(VOC,5 * TENVOLTINC);// step up other direction ( Foward active)
	delay (1000);
	tipcurrent = -1*(readRam(ITIP)&0x7FFF) * .0000031;
	tipVoltage = -1*.00492 * (float) readRam(VRING);
	ringVoltage = -1*.00492 * (float) readRam(VRING);
	rrgImeth =tipVoltage/(1000*tipcurrent-((tipVoltage-1.5)/400) - ((tipVoltage-ringVoltage)/800));
	if (rrgdiv<15 && rrgdiv>5){
		rrgout = rrgImeth;
	}
	else
	{
		rrgout = rrgdiv;
	}
	if (rrgout<0)
		rrgout =1000;

	printf("\nR T-G = %.0f ohms", rrgout*1000);
#endif		
}

/*****************************************************************************************************
RTRtipopen and RTRringopen are used to determine the tip-ring resistance on the line.  They use 
ground-start and reverse ground start states to mesure the resistance using voltage division with the 
Si3220 402k sense resistors.  Refer to AN71 for more details.
*****************************************************************************************************/




void RTRtipopen ( )  // 0 to 50 volts in 5 volt increments

{ 
#ifdef PCM_SLIC_FLOAT		
	double tipVoltage,ringVoltage, rrgdiv, RingtipVoltage, rrgdiv2;
//	doReset();
//	changeCID(0);
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,5);  // Go to Active
	writeRam(VOC,5*TENVOLTINC/3 ); // Write out voltages starting with zero and going to 50/3 ; this is the expected voltage on the high imp resistor
//	delay (100);
	writeReg(LINEFEED,3);
	writeRam(VOC,5*TENVOLTINC );
		delay(1000);// wait a long time for capassitance to settle
		ringVoltage = -1 *.00492 * (float) readRam(VRING);
		tipVoltage = -1 * .00492 * (float) readRam(VTIP);
		RingtipVoltage = ringVoltage-tipVoltage;
		rrgdiv = ((RingtipVoltage)/((tipVoltage-1.5)/402));
		rrgdiv = rrgdiv*1000;
		rrgdiv2 = (rrgdiv*800000)/(800000-rrgdiv);
	writeRam(VOC,2 * TENVOLTINC); // step through voltage slowly to avoid ringing ringer
	writeRam(VOC,0);
	
	printf("\nR T-Rtipopen = %.0f ohms", rrgdiv2);
#endif
}

void RTRringopen ( )  // 0 to 50 volts in 5 volt increments

{ 
#ifdef PCM_SLIC_FLOAT		
	double tipVoltage,ringVoltage, rrgdiv, tipRingVoltage,rrgdiv2;
//	doReset();
//	changeCID(0);
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,5);  // Go to Active
	writeRam(VOC,5*TENVOLTINC/3 ); // Write out voltages starting with zero and going to 50/3 ; this is the expected voltage on the high imp resistor
//	delay (100);
	writeReg(LINEFEED,7);
	writeRam(VOC,5*TENVOLTINC );
		delay(1000);// wait a long time for capassitance to settle
		ringVoltage = -1 *.00492 * (float) readRam(VRING);
		tipVoltage = -1 * .00492 * (float) readRam(VTIP);
		tipRingVoltage = tipVoltage-ringVoltage;
		rrgdiv = ((tipRingVoltage)/((ringVoltage-1.5)/402));
		rrgdiv = rrgdiv*1000;
		rrgdiv2 = (rrgdiv*800000)/(800000-rrgdiv);
	writeRam(VOC,2 * TENVOLTINC); // step through voltage slowly to avoid ringing ringer
	writeRam(VOC,0);
	
	printf("\nR T-Rringopen = %.0f ohms", rrgdiv2);
#endif
}


void RTRVImeth()
{
#ifdef PCM_SLIC_FLOAT		
	double rtr, rtr2, loopVoltage, loopcurrent;
//	doReset();
//	changeCID(0);
//	initialize(); 

	writeRam(VOC,5*TENVOLTINC);
	writeReg(LINEFEED,1);
	delay(1000);
	loopVoltage = -1 *.00492 * (float) readRam(VLOOP);
//	tipVoltage = -1 * .00492 * (float) readRam(VTIP);
//	tipRingVoltage = tipVoltage-ringVoltage;
	
	loopcurrent = -1*(readRam(ILOOP)&0x7FFF) * .000003097;
	
	rtr = loopVoltage/loopcurrent;
	rtr2 = (rtr*800000)/(800000-rtr);
	printf("\nR T-R V over I = %.0f ohms", rtr2);
#endif
}





/* By stepping through the forward active voltages with 0<VOC<50 voltage we will see the leakage the Tip to Ring lead */
/* The stepping through will also identify any non-linear devices connected between Ring and Ground */



int tipRingCurrentOverVoltageFwrd( short currentArray[])  // 0 to 50 volts in 5 volt increments
{  
	short vocValue  , i ;
//	doReset();
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);
	writeRam(VOC,TENVOLTINC);
	writeRam(VOCDELTA,0);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,1);  // Go to  Forward Active
	writeReg(DIAG,0XCC) ; // High Preciscion current loop current

	writeRam(DIAGDCCO,0X20);
	for ( vocValue = TENVOLTINC, i=0 ; i < 5; i++) 
	{

		delay(20);
		writeRam(VOC, vocValue); // Write out voltages starting with zero and going to 50 volts
		vocValue += TENVOLTINC;
		
		writeRam(DIAGDCCO,FLUSH);
		delay(100);
		writeRam(DIAGDCCO,LPFPOLE);
		delay(PROCESSING);
		currents[i] = readRam(DIAGDC);

	}
	printf("\n  RingtoTip=");
	for (i=0;i<5;i++) printf(" %d ",  currents[i]);
	
	if ((currents[3]>200) && (currents[4]> 350))
	{
		printf("\nTip to Ring Short");
			return (0);
	}
	return(-1);
}


void tipRingCurrentOverVoltageRev( short currents[])  // 0 to 50 volts in 5 volt increments
{ 
	unsigned short vocValue =0 , i ;
//	doReset();
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);
	writeRam(VOC,TENVOLTINC);
	writeRam(VOCDELTA,0);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,5);  // Go to Active
	writeReg(DIAG,0XCC) ; // High Preciscion current loop current
	writeRam(DIAGDCCO,0X20);
	for ( vocValue = TENVOLTINC, i=0 ; i < 5; i++) 
	{

		delay(20);
		writeRam(VOC, vocValue); // Write out voltages starting with zero and going to 50 volts
		vocValue += TENVOLTINC;
		
		writeRam(DIAGDCCO,FLUSH);
		delay(100);
		writeRam(DIAGDCCO,LPFPOLE);
		delay(PROCESSING);
		currents[i] = readRam(DIAGDC);
		
	}
	printf("\n  TiptoRing =");
	for (i=0;i<5;i++) printf(" %d ",  currents[i]);
}








/* By stepping through the forward active voltages with VOC = 0 and 0<VCM<50 we will see the leakage for the Ring lead */
/* The stepping through will also identify any non-linear devices connected between Ring and Ground */



int ringToGroundCurrent( short currents[])  // 0 to 50 volts in 10 volt increments
{ 
	unsigned short vocValue  , i ;
//	doReset();
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF Standby State
	writeReg(LINEFEED,3);  // Go to Active
	writeReg(DIAG,0XCD) ; // High Preciscion current loop current
	writeRam(DIAGDCCO,0X20);  
	for ( vocValue = TENVOLTINC, i=0 ; i < 5; i++) 
	{

		delay(20);
		writeRam(VOC, vocValue); // Write out voltages starting with zero and going to 50 volts
		vocValue += TENVOLTINC;
		
		writeRam(DIAGDCCO,FLUSH);
		delay(100);
		writeRam(DIAGDCCO,LPFPOLE);
		delay(PROCESSING);
		currents[i] = readRam(DIAGDC);

	}
	printf("\n RingShort  =");
	for (i=0;i<5;i++) printf(" %d ",  currents[i]);
	
	if ((currents[4]< -100) && (currents[4]< -300))
	{
		printf("\n Ring to Ground Short");
			return (0);
	}
	return(-1);
}

int tipToGroundCurrent( short currents[])  // 0 to 50 volts in 5 volt increments
{ 
	unsigned short vocValue ,  i ;
//	doReset();
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,7);  // Go to Active
	writeReg(DIAG,0XCD) ; // High Preciscion current loop current
	writeRam(DIAGDCCO,0X20);
	for ( vocValue = TENVOLTINC, i=0 ; i < 5; i++) 
	{

		delay(20);
		writeRam(VOC, vocValue); // Write out voltages starting with zero and going to 50 volts
	
		vocValue += TENVOLTINC;
		
		writeRam(DIAGDCCO,FLUSH);
		delay(100);
		writeRam(DIAGDCCO,LPFPOLE);
		delay(PROCESSING);
		currents[i] = readRam(DIAGDC);

	}
	printf("\n TipShort  =");
	for (i=0;i<5;i++) printf(" %d ",  currents[i]);
	if ((currents[4]< -100) && (currents[4]< -300))
	{
		printf("\nTip to Ground Short");
		return (0);
	}
	return(0);
}



int fusePresent ( short scratch[])  // 0 to 50 volts in 5 volt increments

{ 
	unsigned short vocValue ,  average, i ,sum=0 ;
//	doReset();
//	initialize();  /* initialize one Dual ProSLIC */
	writeRam(VCM,0);

	writeRam(VOC,TENVOLTINC);
	writeRam(SBIAS, 0X61);  // TURN OFF SLOW FEED
	writeReg(LINEFEED,7);  // Go to Active
	for ( vocValue = TENVOLTINC, i=0 ; i < 5; i++) 
	{
		delay(20);
		writeRam(VOC, vocValue); // Write out voltages starting with zero and going to 50 volts
		delay(100);
		//currents[i] = readRam(VTIP);
		scratch[i] = readRam(VRING);
		vocValue += TENVOLTINC;	
	}
	printf("\n FusePresent  =");
	for (i=1;i<5;i++)
	{
	 printf(" %d ",  scratch[i]-scratch[i-1]);
	 sum+=scratch[i];
	}
	
	{
		 if (scratch[i] == 0)
		 {
			 printf("\n\t\t\t ++++Fuse Blown+++");
			 return 0;
		 }
		
		average=sum/4;
		printf("\n average = %d",average);
		printf("\n\t\t\t ---%s---", (abs(average-660) < 80) ? "No Phone Attached": "Line or Load Attached");
	}
	return((abs(average-660) < 80));
}

/* By stepping through the reverse active voltages with VOC = 0 and 0<VCM<50 we will see the leakage for the Tip lead */
/* The stepping through will also identify any non-linear devices connected between Tip and Ground */



void logitudinalCurrentOverVoltageReverseAcitive(unsigned short currents[])  // 0 to 50 volts in 5 volt increments
{ 
	unsigned short vcmValue =0 , i ;
	writeReg(LINEFEED,5);  // Go to Reverse Active
	delay (30);
	writeReg(DIAG,0XCD);  // High Preciscion current loop current
	writeRam(VOC, 0 );
	for ( vcmValue = 0, i=0 ; i < 50; i++) 
	{
		writeRam(VCM, vcmValue); // Write out voltages starting with zero and going to 50 volts
		vcmValue += ONEVOLTINC;
		delay(40000/64);
		currents[i] = readRam(DIAGDC);
	}

}



