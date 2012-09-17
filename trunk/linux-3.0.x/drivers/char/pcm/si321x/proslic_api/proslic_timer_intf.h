/*
** $Id: proslic_timer_intf.h,v 1.1 2008-12-31 02:39:58 qwert Exp $
**
** system.h
** System specific functions header file
**
** Author(s): 
** laj
**
** Distributed by: 
** Silicon Laboratories, Inc
**
** File Description:
** This is the header file for the system specific functions like timer functions.
**
** Dependancies:
** proslic_datatypes.h
** definition of timeStamp structure
**
*/


#ifndef TIMER_INTF_H
#define TIMER_INTF_H

/*
** System time delay function pointer 
*/
typedef int (*system_delay_fptr) (void *hTimer, int timeInMs);

/*
** System time elapsed function pointer 
*/
typedef int (*system_timeElapsed_fptr) (void *hTimer, void *startTime, int *timeInMs);

typedef int (*system_getTime_fptr) (void *hTimer, void *time);



#endif
/*
** $Log: proslic_timer_intf.h,v $
** Revision 1.1  2008-12-31 02:39:58  qwert
** Add si3210 driver
**
** Revision 1.5  2008/01/21 21:19:03  lajordan
** renaming to lower case
**
** Revision 1.3  2007/02/21 16:55:06  lajordan
** moved function prototypes out
**
** Revision 1.2  2007/02/16 23:54:56  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/13 20:26:08  lajordan
** no message
**
** Revision 1.1  2006/07/07 21:39:22  lajordan
** no message
**
** Revision 1.1.1.1  2006/07/06 22:06:23  lajordan
** no message
**
** Revision 1.1  2006/06/21 22:42:26  laj
** new api style
**
**
*/
