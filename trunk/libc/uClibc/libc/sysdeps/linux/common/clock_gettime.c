/*
 * clock_gettime() for uClibc
 *
 * Copyright (C) 2003 by Justus Pendleton <uc@ryoohki.net>
 * Copyright (C) 2005 by Peter Kjellerstedt <pkj@axis.com>
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
#include "syscalls.h"
#include <time.h>
#include <sys/time.h>

#ifdef __NR_clock_gettime
_syscall2(int, clock_gettime, clockid_t, clock_id, struct timespec*, tp);
#else
int clock_gettime(clockid_t clock_id, struct timespec* tp)
{
	struct timeval tv;
	int retval = -1;

	switch (clock_id) {
		case CLOCK_REALTIME:
			retval = gettimeofday(&tv, NULL);
			if (retval == 0) {
				TIMEVAL_TO_TIMESPEC(&tv, tp);
			}
			break;

		default:
			errno = EINVAL;
			break;
	}

	return retval;
}
#endif
