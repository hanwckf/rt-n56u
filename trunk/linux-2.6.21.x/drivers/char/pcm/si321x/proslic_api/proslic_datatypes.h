/*
** Copyright ? 2007 by Silicon Laboratories
**
** $Id: proslic_datatypes.h,v 1.1 2008-12-31 02:39:58 qwert Exp $
**
** datatypes.h
** ProSLIC datatypes file
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
** File Description:
** This is the header file that contains
** type definitions for the data types
** used in the demonstration code.
**
** Dependancies:
** 
**
*/
#ifndef DATATYPES_H
#define DATATYPES_H

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif
#define BOOLEAN int

#ifndef NULL
#define NULL ((void *) 0)
#endif

typedef char				int8;
typedef unsigned char		uInt8;
typedef uInt8				uChar;
typedef short int			int16;
typedef unsigned short int	uInt16;
typedef long				int32;
typedef unsigned long		uInt32;
typedef float				float32;
typedef double				float64;

 
#include <linux/types.h>
typedef long long _int64;
/*
** RAM data 
*/

#define ramData uInt32

#endif
/*
** $Log: proslic_datatypes.h,v $
** Revision 1.1  2008-12-31 02:39:58  qwert
** Add si3210 driver
**
** Revision 1.7  2008/01/21 21:19:03  lajordan
** renaming to lower case
**
** Revision 1.5  2007/10/25 16:12:17  lajordan
** fixed some warnings
**
** Revision 1.4  2007/05/31 19:13:25  lajordan
** added line
**
** Revision 1.3  2007/02/15 19:56:09  lajordan
** no message
**
** Revision 1.2  2007/02/14 17:54:53  lajordan
** added comments
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
** Revision 1.2  2005/11/07 23:21:47  laj
** took out extra spaces
**
*/

