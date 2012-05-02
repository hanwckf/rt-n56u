/* vi: set sw=4 ts=4: */
/*
 * epoll_create() / epoll_ctl() / epoll_wait() for uClibc
 *
 * Copyright (C) 2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 */

#include "syscalls.h"
#include <sys/epoll.h>

/*
 * epoll_create()
 */
#ifdef __NR_epoll_create
#define __NR___syscall_epoll_create __NR_epoll_create
static inline _syscall1(int, __syscall_epoll_create, int, size);
#endif

int epoll_create(int size)
{
#ifdef __NR_epoll_create
	return (__syscall_epoll_create(size));
#else
    __set_errno(ENOSYS);
    return -1;
#endif
}

/*
 * epoll_ctl()
 */
#ifdef __NR_epoll_ctl
#define __NR___syscall_epoll_ctl __NR_epoll_ctl
static inline _syscall4(int, __syscall_epoll_ctl, int, epfd, int, op, int, fd, struct epoll_event *, event);
#endif

int epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)
{
#ifdef __NR_epoll_ctl
	return (__syscall_epoll_ctl(epfd, op, fd, event));
#else
	__set_errno(ENOSYS);
	return -1;
#endif
}

/*
 * epoll_wait()
 */
#ifdef __NR_epoll_wait
#define __NR___syscall_epoll_wait __NR_epoll_wait
static inline _syscall4(int, __syscall_epoll_wait, int, epfd, struct epoll_event *, events, int, maxevents, int, timeout);
#endif

int epoll_wait(int epfd, struct epoll_event *events, int maxevents, int timeout)
{
#ifdef __NR_epoll_wait
	return (__syscall_epoll_wait(epfd, events, maxevents, timeout));
#else
	__set_errno(ENOSYS);
	return -1;
#endif
}
