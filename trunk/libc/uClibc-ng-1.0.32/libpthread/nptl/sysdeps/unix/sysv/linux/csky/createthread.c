/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

/* Value passed to 'clone' for initialization of the thread register.  */
#define TLS_VALUE (pd + 1)

/* Get the real implementation.	 */
#include <sysdeps/pthread/createthread.c>
