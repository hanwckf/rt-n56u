/* vi: set sw=4 ts=4: */
/*
 * epoll_create() / epoll_ctl() / epoll_wait() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/epoll.h>
#ifdef __UCLIBC_HAS_THREADS_NATIVE__
# include <sysdep-cancel.h>
#else
# define SINGLE_THREAD_P 1
#endif

/*
 * epoll_create()
 */
#ifdef __NR_epoll_create
_syscall1(int, epoll_create, int, size)
#endif

/*
 * epoll_create1()
 */
#ifdef __NR_epoll_create1
_syscall1(int, epoll_create1, int, flags)
#endif

/*
 * epoll_ctl()
 */
#ifdef __NR_epoll_ctl
_syscall4(int,epoll_ctl, int, epfd, int, op, int, fd, struct epoll_event *, event)
#endif

/*
 * epoll_wait()
 */
#ifdef __NR_epoll_wait
extern __typeof(epoll_wait) __libc_epoll_wait;
int __libc_epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	if (SINGLE_THREAD_P)
		return INLINE_SYSCALL(epoll_wait, 4, epfd, events, maxevents, timeout);
# ifdef __UCLIBC_HAS_THREADS_NATIVE__
	else {
		int oldtype = LIBC_CANCEL_ASYNC ();
		int result = INLINE_SYSCALL(epoll_wait, 4, epfd, events, maxevents, timeout);
		LIBC_CANCEL_RESET (oldtype);
		return result;
	}
# endif
}
weak_alias(__libc_epoll_wait, epoll_wait)
#endif

/*
 * epoll_pwait()
 */
#ifdef __NR_epoll_pwait
# include <signal.h>

extern __typeof(epoll_pwait) __libc_epoll_pwait;
int __libc_epoll_pwait(int epfd, struct epoll_event *events, int maxevents,
						int timeout, const sigset_t *set)
{
    int nsig = _NSIG / 8;
	if (SINGLE_THREAD_P)
		return INLINE_SYSCALL(epoll_pwait, 6, epfd, events, maxevents, timeout, set, nsig);
# ifdef __UCLIBC_HAS_THREADS_NATIVE__
	else {
		int oldtype = LIBC_CANCEL_ASYNC ();
		int result = INLINE_SYSCALL(epoll_pwait, 6, epfd, events, maxevents, timeout, set, nsig);
		LIBC_CANCEL_RESET (oldtype);
		return result;
	}
# endif
}
weak_alias(__libc_epoll_pwait, epoll_pwait)
#endif
