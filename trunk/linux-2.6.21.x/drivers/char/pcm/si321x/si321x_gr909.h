/*
** Copyright © 2007 by Silicon Laboratories
**
** $Id: si321x_gr909.h,v 1.1 2008-12-31 02:34:53 qwert Exp $
**
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
**
*/
#ifndef GR909_H
#define GR909_H

#define POLL_RATE 10 /*ms*/ /*****************set this for you system*********/

typedef struct{
	proslicChanType *pProslic;
	unsigned char CalFlag;
	int32 tgAC; //mV
	int32 rgAC;
	int32 trAC;
	int32 tgDC;
	int32 rgDC;
	int32 trDC;
	int32 tg; //ohms
	int32 rg;
	int32 tr;
	int32 rohTrue;
	int32 renValue;	//mREN
}ProSLICGR909Type;


/* NOTE: Since we're dealing in fixed point 32 bit math, please be careful in
 * adjusting the number of samples collected since this could lead to an
 * overflow condition.
 */


#define SI321X_LT_IQ12_SCALE 319 /* In mA */
#define SI321X_LT_IQ56_SCALE 316 /* In mA */
#define SI321X_LT_ISCALE 1250 /* In mA */


typedef enum {
	NOTHING,
    RTG,
    RTR,
    RRG,
} SI321X_FAULT_PATH;

/* The structure below is used for calibrating the REN load test */
typedef struct
{
	int32 highRenSlope;
	int32 lowRenSlope;
	int32 highRenOffs;
	int32 lowRenOffs;
	int32 renTrans;
	int32 extraLowRenOffset;
	int32 extraLowRenSlope;
} si321x_rencal_t;

/* Generic container for any calibration or other data for device hardware
 * specifc parameters.
*/
typedef struct
{
	si321x_rencal_t ren;
} si321x_lt_dev_info;


/* The structure below is used for preserving any registers on how they were
 * prior to running the test.
*/
typedef struct
{
	uInt16 ring_osc_coef, ring_x,ring_y,loop_close_thr;
	uInt8 ring_osc_ctl, off_hook_v,
		bias_squelch,bat_feed,vbath,vbatl,common_v,onhookv,
		loop_i_limit,audio_gain,int0,int1,int2,reg65;
} si321x_restore_settings;

/* Generic test structure for handling the test state machines */
typedef struct {
	uInt16 stage;  //initialize to zero
	uInt16 sampleIterations;
	uInt16 waitIterations;
	uInt8 channelNumber;
	si321x_restore_settings old_settings; /* What was the state of the registers prior to running the test? */
} si321x_test_state;

/* FEMF/HAZ volt test structure */
#define SI321X_MAX_FEMF_SAMPLES 256
typedef struct {
	int32 vtip[SI321X_MAX_FEMF_SAMPLES];  /* All measurement values are 8 bit in size multiplied by a constant of 9 bits and are in terms of mV*/
	int32 vring[SI321X_MAX_FEMF_SAMPLES];
	int32 vloop[SI321X_MAX_FEMF_SAMPLES];
	si321x_test_state State;
	int32 samples;
} SI321x_VOLTAGES_STATE;

typedef struct {
	si321x_test_state State;
	SI321X_FAULT_PATH whichPath; //fault path
	int32 rVal; //result
	int32 	dcRingV;
	int32		dcTipV;
	int32		IQ1val,IQ2val,IQ5val,IQ6val;
	uInt8 lfState;
	int32 i;
} SI321x_LOWRMEAS_STATE;

#define SI321X_LT_RESF_INIT_RES 100000000UL
#define SI321X_LT_RESF_SAMPLES  8

#define SI321X_LT_RESF_MINRESF	10000


typedef struct {
	si321x_test_state State;
	SI321x_LOWRMEAS_STATE lowR;
	int32			dcRingV[2];
	int32			dcTipV[2];
	uInt32 iLoop;
	uInt8 voc;       
	uInt32 iRing;
	uInt32 iTip;
	uInt8 vCm;
	uInt32 IQ1val,IQ2val,IQ5val,IQ6val;
	int32 max;
} SI321x_RMEAS_STATE;


#define SI321X_LT_RESF_NORSH_SAMPLES 16
#define SI321X_LT_RESF_NORSH_DEF_RES 100000000 /* in ohms   Default to 100meg */
typedef struct {
	si321x_test_state DiffState;
	si321x_test_state LongState;
	int32 vTip;
	int32 vRing;
	uInt32 iLoop;
	uInt8 voc;       
	uInt32 iRing;
	uInt32 iTip;
	uInt8 vCm;
	int32 diff;
	int32	IQ1val,IQ2val,IQ5val,IQ6val;
} SI321x_RMEASCOARSE_STATE;

#define SI321X_LT_MILREN 100
#define SI321X_LT_REN_SAMPLES 16
#define SI321X_LT_REN_RST 200000
#define N_REN_SAMPLES (500/POLL_RATE)
typedef struct {
	si321x_test_state State;
	int32 v1;
	uInt16 pollCount;
	int32 sumIq2;
	si321x_rencal_t *renCal;
	uInt8 RenCalFlag;
	int32 iValues[N_REN_SAMPLES];
} SI321x_REN_STATE;

#define SI321X_LT_ROH_SAMPLES 16
#define SI321X_LT_ROH_PERCENT_DIFF 25
typedef struct {
	si321x_test_state State;
	int32 Rv1;
	int32 Rv2;
	int32	vTip;
	int32   vRing;
	int32   IQ1val,IQ2val,IQ5val,IQ6val;
} SI321x_ROH_STATE;

typedef struct {
	int bomOption;
	int rshValue;
} SI321x_GR909_Config;

#define SI321X_LV_BOM 0
#define SI321X_HV_BOM 1

#define SI321x_ROH_MAXRES 1000000000

#define SI321x_FAIL_RESFAULT_INC     0x0080
#define SI321x_FAIL_ROH_DETECTED     0x0100

#define DEFAULT_909TYPE_INIT {0,0,0,0,0,0,0,0,0,0,0,0,0,0}

/* Returns 0 for not done, 1 for done */
int32 ProSLIC_GR909_INIT(ProSLICGR909Type *pProSLIC909,SI321x_GR909_Config *config);
int32 ProSLIC_GR909_FOREIGN_VOLTAGES(ProSLICGR909Type *pProSLIC909,SI321x_VOLTAGES_STATE *pState);
int32 ProSLIC_GR909_HAZARD_VOLTAGES(ProSLICGR909Type *pProSLIC909,SI321x_VOLTAGES_STATE *pState);
int32 ProSLIC_GR909_RESISTIVE_FAULT_NOSH(ProSLICGR909Type *pProSLIC909,SI321x_RMEASCOARSE_STATE *pState);
int32 ProSLIC_GR909_RESISTIVE_FAULT(ProSLICGR909Type *pProSLIC909,SI321x_RMEAS_STATE *pState);
int32 ProSLIC_GR909_RECEIVER_OFFHOOK(ProSLICGR909Type *pProSLIC909,SI321x_ROH_STATE *pState);
int32 ProSLIC_GR909_REN(ProSLICGR909Type *pProSLIC909,SI321x_REN_STATE *pState);
int32 ProSLIC_GR909_ABORT (ProSLICGR909Type *pProSLIC909,si321x_test_state *pState);


#endif
//$Log: si321x_gr909.h,v $
//Revision 1.1  2008-12-31 02:34:53  qwert
//Add si3210 driver
//
//Revision 1.9  2008/01/21 21:13:52  lajordan
//rename
//
//Revision 1.7  2007/06/04 18:31:45  lajordan
//no message
//
//Revision 1.6  2007/05/29 19:49:58  lajordan
//changed to current sense ren test
//
//Revision 1.5  2007/05/24 00:22:21  lajordan
//no message
//
//Revision 1.4  2007/05/23 23:38:39  lajordan
//no message
//
//Revision 1.3  2007/05/22 21:57:06  lajordan
//updated for HV BOM
//
//Revision 1.2  2007/04/03 20:26:19  lajordan
//improved ren test
//
//Revision 1.1  2007/04/02 19:27:09  lajordan
//no message
//
//Revision 1.8  2007/02/01 17:40:22  lajordan
//cosmetic
//
//Revision 1.7  2007/02/01 04:08:39  lajordan
//updated to polling
//
//Revision 1.6  2006/11/29 19:59:48  lajordan
//removed unused code
//
//Revision 1.5  2006/08/02 18:48:27  lajordan
//moved ui out of gr909.c
//
//Revision 1.4  2006/08/02 17:25:55  lajordan
//added log and header fields
//