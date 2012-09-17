/* vi: set sw=4 ts=4: */
/*
 * select() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/select.h>
#include <stdint.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#else
#define SINGLE_THREAD_P 1
#endif

#define USEC_PER_SEC 1000000L

extern __typeof(select) __libc_select;

#if !defined(__NR__newselect) && !defined(__NR_select) && defined __USE_XOPEN2K
# define __NR___libc_pselect6 __NR_pselect6
static __always_inline _syscall6(int, __libc_pselect6, int, n, fd_set *, readfds, fd_set *, writefds,
        fd_set *, exceptfds, const struct timespec *, timeout,
        const sigset_t *, sigmask)

int __libc_select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                  struct timeval *timeout)
{
	struct timespec _ts, *ts = 0;
	if (timeout) {
		uint32_t usec;
		_ts.tv_sec = timeout->tv_sec;

		/* GNU extension: allow for timespec values where the sub-sec
		* field is equal to or more than 1 second.  The kernel will
		* reject this on us, so take care of the time shift ourself.
		* Some applications (like readline and linphone) do this.
		* See 'clarification on select() type calls and invalid timeouts'
		* on the POSIX general list for more information.
		*/
		usec = timeout->tv_usec;
		if (usec >= USEC_PER_SEC) {
			_ts.tv_sec += usec / USEC_PER_SEC;
			usec %= USEC_PER_SEC;
		}
		_ts.tv_nsec = usec * 1000;

		ts = &_ts;
	}

	if (SINGLE_THREAD_P)
		return __libc_pselect6(n, readfds, writefds, exceptfds, ts, 0);
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __libc_pselect6(n, readfds, writefds, exceptfds, ts, 0);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif

}

#else

#ifdef __NR__newselect
# define __NR___syscall_select __NR__newselect
#else
# define __NR___syscall_select __NR_select
#endif

static __always_inline _syscall5(int, __syscall_select, int, n, fd_set *, readfds,
		fd_set *, writefds, fd_set *, exceptfds, struct timeval *, timeout);

int __libc_select(int n, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
                  struct timeval *timeout)
{
	if (SINGLE_THREAD_P)
		return __syscall_select(n, readfds, writefds, exceptfds, timeout);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
	int oldtype = LIBC_CANCEL_ASYNC ();
	int result = __syscall_select(n, readfds, writefds, exceptfds, timeout);
	LIBC_CANCEL_RESET (oldtype);
	return result;
#endif
}

#endif

weak_alias(__libc_select,select)
libc_hidden_weak(select)
