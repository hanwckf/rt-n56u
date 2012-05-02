/*
** $Id: proslic_timer_intf_linux.c,v 1.1 2008-12-31 02:39:58 qwert Exp $
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
#include "../../proslic_datatypes.h"
#include "../../proslic_timer_intf.h"
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

static void sleep( uInt32 wait ) //inaccurate sleep to gauge PC speed in timerInit
{
	uInt32 goal;
	goal = wait + clock();
	while( goal > (uInt32)clock() );
}

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
	pTimerObj->ticksPerSecond=384000000;
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

