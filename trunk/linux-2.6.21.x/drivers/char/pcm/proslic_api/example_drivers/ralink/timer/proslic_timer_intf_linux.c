/*
** $Id: proslic_timer_intf_linux.c,v 1.3 2010-12-02 09:43:38 qwert Exp $
**
** system.c
** System specific functions implementation file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the implementation file for the system specific functions like timer functions.
**
** Dependancies:
** datatypes.h
**
*/
#include "si_voice_datatypes.h"
#include "si_voice_timer_intf.h"
#include <linux/time.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <asm/div64.h>
#include "timer.h"
#include "mipsregs.h"

/*
** First we define the local functions
*/
static _int64 readTSC () //read precision timer register from PC
{
	union {
		_int64 extralong;
		unsigned long longish[2];
	} t;
	unsigned long a,b;
 /*
	_asm {
	_emit 0x0f;
	_emit 0x31;
	mov a,eax;
	mov b,edx;
}*/

	a = 0;
	b = read_c0_count();
	if(b>(unsigned long)0x7FFFFFFF)
	{
		a = 1;
		b <<=1; 
	}
	t.longish[0]=a;t.longish[1]=b;
	//t.extralong = b;
	
	return t.extralong;
}
/*
static void sleep( uInt32 wait ) //inaccurate sleep to gauge PC speed in timerInit
{
	uInt32 goal;
	goal = wait + clock();
	while( goal > (uInt32)clock() );
}
*/
/*
** These are the global functions
*/

/*
** Function: SYSTEM_TimerInit
*/
void TimerInit (systemTimer_S *pTimerObj){
	_int64 time0, time1;
	/*
	sleep(1);
	time0= readTSC();
	sleep (1800);
	time1 = readTSC();
	*/
	pTimerObj->ticksPerSecond=400000000>>1;
	//((time1-time0)/1800000)*1000000;
}


/*
** Function: SYSTEM_Delay
*/
int time_DelayWrapper (void *hTimer, int timeInMs){
	//_int64 target = readTSC() + (((systemTimer_S *)hTimer)->ticksPerSecond * timeInMs ) /1000 ;
	//while (readTSC() < target) ;
	mdelay(timeInMs);
	return 0;
}


/*
** Function: SYSTEM_TimeElapsed
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs){
	_int64 diff = readTSC() - ((timeStamp *)startTime)->time;
	*timeInMs = (int)((int)diff / ((int)((systemTimer_S *)hTimer)->ticksPerSecond/1000));
	//startTime->time = readTSC();
	return 0;
}

/*
** Function: SYSTEM_GetTime
*/
int time_GetTimeWrapper (void *hTimer, void *time){
	((timeStamp *)time)->time = readTSC();
	return 0;
}

/*
** $Log: proslic_timer_intf_linux.c,v $
** Revision 1.3  2010-12-02 09:43:38  qwert
** For 2.6.36.x compatabile
**
** Revision 1.2  2010-12-01 07:40:16  qwert
** Add SPI_CS1 high low active item for Vitesse and slic
**
** Revision 1.1  2010-07-30 07:55:37  qwert
** Add si3227 driver
**
** Revision 1.5  2008/07/24 21:06:16  lajordan
** no message
**
** Revision 1.4  2007/03/22 18:53:43  lajordan
** fixed warningg
**
** Revision 1.3  2007/02/26 16:46:16  lajordan
** cleaned up some warnings
**
** Revision 1.2  2007/02/16 23:55:07  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
** Revision 1.1  2006/07/07 21:38:56  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.1  2006/06/29 19:17:21  laj
** no message
**
** Revision 1.1  2006/06/21 22:42:26  laj
** new api style
**
**
*/
