/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <string.h>
#include "_syserrmsg.h"


char *strerror(int errnum)
{
    static char buf[_STRERROR_BUFSIZE];

	__xpg_strerror_r(errnum, buf, sizeof(buf));

	return buf;
}
libc_hidden_def(strerror)
