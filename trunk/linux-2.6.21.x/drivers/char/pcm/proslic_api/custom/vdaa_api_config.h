/*
** Copyright ?2008-2010 by Silicon Laboratories
**
** $Id: vdaa_api_config.h,v 1.2 2010-12-02 09:43:38 qwert Exp $
**
** vdaa_api_config.h
** VoiceDAA header config file
**
** Author(s): 
** naqamar, laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** This file contains proprietary information.	 
** No dissemination allowed without prior written permission from
** Silicon Laboratories, Inc.
**
** File Description:
** This file is used 
** in the VoiceDAA demonstration code. 
**
**
*/

#ifndef VDAA_API_CFG_H
#define VDAA_API_CFG_H

/* #define DISABLE_MALLOC */
/* #define DISABLE_VDAA_RING_DETECT_SETUP */
/* #define DISABLE_VDAA_AUDIO_GAIN_SETUP */
/* #define DISABLE_VDAA_PCM_SETUP */
/* #define DISABLE_VDAA_COUNTRY_SETUP */
/* #define DISABLE_VDAA_HYBRID_SETUP */
#define DISABLE_VDAA_LOOPBACK_SETUP
#define DISABLE_VDAA_IMPEDANCE_SETUP

#ifndef ENABLE_DEBUG
#define ENABLE_DEBUG
#endif
//#include "stdio.h"
#define LOGPRINT(fmt, args...) { }

#endif