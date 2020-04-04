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

#if defined __NR_sched_setaffinity && defined __USE_GNU
# include <sched.h>
# include <sys/types.h>
# include <string.h>
# include <unistd.h>
# include <alloca.h>
# define __NR___syscall_sched_setaffinity __NR_sched_setaffinity
static __always_inline _syscall3(int, __syscall_sched_setaffinity, __kernel_pid_t, pid,
				 size_t, cpusetsize, const cpu_set_t *, cpuset)

int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *cpuset)
{
	return __syscall_sched_setaffinity(pid, cpusetsize, cpuset);
}
#endif
