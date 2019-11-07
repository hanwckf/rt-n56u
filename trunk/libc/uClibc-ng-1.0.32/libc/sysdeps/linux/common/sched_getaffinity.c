/* Copyright (C) 2002, 2003, 2004, 2005, 2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sys/syscall.h>

#if defined __NR_sched_getaffinity && defined __USE_GNU
# include <sched.h>
# include <string.h>
# include <sys/types.h>
# include <sys/param.h>
# define __NR___syscall_sched_getaffinity __NR_sched_getaffinity
static __always_inline _syscall3(int, __syscall_sched_getaffinity, __kernel_pid_t, pid,
				 size_t, cpusetsize, cpu_set_t *, cpuset)

int sched_getaffinity(pid_t pid, size_t cpusetsize, cpu_set_t *cpuset)
{
	int res = (__syscall_sched_getaffinity(pid, MIN(INT_MAX, cpusetsize),
					       cpuset));

	if (res != -1) {
		/* Clean the rest of the memory the kernel didn't do.  */
		memset ((char *) cpuset + res, '\0', cpusetsize - res);

		res = 0;
	}
	return res;
}
#endif
