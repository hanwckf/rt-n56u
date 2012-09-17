/*
** $Id: pbx.h,v 1.1 2008-12-31 02:34:53 qwert Exp $
**
** pbx.h
** PBX demo header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for the PBX demo.
**
** Dependancies:
** datatypes.h, ProSLIC.h, system.h
**
*/
#ifndef PBX_H
#define PBX_H

#include "proslic_api/ralink/timer/timer.h"

#define PRINTF_IS_OK TRUE

//#ifdef SI324X
//	#define NUMBER_OF_CHAN 4
//	#define NUMBER_OF_PROSLIC (NUMBER_OF_CHAN/4)
//#endif

#ifdef SI321X
	#define NUMBER_OF_CHAN 1
	#define NUMBER_OF_PROSLIC (NUMBER_OF_CHAN)
#endif

typedef struct chanStatus chanState; //forward declaration

typedef void (*procState) (chanState *pState, ProslicInt eInput);


/*
** structure to hold state information for pbx demo
*/
struct chanStatus { 
	proslicChanType *ProObj;
	timeStamp onHookTime;
	timeStamp offHookTime;
	procState currentState;
	uInt16 digitCount;
	uInt8 digits[20];
	uInt8 ringCount;
	uInt16 connectionWith;
	uInt16 powerAlarmCount;
	pulseDialType pulseDialData;
	BOOLEAN eventEnable;
} ; 


/*
** Function: PBX_Demo
**
** Description: 
** PBX State Machine
**
** Input Parameters: 
** 
** Return:
** none
*/
uInt16 PBX_Demo ();

#endif
/*
** $Log: pbx.h,v $
** Revision 1.1  2008-12-31 02:34:53  qwert
** Add si3210 driver
**
** Revision 1.9  2007/05/30 22:20:28  lajordan
** added wideband feature
**
** Revision 1.8  2007/02/16 23:54:25  lajordan
** no message
**
** Revision 1.7  2007/02/15 23:33:08  lajordan
** no message
**
** Revision 1.6  2007/02/01 21:29:58  lajordan
** updated for broadcast init
**
** Revision 1.5  2006/07/18 20:39:15  lajordan
** no message
**
** Revision 1.3  2006/07/18 20:31:58  lajordan
** no message
**
** Revision 1.2  2006/07/14 21:57:38  lajordan
** pulse dial
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.2  2006/06/21 22:42:26  laj
** new api style
**
** Revision 1.1  2005/11/14 17:43:21  laj
** pbx demo files added
**
*/