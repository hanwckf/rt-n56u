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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <features.h>
#ifdef __USE_GNU

#include <sched.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include <alloca.h>

#if defined __NR_sched_setaffinity

#define __NR___syscall_sched_setaffinity __NR_sched_setaffinity
static __inline__ _syscall3(int, __syscall_sched_setaffinity, __kernel_pid_t, pid,
			size_t, cpusetsize, cpu_set_t *, cpuset)

static size_t __kernel_cpumask_size;

int sched_setaffinity(pid_t pid, size_t cpusetsize, const cpu_set_t *cpuset)
{
	size_t cnt;
	if (unlikely (__kernel_cpumask_size == 0)) {
		INTERNAL_SYSCALL_DECL (err);
		int res;
		size_t psize = 128;
		void *p = alloca (psize);

		while (res = INTERNAL_SYSCALL (sched_getaffinity, err, 3, getpid (),
					       psize, p),
		       INTERNAL_SYSCALL_ERROR_P (res, err)
		       && INTERNAL_SYSCALL_ERRNO (res, err) == EINVAL)
			p = extend_alloca (p, psize, 2 * psize);

		if (res == 0 || INTERNAL_SYSCALL_ERROR_P (res, err)) {
			__set_errno (INTERNAL_SYSCALL_ERRNO (res, err));
			return -1;
		}

		__kernel_cpumask_size = res;
	}

	/* We now know the size of the kernel cpumask_t.  Make sure the user
	   does not request to set a bit beyond that.  */
	for (cnt = __kernel_cpumask_size; cnt < cpusetsize; ++cnt)
		if (((char *) cpuset)[cnt] != '\0') {
			/* Found a nonzero byte.  This means the user request cannot be
			   fulfilled.  */
			__set_errno (EINVAL);
			return -1;
		}

	return INLINE_SYSCALL (sched_setaffinity, 3, pid, cpusetsize, cpuset);
}
#endif
#endif /* __USE_GNU */
