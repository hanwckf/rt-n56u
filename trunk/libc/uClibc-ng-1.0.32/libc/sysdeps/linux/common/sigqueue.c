/* Copyright (C) 1997, 1998, 2000, 2003, 2005 Free Software Foundation, Inc.
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

#if defined __NR_rt_sigqueueinfo && defined __USE_POSIX199309
# include <signal.h>
# include <unistd.h>
# include <string.h>

# define __NR___syscall_rt_sigqueueinfo __NR_rt_sigqueueinfo
static __always_inline _syscall3(int, __syscall_rt_sigqueueinfo, pid_t, pid, int, sig, void*, value)

/* Return any pending signal or wait for one for the given time.  */
int sigqueue (pid_t pid, int sig, const union sigval val)
{
	siginfo_t info;

	/* First, clear the siginfo_t structure, so that we don't pass our
	   stack content to other tasks.  */
	memset(&info, 0, sizeof(info));
	/* We must pass the information about the data in a siginfo_t value.  */
	info.si_signo = sig;
	info.si_code = SI_QUEUE;
	info.si_pid = getpid ();
	info.si_uid = getuid ();
	info.si_value = val;

	return __syscall_rt_sigqueueinfo(pid, sig, &info);
}

#endif
