/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2003 by Erik Andersen <andersen@codepoet.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Library General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library General Public License
 * for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

#define _GNU_SOURCE
#define _LARGEFILE64_SOURCE
#include <features.h>
#undef __OPTIMIZE__
/* We absolutely do _NOT_ want interfaces silently
 *  *  * renamed under us or very bad things will happen... */
#ifdef __USE_FILE_OFFSET64
# undef __USE_FILE_OFFSET64
#endif


#ifdef __NR_ulimit

#include <sys/types.h>
#include <sys/syscall.h>

_syscall2(long, ulimit, int, cmd, int, arg);

#else

#include <stdarg.h>
#include <unistd.h>
#include <ulimit.h>
#include <errno.h>
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
#endif

