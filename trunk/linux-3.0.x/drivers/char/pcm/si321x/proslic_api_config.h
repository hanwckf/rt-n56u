/*
** Copyright ?2007 by Silicon Laboratories
**
** $Id: proslic_api_config.h,v 1.1 2008-12-31 02:34:53 qwert Exp $
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

/*API configuration options*/
//#define DISABLE_FSK_SETUP 
//#define DISABLE_TONE_SETUP 
//#define DISABLE_RING_SETUP 
//#define DISABLE_AUDIOGAIN_SETUP 
//#define DISABLE_DCFEED_SETUP 
//#define DISABLE_PCM_SETUP 
#define DISABLE_PULSE_SETUP
#define ENABLE_DEBUG 
//#define DISABLE_ZSYNTH_SETUP 
//#define DISABLE_MALLOC

/*Debug output handling definition*/
#define PRINT_TO_STRING 0
#if (PRINT_TO_STRING)
extern char outputBuffer[];
#define LOGPRINT(...) sprintf(&(outputBuffer[strlen(outputBuffer)]),__VA_ARGS__)
#else
//#include "stdio.h"
#define LOGPRINT printk
#endif

/*si321x hardware specific settings
* If these are not defined the default
* values are loaded */
#define	INIT_PWR_ALARM_Q1Q2		0x7c0  		
#define	INIT_PWR_ALARM_Q3Q4		0x2600	
#define	INIT_PWR_ALARM_Q5Q6		0x1B80		
#define	INIT_THERM_LP_POLE_Q1Q2		0x08c	
#define	INIT_THERM_LP_POLE_Q3Q4		0x0100		
#define	INIT_THERM_LP_POLE_Q5Q6		0x010	
#define SI3215_TONE /*we want to support Si3215/16 tones*/
#define SI3210_TONE /*we want to support Si3210 tones*/
