/* vi: set sw=4 ts=4: */
/*
 * utime() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <utime.h>


#ifdef __NR_utime
_syscall2(int, utime, const char *, file, const struct utimbuf *, times)
#else
#include <stdlib.h>
#include <sys/time.h>


int utime(const char *file, const struct utimbuf *times)
{
	struct timeval timevals[2];

	if (times != NULL) {
		timevals[0].tv_usec = 0L;
		timevals[1].tv_usec = 0L;
		timevals[0].tv_sec = (long int) times->actime;
		timevals[1].tv_sec = (long int) times->modtime;
	}
	return utimes(file, times ? timevals : NULL);
}
#endif
link_warning(utime, "the use of OBSOLESCENT `utime' is discouraged, use `utimes'")
libc_hidden_def(utime)
