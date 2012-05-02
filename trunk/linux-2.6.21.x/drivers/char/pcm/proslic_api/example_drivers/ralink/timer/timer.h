/*
** $Id: timer.h,v 1.1 2010-07-30 07:55:37 qwert Exp $
** 
** This file is system specific and should be edited for your hardware platform
**
** This file is used by proslic_timer_intf.h and proslic_spiGci.h
*/
#ifndef TIME_TYPE_H
#define TIME_TYPE_H



/*
** System timer interface structure 
*/
typedef struct{
	_int64 ticksPerSecond;
} systemTimer_S;

/*
** System time stamp
*/
typedef struct{
	_int64 time;
} timeStamp;

/*
** Function: SYSTEM_TimerInit
**
** Description: 
** Initializes the timer used in the delay and time measurement functions
** by doing a long inaccurate sleep and counting the ticks
**
** Input Parameters: 
**
** Return:
** none
*/
void TimerInit (systemTimer_S *pTimerObj);
/*
** Function: DelayWrapper
**
** Description: 
** Waits the specified number of ms
**
** Input Parameters: 
** hTimer: timer pointer
** timeInMs: time in ms to wait
**
** Return:
** none
*/
int time_DelayWrapper (void *hTimer, int timeInMs);


/*
** Function: TimeElapsedWrapper
**
** Description: 
** Calculates number of ms that have elapsed
**
** Input Parameters: 
** hTImer: pointer to timer object
** startTime: timer value when function last called
** timeInMs: pointer to time elapsed
**
** Return:
** timeInMs: time elapsed since start time
*/
int time_TimeElapsedWrapper (void *hTimer, void *startTime, int *timeInMs);

int time_GetTimeWrapper (void *hTimer, void *time);
#endif
/*
** $Log: timer.h,v $
** Revision 1.1  2010-07-30 07:55:37  qwert
** Add si3227 driver
**
** Revision 1.2  2007/02/21 16:55:18  lajordan
** moved function prototypes here
**
** Revision 1.1  2007/02/16 23:55:07  lajordan
** no message
**
** Revision 1.2  2007/02/15 23:33:25  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
*/