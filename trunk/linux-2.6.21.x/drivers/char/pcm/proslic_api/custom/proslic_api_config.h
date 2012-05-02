/*
** Copyright ? 2007 by Silicon Laboratories
**
** $Id: proslic_api_config.h,v 1.2 2010-12-02 09:43:38 qwert Exp $
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

/*
** ProSLIC Driver Selection
**
** Define device driver to be compliled
*/
/* #define SI321X */
/* #define SI322X */
/* #define SI3217X */
/* #define SI3226X */
/* #define SI324X */


#define DISABLE_DTMF_SETUP 
/*#define DISABLE_FSK_SETUP  */
/*#define DISABLE_TONE_SETUP  */
/*#define DISABLE_RING_SETUP  */
/*#define DISABLE_AUDIOGAIN_SETUP  */
/*#define DISABLE_DCFEED_SETUP  */
/*#define DISABLE_GPIO_SETUP  */
/*#define DISABLE_PCM_SETUP  */
#define ENABLE_DEBUG 
/*#define DISABLE_CI_SETUP  */
/*#define DISABLE_ZSYNTH_SETUP  */
/*#define DISABLE_MALLOC */
/*#define GCI_MODE  */

//#include "stdio.h"
/*#if (PRINT_TO_STRING) */
/*extern char outputBuffer[]; */
/*#define LOGPRINT(...) sprintf(&(outputBuffer[strlen(outputBuffer)]),__VA_ARGS__) */
/*#else */
#define LOGPRINT(fmt, args...) { }
/*#endif */

/*
** PSTN Detection Options
*/
/* #define PSTN_DET_ENABLE  */                /* Define to include Differential PSTN detection code */

#ifdef PSTN_DET_ENABLE
#define PSTN_DET_OPEN_FEMF_SETTLE   1500     /* OPEN foreign voltage measurement settle time */
#define PSTN_DET_DIFF_SAMPLES       4        /* Number of I/V samples averaged [1 to 16] */
#define PSTN_DET_MIN_ILOOP          700      /* Minimum acceptable loop current */
#define PSTN_DET_MAX_FEMF           10000    /* Maximum OPEN state foreign voltage */
#define PSTN_DET_POLL_RATE          10       /* Rate of re-entrant code in ms */
#define PSTN_DET_DIFF_IV1_SETTLE    1000     /* Settle time before first I/V measurment in ms */
#define PSTN_DET_DIFF_IV2_SETTLE    1000     /* Settle time before first I/V measurment in ms */
#endif

#ifndef PROSLIC_API_CFG
#define PROSLIC_API_CFG



#endif
