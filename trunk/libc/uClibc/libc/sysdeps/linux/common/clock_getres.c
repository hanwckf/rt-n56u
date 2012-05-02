/*
 * clock_getres() for uClibc
 *
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
#include <unistd.h>

#ifdef __NR_clock_getres
_syscall2(int, clock_getres, clockid_t, clock_id, struct timespec*, res);
#else
int clock_getres(clockid_t clock_id, struct timespec* res)
{
	long clk_tck;
	int retval = -1;

	switch (clock_id) {
		case CLOCK_REALTIME:
			if ((clk_tck = sysconf(_SC_CLK_TCK)) < 0)
				clk_tck = 100;
			res->tv_sec = 0;
			res->tv_nsec = 1000000000 / clk_tck;
			retval = 0;
			break;

		default:
			errno = EINVAL;
			break;
	}

	return retval;
}
#endif
