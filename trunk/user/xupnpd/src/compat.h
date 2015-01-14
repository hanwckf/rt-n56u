/* 
 * Copyright (C) 2011-2015 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
*/

#ifndef __COMPAT_H
#define __COMPAT_H

#ifdef __FreeBSD__
#define O_LARGEFILE     0
#define lseek64         lseek
typedef off_t           off64_t;
#endif /*  __FreeBSD__  */

#ifdef _UNDER_WIN
#include <cygwin/types.h>
#define O_LARGEFILE	0
#define off64_t	_off64_t
#define lseek64	lseek
#endif /*  _UNDER_WIN  */

#endif /* __COMPAT_H */
