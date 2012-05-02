/*
** Copyright (c) 2007 by Silicon Laboratories
**
** $Id: si324x.h,v 1.1 2010-07-30 07:55:38 qwert Exp $
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
**
*/

#ifndef SI324XH_H
#define SI324XH_H

#include "proslic.h"

/*
** Si324x DataTypes/Function Definitions 
*/

#define NUMIRQ 4


/*
** Defines structure for configuring gpio
*/
typedef struct {
	uInt8 outputEn;
	uInt8 analog;
	uInt8 direction;
	uInt8 manual;
	uInt8 polarity;
	uInt8 openDrain;
} Si324x_GPIO_Cfg;

/*
** Defines structure for configuring dc feed
*/
typedef struct {
	ramData slope_vlim;
	ramData slope_rfeed;
	ramData slope_ilim;
	ramData delta1;
	ramData delta2;
	ramData v_vlim;
	ramData v_rfeed;
	ramData v_ilim;
	ramData const_rfeed;
	ramData const_ilim;
	ramData i_vlim;
	ramData lcronhk;
	ramData lcroffhk;
	ramData lcrdbi;
	ramData longhith;
	ramData longloth;
	ramData longdbi;
	ramData lcrmask;
	ramData lcrmask_polrev;
	ramData lcrmask_state;
	ramData lcrmask_linecap;
	ramData vcm_oh;
	ramData vov_bat;
	ramData vov_gnd;
} Si324x_DCfeed_Cfg;

/*
** Defines structure for configuring general parameters
*/
typedef struct {
	ramData vbatr_expect;
	ramData coef_p_hvic;
	ramData p_th_hvic;
	ramData vbath_expect;
	uInt8	cm_clamp;
	uInt8	autoRegister;
	ramData cm_dbi;
	ramData bat_dbi;
	ramData bat_hyst;
	ramData bat_settle;
	uInt8 batselmap;
	ramData coef_p_offld;
	uInt8 offld;
	ramData p_th_offld;
	ramData vbatl_expect;
	uInt8 irqen1;
	uInt8 irqen2;
	uInt8 irqen3;
	uInt8 irqen4;
} Si324x_General_Cfg;
/*
** Defines structure for configuring pcm
*/
typedef struct {
	uInt8 pcmFormat;
	uInt8 widebandEn;
	uInt8 pcm_tri;
	uInt8 tx_edge;
	uInt8 tx_both;
	uInt8 tx_hwy;
	uInt8 rx_hwy;
} Si324x_PCM_Cfg;

/*
** Defines structure for configuring pulse metering
*/
typedef struct {
	ramData pm_amp_thresh;
	uInt8 pmFreq;
	uInt8 pmRampRate;
} Si324x_PulseMeter_Cfg;

/*
** Defines structure for configuring FSK generation
*/
typedef struct {
	ramData fsk01;
	ramData fsk10;
	ramData fskamp0;
	ramData fskamp1;
	ramData fskfreq0;
	ramData fskfreq1;
	uInt8 eightBit;
	uInt8 fskdepth;
} Si324x_FSK_Cfg;

/*
** Defines structure for configuring dtmf decode
*/
typedef struct {
	ramData dtmfdtf_b0_1;
	ramData dtmfdtf_b1_1;
	ramData dtmfdtf_b2_1;
	ramData dtmfdtf_a1_1;
	ramData dtmfdtf_a2_1;
	ramData dtmfdtf_b0_2;
	ramData dtmfdtf_b1_2;
	ramData dtmfdtf_b2_2;
	ramData dtmfdtf_a1_2;
	ramData dtmfdtf_a2_2;
	ramData dtmfdtf_b0_3;
	ramData dtmfdtf_b1_3;
	ramData dtmfdtf_b2_3;
	ramData dtmfdtf_a1_3;
	ramData dtmfdtf_a2_3;
} Si324x_DTMFDec_Cfg;

/*
** Defines structure for configuring impedence synthesis
*/
typedef struct {
	ramData zsynth_b0;
	ramData zsynth_b1;
	ramData zsynth_b2;
	ramData zsynth_a1;
	ramData zsynth_a2;
	uInt8 ra;
} Si324x_Zsynth_Cfg;

/*
** Defines structure for configuring hybrid
*/
typedef struct {
	ramData ecfir_c2;
	ramData ecfir_c3;
	ramData ecfir_c4;
	ramData ecfir_c5;
	ramData ecfir_c6;
	ramData ecfir_c7;
	ramData ecfir_c8;
	ramData ecfir_c9;
	ramData eciir_b0;
	ramData eciir_b1;
	ramData eciir_a1;
	ramData eciir_a2;
} Si324x_hybrid_Cfg;

/*
** Defines structure for configuring GCI CI bits
*/
typedef struct {
	uInt8 gci_ci;
} Si324x_CI_Cfg;

/*
** Defines structure for configuring modem tone detect
*/
typedef struct {
	ramData rxmodpwr;
	ramData rxpwr;
	ramData modem_gain;
	ramData txmodpwr;
	ramData txpwr;
} Si324x_modemDet_Cfg;

/*
** Defines structure for configuring audio eq
*/
typedef struct {
	ramData txaceq_c0;
	ramData txaceq_c1;
	ramData txaceq_c2;
	ramData txaceq_c3;

	ramData rxaceq_c0;
	ramData rxaceq_c1;
	ramData rxaceq_c2;
	ramData rxaceq_c3;
} Si324x_audioEQ_Cfg;

/*
** Defines structure for configuring audio gain
*/
typedef struct {
	ramData acgain;
	uInt8 mute;
} Si324x_audioGain_Cfg;

typedef struct {
	Si324x_audioEQ_Cfg audioEQ;
	Si324x_hybrid_Cfg hybrid;
    Si324x_Zsynth_Cfg zsynth;
	Si324x_audioGain_Cfg txgain;
	Si324x_audioGain_Cfg rxgain;
	ramData rxachpf_b0_1;
	ramData  rxachpf_b1_1;
	ramData  rxachpf_a1_1;
} Si324x_Impedance_Cfg;

/*
** Defines structure for configuring tone generator
*/
typedef struct {
	Oscillator_Cfg osc1;
	Oscillator_Cfg osc2;
	uInt8 omode;
} Si324x_Tone_Cfg; 

/*
** Defines structure for configuring ring generator
*/
typedef struct {
	ramData freq;
	ramData amp;
	ramData offset;
	ramData phas;
	ramData rtper;
	uInt8 talo;
	uInt8 tahi;
	uInt8 tilo;
	uInt8 tihi;
	uInt8 ringcon;
	ramData rtcount;
	ramData adap_ring_min_i;
	ramData slope_ring;
	ramData rrd_delay;
	ramData rrd_delay2;
	ramData counter_vtr_val;
	ramData rtacdb;
    ramData rtacth;
	ramData rtdcdb;
	ramData rtdcth;
	ramData vov_ring_bat;
	ramData vov_ring_gnd;
	ramData vcm_ring;
} Si324x_Ring_Cfg;



/*
** This defines names for the interrupts in the ProSLIC
*/
typedef enum {
OSC1_T1_SI324X,
OSC1_T2_SI324X,
OSC2_T1_SI324X,
OSC2_T2_SI324X,
RING_T1_SI324X,
RING_T2_SI324X,
FSKBUF_AVAIL_SI324X,
VBAT_SI324X,
RING_TRIP_SI324X,
LOOP_STAT_SI324X,
LONG_STAT_SI324X,
VOC_TRACK_SI324X,
DTMF_SI324X,
INDIRECT_SI324X,
TXMDM_SI324X,
RXMDM_SI324X,
PQ1_SI324X,
PQ2_SI324X,
PQ3_SI324X,
PQ4_SI324X,
PQ5_SI324X,
PQ6_SI324X,
RING_FAIL_SI324X,
CM_BAL_SI324X,
USER_IRQ0_SI324X,
USER_IRQ1_SI324X,
USER_IRQ2_SI324X,
USER_IRQ3_SI324X,
USER_IRQ4_SI324X,
USER_IRQ5_SI324X,
USER_IRQ6_SI324X,
USER_IRQ7_SI324X
}Si324xProslicInt;

#endif
