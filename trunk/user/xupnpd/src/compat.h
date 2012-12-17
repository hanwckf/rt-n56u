/* 
 * Copyright (C) 2011-2012 Anton Burdinuk
 * clark15b@gmail.com
 * https://tsdemuxer.googlecode.com/svn/trunk/xupnpd
*/

#ifndef __COMPAT_H
#define __COMPAT_H

#ifdef __FreeBSD__
#define O_LARGEFILE     0
#define lseek64         lseek
typedef off_t           off64_t;
#endif

#endif
