/* Copyright (C) 1996-1998,2001,2002,2003,2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

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

#include <features.h>

#ifdef __USE_XOPEN2K

#include <sys/syscall.h>
#include <sys/select.h>
#include <sys/time.h>
#include <signal.h>
#include <cancel.h>

static int __NC(pselect)(int nfds, fd_set *readfds, fd_set *writefds,
			 fd_set *exceptfds, const struct timespec *timeout,
			 const sigset_t *sigmask)
{
#ifdef __NR_pselect6
	/* The Linux kernel can in some situations update the timeout value.
	 * We do not want that so use a local variable.
	 */
	struct timespec _ts;

	/* Note: the system call expects 7 values but on most architectures
	   we can only pass in 6 directly.  If there is an architecture with
	   support for more parameters a new version of this file needs to
	   be created.  */
	struct {
		__kernel_ulong_t ss;
		__kernel_size_t  ss_len;
	} data;

	if (timeout != NULL) {
		_ts = *timeout;
		timeout = &_ts;
	}

	if (sigmask != NULL) {
		data.ss = (__kernel_ulong_t) sigmask;
		data.ss_len = __SYSCALL_SIGSET_T_SIZE;

		sigmask = (void *)&data;
	}

	return INLINE_SYSCALL(pselect6, 6, nfds, readfds, writefds, exceptfds, timeout, sigmask);
#else
	struct timeval tval;
	int retval;
	sigset_t savemask;

	/* Change nanosecond number to microseconds.  This might mean losing
	   precision and therefore the `pselect` should be available.  But
	   for now it is hardly found.  */
	if (timeout != NULL)
		TIMESPEC_TO_TIMEVAL (&tval, timeout);

	/* The setting and restoring of the signal mask and the select call
	   should be an atomic operation.  This can't be done without kernel
	   help.  */
	if (sigmask != NULL)
		sigprocmask (SIG_SETMASK, sigmask, &savemask);

	/* The comment below does not apply on uClibc, since we use __select_nocancel */
	/* Note the pselect() is a cancellation point.  But since we call
	   select() which itself is a cancellation point we do not have
	   to do anything here.  */
	retval = __NC(select)(nfds, readfds, writefds, exceptfds,
			timeout != NULL ? &tval : NULL);

	if (sigmask != NULL)
		sigprocmask (SIG_SETMASK, &savemask, NULL);

	return retval;
#endif
}
CANCELLABLE_SYSCALL(int, pselect, (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
				   const struct timespec *timeout, const sigset_t *sigmask),
		    (nfds, readfds, writefds, exceptfds, timeout, sigmask))

#endif
