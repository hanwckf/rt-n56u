/*
** Si321x Function Definitions/Datatypes
*/
#ifndef SI321XH_H
#define SI321XH_H

#include "proslic.h"


#define SI321X_POWERUP_VOLT_THRESH 0x11


typedef struct {
	Oscillator_Cfg osc1;
	Oscillator_Cfg osc2;
	uInt8 omode1;
	uInt8 omode2;
} Si321x_Tone_Cfg; 


/*
** Defines structure for configuring ring generator
*/
typedef struct {
	ramData rngx;
	ramData rngy;
	ramData roff;
	ramData rco;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
	uInt8 ringcon;
	uInt8 rtdi;
	uInt16 nrtp;
} Si321x_Ring_Cfg;

/*
** Defines structure for configuring gpio
*/
typedef void Si321x_GPIO_Cfg;

/*
** Defines structure for configuring dc feed
*/
typedef struct {
	uInt8 ilim;
	uInt8 voc;
	uInt8 vcm;
} Si321x_DCfeed_Cfg;

/*
** Defines structure for configuring pcm
*/
typedef struct {
	uInt8 pcmf;
	uInt8 wbe;
	uInt8 tri;
} Si321x_PCM_Cfg;

/*
** Defines structure for configuring pulse metering
*/
typedef struct {
	ramData plsco;
	ramData plsx;
	ramData plsd;
	uInt16 pat;  //Active Timer (Hi & Low byte)
	uInt16 pit;  //Inactive Timer (Hi & Low byte)
} Si321x_PulseMeter_Cfg;


/*
** Defines structure for configuring FSK generation
*/
typedef struct {
	ramData fsk01;
	ramData fsk10;
	ramData fsk0x; //amp
	ramData fsk1x;	//amp
	ramData fsk0; //freq
	ramData fsk1; //freq
} Si321x_FSK_Cfg;

/*
** Defines structure for configuring dtmf decode
*/
typedef void Si321x_DTMFDec_Cfg;

/*
** Defines structure for configuring impedence synthesis
*/
typedef struct {
	uInt8 clc;
	uInt8 tiss;
} Si321x_Zsynth_Cfg;

/*
** Defines structure for configuring hybrid
*/
typedef void Si321x_hybrid_Cfg;

/*
** Defines structure for configuring GCI CI bits
*/
typedef void Si321x_CI_Cfg;

/*
** Defines structure for configuring modem tone detect
*/
typedef void Si321x_modemDet_Cfg;

/*
** Defines structure for configuring audio eq
*/
typedef void Si321x_audioEQ_Cfg;

/*
** Defines structure for configuring audio gain
*/
typedef struct {
	uInt8 gain;
	uInt16 digGain;
} Si321x_audioGain_Cfg;

/*
** Defines structure for configuring ring trip
*/
typedef struct {
	uInt8 rtdi;
	ramData rptp;
	ramData nrtp;
} Si321x_ringTrip_Cfg;

/*
** Defines country config structure
*/
typedef struct {
	uInt16 countryCode;
	ProslicRegInit *registers;
	ProslicRAMInit *rams;
	Si321x_FSK_Cfg fsk;
	Si321x_Ring_Cfg ring;
	Si321x_ringTrip_Cfg ringtrip;
	Si321x_Zsynth_Cfg zsynth;
	Si321x_DCfeed_Cfg dcfeed;
//	audioEQ_Cfg audioEQ;
//	hybrid_Cfg hybrid;
//	modemDet_Cfg modemDet;
} Si321x_country_Cfg;

/*
** This defines names for the interrupts in the ProSLIC
*/

#define OSC1_T1_SI321X 0 
#define OSC1_T2_SI321X 1
#define OSC2_T1_SI321X 2
#define OSC2_T2_SI321X 3
#define RING_T1_SI321X 4
#define RING_T2_SI321X 5
#define PM_T1_SI321X 6
#define PM_T2_SI321X 7
#define RING_TRIP_SI321X 8
#define LOOP_STAT_SI321X 9
#define PQ1_SI321X 10
#define PQ2_SI321X 11
#define PQ3_SI321X 12
#define PQ4_SI321X 13
#define PQ5_SI321X 14
#define PQ6_SI321X 15
#define DTMF_SI321X 16
#define INDIRECT_SI321X 17
#define CM_BAL_SI321X 18


#endif
