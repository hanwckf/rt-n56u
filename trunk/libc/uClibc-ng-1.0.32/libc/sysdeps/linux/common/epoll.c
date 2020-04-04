/*
 * epoll_create() / epoll_ctl() / epoll_wait() / epoll_pwait() for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sys/epoll.h>
#include <cancel.h>

#ifdef L_epoll_create
# ifdef __NR_epoll_create
_syscall1(int, epoll_create, int, size)
# endif

# ifdef __NR_epoll_create1
_syscall1(int, epoll_create1, int, flags)
# endif

# if defined __NR_epoll_create1 && !defined __NR_epoll_create
int epoll_create(int size)
{
	return INLINE_SYSCALL(epoll_create1, 1, 0);
}
# endif
#endif

#if defined L_epoll_ctl && defined __NR_epoll_ctl
_syscall4(int, epoll_ctl, int, epfd, int, op, int, fd, struct epoll_event *, event)
#endif

#if defined L_epoll_pwait && defined __NR_epoll_pwait
# include <signal.h>

# define __NR___syscall_epoll_pwait __NR_epoll_pwait
static __always_inline _syscall6(int, __syscall_epoll_pwait, int, epfd, struct epoll_event *, events,
				 int, maxevents, int, timeout, const sigset_t *, sigmask, size_t, sigsetsize)

static int __NC(epoll_pwait)(int epfd, struct epoll_event *events, int maxevents, int timeout,
			     const sigset_t *set)
{
	return __syscall_epoll_pwait(epfd, events, maxevents, timeout, set, __SYSCALL_SIGSET_T_SIZE);
}
CANCELLABLE_SYSCALL(int, epoll_pwait, (int epfd, struct epoll_event *events, int maxevents, int timeout,
				       const sigset_t *set),
		    (epfd, events, maxevents, timeout, set))
#endif

#if defined L_epoll_wait
# if defined __NR_epoll_wait
static int __NC(epoll_wait)(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return INLINE_SYSCALL(epoll_wait, 4, epfd, events, maxevents, timeout);
}
CANCELLABLE_SYSCALL(int, epoll_wait, (int epfd, struct epoll_event *events, int maxevents, int timeout),
		    (epfd, events, maxevents, timeout))


# elif /* !defined L_epoll_wait && */ defined __NR_epoll_pwait
/*
 * If epoll_wait is not defined, then call epoll_pwait instead using NULL
 * for sigmask argument
 */
#  include <stddef.h>
int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
	return INLINE_SYSCALL(epoll_pwait, 5, epfd, events, maxevents, timeout, NULL);
}
# endif
#endif
