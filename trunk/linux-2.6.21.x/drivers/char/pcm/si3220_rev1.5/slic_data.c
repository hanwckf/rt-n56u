#include "dual_io.h"
#include "dual.h"
#include "registers.h"
#include "sram.h"

/*****************************************************************************
initSram Values are the SRAM values loaded during initlization.  The actual values 
are found in sram.h
**********************************************************************************/

unsigned short initSramValues[] =
{
initVOC        	,
initVOCDELTA   	,
initVOCLTH     	,
initVOCHTH     	,
initVCM        	,
initVOV        	,
initVOVRING    	,
initVLOOP        	,
initILOOP        	,
initILONG       	,
initVOCTRACK   	,
initVTIP   	,
initVRING  	,
initVBAT   	,
initITIPP    	,
initIRINGP    	,
initIRINGN    	,
initITIPN    	,
initIRING    	,
initITIP    	,
initVRNGNG  	,
initIRNGNG  	,
initLCROFFHK    	,
initLCRONHK     	,
initLCRDBI     	,
initLCRLPF     	,
initLCRMASK    	,
initLONGHITH   	,
initLONGLOTH   	,
initLONGDBI    	,
initLONGLPF    	,
initBATHTH     	,
initBATLTH     	,
initBSWLPF     	,
initBATLPF     	,
initCMLOTH     	,
initCMHITH     	,
initPTH12     	,
initPTH34     	,
initPTH56     	,
initPLPFQ12       	,
initPLPFQ34       	,
initPLPFQ56       	,
initRB56       	,
initPQ1DH      	,
initPQ2DH      	,
initPQ3DH      	,
initPQ4DH      	,
initPQ5DH      	,
initPQ6DH      	,
initPSUM      	,
initDIAGDCTH   	,
initDIAGDCCO   	,
initDIAGACTH   	,
initDIAGACCO   	,
initDIAGPK     	,
initRINGOF     	,
initRINGFRHI   	,
initRINGFRLO   	,
initRINGAMP    	,
initRINGPHAS   	,
initRTCOUNT    	,
initRTDCTH     	,
initRTPER      	,
initRTACTH     	,
initRTDCDB     	,
initRTACDB     	,
initPMFREQ     	,
initPMAMPL     	,
initPMRAMP     	,
initPMAMPTH    	,
initRXGAIN    	,
initTXGAIN    	,
initTXEQC03   	,
initTXEQC02   	,
initTXEQCO1   	,
initTXEQCO0   	,
initRXEQCO3   	,
initRXEQCO2   	,
initRXEQCO1   	,
initRXEQCO0   	,
initRXIIRPOL   	,
initECCO1      	,
initECCO2      	,
initECCO3      	,
initECCO4      	,
initECCO5      	,
initECCO6      	,
initECCO7      	,
initECCO0      	,
initECIIRB0    	,
initECIIRB1    	,
initECIIRA1    	,
initECIIRA2    	,
initOSC1FREQ   	,
initOSC1AMP    	,
initOSC1PHAS   	,
initOSC2FREQ   	,
initOSC2AMP    	,
initOSC2PHAS   	,
initFSKFREQ0   	,
initFSKFREQ1   	,
initFSKAMP0    	,
initFSKAMP1    	,
initFSK01HI  	,
initFSK01LO  	,
initFSK10HI  	,
initFSK10LO  	,
initDTROW0TH     	,
initDTROW1TH     	,
initDTROW2TH     	,
initDTROW3TH     	,
initDTCOLTH      	,
initDTFTWTH      	,
initDTRTWTH      	,
initDTROWRTH     	,
initDTCOLRTH     	,
initDTROW2HTH    	,
initDTCOL2HTH    	,
initDTMINPTH   	,
initDTHOTTH   	,
initRXPWR      	,
initTXPWR      	,
initRXMODPWR   	,
initTXMODPWR   	,
initTESTB0L1   	,
initTESTB0H1   	,
initTESTB1L1   	,
initTESTB1H1   	,
initTESTB2L1   	,
initTESTB2H1   	,
initTESTA1L1   	,
initTESTA1H1   	,
initTESTA2L1   	,
initTESTA2H1   	,
initTESTB0L2   	,
initTESTB0H2   	,
initTESTB1L2   	,
initTESTB1H2   	,
initTESTB2L2   	,
initTESTB2H2   	,
initTESTA1L2   	,
initTESTA1H2   	,
initTESTA2L2   	,
initTESTA2H2   	,
initTESTB0L3   	,
initTESTB0H3   	,
initTESTB1L3   	,
initTESTB1H3   	,
initTESTB2L3   	,
initTESTB2H3   	,
initTESTA1L3   	,
initTESTA1H3   	,
initTESTA2L3   	,
initTESTA2H3   	,
initTESTPKO    	,
initTESTABO    	,
initTESTWLN    	,
initTESTAVBW   	,
initTESTPKFL   	,
initTESTAVFL   	,
initTESTPKTH   	,
initTESTAVTH   	,
initTXHPF1    	,
initTXHPF2    	,
initTXHPF3    	,
} ;


/*********************************************************************************
These special values are the ones for External Ringing (Si3225).  If the Si3220 is 
being used, it is not necessary to load the additional values.  The demo code supports
both, internal and external ringing.
**********************************************************************************/


unsigned short initextSRAMValues[][2]=
{
	{LFSDELAY,initLFSDELAY},
	{ZERDELAY,initZERDELAY},
	{RTDCTH,initextRTDCTH},
	{RTACTH,initextRTACTH},
	{0,0}
};


/********************************************************************************
These are the direct register settings that are set during initlization.  These may
need to be tweeked for different applications.

**********************************************************************************/


unsigned char direct_init_table[][2] = 
{
	//impedance coefficents
	{	ZRS		,	0x05},//set for 600 ohms, use coefficent generator for values
	{	ZZ		,	0x01},
	{	ZB0LO	,	0xA0},
	{	ZB0MID	,	0x1E},
	{	ZB0HI	,	0x0E},
	{	ZB1LO	,	0x3C},
	{	ZB1MID	,	0xE2},
	{	ZB1HI	,	0xF0},
	{	ZB2LO	,	0x5F},
	{	ZB2MID	,	0xE7},
	{	ZB2HI	,	0xF4},
	{	ZB3LO	,	0x01},
	{	ZB3MID	,	0x18},
	{	ZB3HI	,	0x0C},
	{	ZA1LO	,	0x85},
	{	ZA1MID	,	0x4C},
	{	ZA1HI	,	0x0F},
	{	ZA2LO	,	0x66},
	{	ZA2MID	,	0xB3},
	{	ZA2HI	,	0xF8},
	
	
	{	AUDGAIN	,	0x0},
	{	ILIM	,	0x05},//set for 22mA

	{	PCMMODE	,	0x00},

	//oscillators coefficents
	{	O1TAHI	,	0x00},
	{	O1TALO	,	0x00},
	{	O1TIHI	,	0x00},
	{	O1TILO	,	0x00},
	{	O2TAHI	,	0x00},
	{	O2TALO	,	0x00},
	{	O2TIHI	,	0x00},
	{	O2TILO	,	0x00},
	{	OCON	,	0x00},
	{	OMODE	,	0x02},

	{	TONDEN	,	0x00},
	
	//ringing coefficents
	{	RINGCON	,	0x18},//sinusoidal, balanced, timers enabled
	
	{	RINGTAHI,	0x00},
	{	RINGTALO,	0x00},
	{	RINGTIHI,	0x00},
	{	RINGCON	,	0x00},
	
	//external ringing relay enable
	{	RLYCON	,	0xA3},// used only for external ringing,  can remove is using the Si3220

	//pulse metering coefficents
	{	PMTAHI	,	0x00},//pulse metering
	{	PMTALO	,	0x00},
	{	PMTIHI	,	0x00},
	{	PMTILO	,	0x00},
	{	PMCON	,	0x00},

	{	POLREV	,	0x00},//normal polarity
	{	SBIAS	,	0xE0},//bias current set to 4mA
	
};
