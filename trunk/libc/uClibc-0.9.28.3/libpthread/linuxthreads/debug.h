/****************************************************************************
**
** NAME:
** debug.h
**
** DESCRIPTION:
** This header file defines the debug macros used in pthreads. To turn
** debugging on, add -DDEBUG_PT to CFLAGS. It was added to the original
** distribution of linuxthreads.
**
** This program is free software; you can redistribute it and/or        
** modify it under the terms of the GNU Library General Public License  
** as published by the Free Software Foundation; either version 2       
** of the License, or (at your option) any later version.               
**                                                                      
** This program is distributed in the hope that it will be useful,      
** but WITHOUT ANY WARRANTY; without even the implied warranty of       
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        
** GNU Library General Public License for more details.                 
**
****************************************************************************/

#ifndef _PT_DEBUG_H
#define _PT_DEBUG_H

#include <features.h>

#ifdef __DODEBUG_PT__
# define DEBUG_PT
#endif

/* include asserts for now */
#define DO_ASSERT

/* define the PDEBUG macro here */
#undef PDEBUG
#ifdef DEBUG_PT
#  define PDEBUG(fmt, args...) __pthread_message("%s: " fmt, __FUNCTION__, ## args)
#else
#  define PDEBUG(fmt, args...) /* debug switched off */
#endif

/* nothing; placeholder to disable a PDEBUG message but don't delete it */
#undef PDEBUGG
#define PDEBUGG(fmt, args...) 

/* Define ASSERT to stop/warn. Should be void in production code */
#undef ASSERT
#ifdef DO_ASSERT
#  define ASSERT(x) if (!(x)) fprintf(stderr, "pt: assertion failed in %s:%i.\n",\
                    __FILE__, __LINE__)
#else
#  define ASSERT(x)
#endif

#endif /* _PT_DEBUG_H */
