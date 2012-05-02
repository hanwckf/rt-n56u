/*******************************************************************************
						ISR.c Copyright 2002 Silicon Laboratories

This file gathers interrupts and sends it back to the main file for processing.
************************************************************************************/

#include "dual_io.h"
#include "dual.h"

unsigned long interruptBits () {
	// Determines which interrupt bit is set
	int  count = 1;
	unsigned char j = 0x01 ;
	union {
		unsigned char reg_data[4];
		long interruptBits;
	} unionOfInterruptBits ;
	unionOfInterruptBits.interruptBits=0;
	
// ONLY CLEAR the ACTIVE INTERRUPT or YOU WILL CREATE CRITICAL SECTION ERROR of LEAVING
// THE TIME OPEN BETWEEN THE Nth BIT and the N+1thbit within the same BYTE.
// eg. if the inactive oscillators are firing at nearly the same time
// you would only see one.


	unionOfInterruptBits.reg_data[0] = readReg( IRQ1);

	unionOfInterruptBits.reg_data[1] = readReg( IRQ2);
	 
	unionOfInterruptBits.reg_data[2] = readReg( IRQ3);

	writeReg(IRQ1, 0);
	writeReg(IRQ2, 0);
	writeReg(IRQ3, 0);
	return unionOfInterruptBits.interruptBits ;

}



unsigned char interruptChannelData(void) {
	int i=0, data;

	data = Interrupt();

	//if ( data != 0 ){
	

		pCCData->interrupt = interruptBits(); // Store which interrupt occured for which channel.


	//}
	

	return data;
}