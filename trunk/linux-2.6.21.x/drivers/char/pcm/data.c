/*******************************************************************************
						Data.c Copyright 2002 Silicon Laboratories

This file contains the neccessary data for the main program.
************************************************************************************/
#include "dual_io.h"
//#include "stdlib.h"
#include "dual.h"
#include <asm/types.h>


unsigned char currentChannel =0;
char * TYPE[] =
{ 
	"Si3220",
	"Si3221",
	"Si3225",
	"Si3232"
};

char * REVISION[]=
{
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
};

char * PFREQUENCY[]=
{
	"",
	" ",
	"2048",
};



__s64 TicksPerSecond =0;



int numberOfChannels = MAXCHANNELS ;

channelStruct *pCCData, channelData[MAXCHANNELS];

 

char * szRamSymbols[]=
{
 szVOC         	,
 szVOCDELTA    	,
 szVOCLTH      	,
 szVOCHTH      	,
 szVCM         	,
 szVOV         	,
 szVOVRING     	,
 szVLOOP         	,
 szILOOP         	,
 szILONG        	,
 szVOCTRACK    	,
 szVTIP    	,
 szVRING   	,
 szVBAT    	,
 szITIPP     	,
 szIRINGP     	,
 szIRINGN     	,
 szITIPN     	,
 szIRING     	,
 szITIP     	,
 szVRNGNG   	,
 szIRNGNG   	,
 szLCROFFHK     	,
 szLCRONHK      	,
 szLCRDBI      	,
 szLCRLPF      	,
 szLCRMASK     	,
 szLONGHITH    	,
 szLONGLOTH    	,
 szLONGDBI     	,
 szLONGLPF     	,
 szBATHTH      	,
 szBATLTH      	,
 szBSWLPF      	,
 szBATLPF      	,
 szCMLOTH      	,
 szCMHITH      	,
 szPTH12      	,
 szPTH34      	,
 szPTH56      	,
 szPLPFQ12        	,
 szPLPFQ34        	,
 szPLPFQ56        	,
 szRB56        	,
 szPQ1DH       	,
 szPQ2DH       	,
 szPQ3DH       	,
 szPQ4DH       	,
 szPQ5DH       	,
 szPQ6DH       	,
 szPSUM       	,
 szDIAGDCTH    	,
 szDIAGDCCO    	,
 szDIAGACTH    	,
 szDIAGACCO    	,
 szDIAGPK      	,
 szRINGOF      	,
 szRINGFRHI    	,
 szRINGFRLO    	,
 szRINGAMP     	,
 szRINGPHAS    	,
 szRTCOUNT     	,
 szRTDCTH      	,
 szRTPER       	,
 szRTACTH      	,
 szRTDCDB      	,
 szRTACDB      	,
 szPMFREQ      	,
 szPMAMPL      	,
 szPMRAMP      	,
 szPMAMPTH     	,
 szRXGAIN     	,
 szTXGAIN     	,
 szTXEQC03    	,
 szTXEQC02    	,
 szTXEQCO1    	,
 szTXEQCO0    	,
 szRXEQCO3    	,
 szRXEQCO2    	,
 szRXEQCO1    	,
 szRXEQCO0    	,
 szRXIIRPOL    	,
 szECCO1       	,
 szECCO2       	,
 szECCO3       	,
 szECCO4       	,
 szECCO5       	,
 szECCO6       	,
 szECCO7       	,
 szECCO0       	,
 szECIIRB0     	,
 szECIIRB1     	,
 szECIIRA1     	,
 szECIIRA2     	,
 szOSC1FREQ    	,
 szOSC1AMP     	,
 szOSC1PHAS    	,
 szOSC2FREQ    	,
 szOSC2AMP     	,
 szOSC2PHAS    	,
 szFSKFREQ0    	,
 szFSKFREQ1    	,
 szFSKAMP0     	,
 szFSKAMP1     	,
 szFSK01HI   	,
 szFSK01LO   	,
 szFSK10HI   	,
 szFSK10LO   	,
 szDTROW0TH      	,
 szDTROW1TH      	,
 szDTROW2TH      	,
 szDTROW3TH      	,
 szDTCOLTH       	,
 szDTFTWTH       	,
 szDTRTWTH       	,
 szDTROWRTH      	,
 szDTCOLRTH      	,
 szDTROW2HTH     	,
 szDTCOL2HTH     	,
 szDTMINPTH    	,
 szDTHOTTH    	,
 szRXPWR       	,
 szTXPWR       	,
 szRXMODPWR    	,
 szTXMODPWR    	,
 szTESTB0L1    	,
 szTESTB0H1    	,
 szTESTB1L1    	,
 szTESTB1H1    	,
 szTESTB2L1    	,
 szTESTB2H1    	,
 szTESTA1L1    	,
 szTESTA1H1    	,
 szTESTA2L1    	,
 szTESTA2H1    	,
 szTESTB0L2    	,
 szTESTB0H2    	,
 szTESTB1L2    	,
 szTESTB1H2    	,
 szTESTB2L2    	,
 szTESTB2H2    	,
 szTESTA1L2    	,
 szTESTA1H2    	,
 szTESTA2L2    	,
 szTESTA2H2    	,
 szTESTB0L3    	,
 szTESTB0H3    	,
 szTESTB1L3    	,
 szTESTB1H3    	,
 szTESTB2L3    	,
 szTESTB2H3    	,
 szTESTA1L3    	,
 szTESTA1H3    	,
 szTESTA2L3    	,
 szTESTA2H3    	,
 szTESTPKO     	,
 szTESTABO     	,
 szTESTWLN     	,
 szTESTAVBW    	,
 szTESTPKFL    	,
 szTESTAVFL    	,
 szTESTPKTH    	,
 szTESTAVTH    	,
 szTXHPF1     	,
 szTXHPF2     	,
 szTXHPF3     	
 };



struct connection  connectionTable[8];
  


char * szStates[]=
{ 
	"MAKEbUSY", 
	"STATEcHANGE",
	"DIALtONE",
	"INITIALIZING",
	"POWERuP",
	"CALIBRATE",
	"PROGRAMMEDrING",
	"POWERdOWN",
	"POWERlEAKtEST",
	"MAKErINGbACKjAPAN", 
	"MAKEbUSYjAPAN", 
	"RINGbACKjAPAN", 
	"MAKErINGbACK",
	"RINGbACK",
	"MAKErEORDER",
	"REORDER",
	"MAKEcONGESTION",
	"CONGESTION",
	"PRENEON",
	"NEON",
	"CALLBACKpROGRAMMED", 
	"BUSY", 
	"CALLbACK", 
	"CALLING", 
	"MAKEoFFHOOK",
	"ONHOOK", 
	"OFFHOOK", 
	"DIGITDECODING",
	"OFFHOOK2",
	"LOOPtRANSITION", 
	"FIRSTrING", 
	"DEFEREDcALLBACKpROGRAMMED",
	"CALLERiD",
	"RINGING",
	"DTMFtRANISTION",
	"MAKEpULSE",
	"PULSING",
	"DIALpULSE"
} ;


