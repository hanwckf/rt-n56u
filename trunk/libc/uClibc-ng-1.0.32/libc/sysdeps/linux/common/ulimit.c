/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#include <stdarg.h>
#include <unistd.h>
#include <ulimit.h>
#include <sys/resource.h>


long int ulimit(int cmd, ...)
{
	va_list va;
	struct rlimit limit;
	long int result = -1;
	va_start (va, cmd);
	switch (cmd) {
		/* Get limit on file size.  */
		case UL_GETFSIZE:
			if (getrlimit(RLIMIT_FSIZE, &limit) == 0)
				result =  limit.rlim_cur / 512; /* bytes to 512 byte blocksize */
			break;
		/* Set limit on file size.  */
		case UL_SETFSIZE:
			result = va_arg (va, long int);
			if ((rlim_t) result > RLIM_INFINITY / 512) {
				limit.rlim_cur = RLIM_INFINITY;
				limit.rlim_max = RLIM_INFINITY;
			} else {
				limit.rlim_cur = result * 512;
				limit.rlim_max = result * 512;
			}
			result = setrlimit(RLIMIT_FSIZE, &limit);
			break;
		case __UL_GETOPENMAX:
			result = sysconf(_SC_OPEN_MAX);
			break;
		default:
			__set_errno(EINVAL);
	}
	va_end (va);
	return result;
}
