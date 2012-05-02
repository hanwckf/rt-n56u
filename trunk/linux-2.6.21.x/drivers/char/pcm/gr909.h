

/*
*Silicon Laborities Si322x  G909.h MLT Header File
*
* Copyright 2002 Silicon Laborities Inc.  Austin, Texas
* Author: Silicon Labs Applications.
*            website www.silabs.com
*   
*/
void doReset(void);

void openVoltages ( 
unsigned short *acVloop, unsigned short *dcVloop,				   
unsigned short *acVring, unsigned short *dcVring,
unsigned short *acVtip, unsigned short *dcVtip				   
 );

void openVoltages2 ( 
unsigned short *acVloop, unsigned short *dcVloop,				   
unsigned short *acVring, unsigned short *dcVring,
unsigned short *acVtip, unsigned short *dcVtip				   
 );

int tipRingCurrentOverVoltageFwrd( short currentArray[]); 


void tipRingCurrentOverVoltageRev( short currents[]); 
 
void tipRingCurrentOverVoltageRevPlus( short currents[]); 

int ringToGroundCurrent( short currents[]);  

int tipToGroundCurrent( short currents[]); 
 
 
int LowREN( void); 
 
int lineCap ( void); 

void RENtest (  unsigned short *ren);
 

int fusePresent ( short scratch[]);
 
void logitudinalCurrentOverVoltageReverseAcitive(unsigned short currents[]); 
 
int capUsingRinging( void); 
 
int capUsingRinging40Hz( void);

void RTG ( ) ;

void RTRtipopen ( ) ;

void RTRringopen ( );

void RRG ( );


void RTRVImeth();

#define SAMPLES 50
#define VSCALE .004907 
#define ONEVOLTINC 207 // one volt increment for the VOC and other voltage registers
#define TENVOLTINC 2070
#define PROCESSING 500
#define FLUSH 0X7FFF
#define LPFPOLE 250 *500 / PROCESSING