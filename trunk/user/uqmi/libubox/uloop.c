/*
 * uloop - event loop implementation
 *
 * Copyright (C) 2010-2013 Felix Fietkau <nbd@openwrt.org>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <sys/time.h>
#include <sys/types.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <poll.h>
#include <string.h>
#include <fcntl.h>
#include <stdbool.h>

#include "uloop.h"
#include "utils.h"

#ifdef USE_KQUEUE
#include <sys/event.h>
#endif
#ifdef USE_EPOLL
#include <sys/epoll.h>
#endif
#include <sys/wait.h>

#define ULOOP_MAX_EVENTS 10

static struct list_head timeouts = LIST_HEAD_INIT(timeouts);
static struct list_head processes = LIST_HEAD_INIT(processes);

static int poll_fd = -1;
bool uloop_cancelled = false;
bool uloop_handle_sigchld = true;
static bool do_sigchld = false;
static int cur_fd, cur_nfds;

#ifdef USE_KQUEUE

int uloop_init(void)
{
	struct timespec timeout = { 0, 0 };
	struct kevent ev = {};

	if (poll_fd >= 0)
		return 0;

	poll_fd = kqueue();
	if (poll_fd < 0)
		return -1;

	EV_SET(&ev, SIGCHLD, EVFILT_SIGNAL, EV_ADD, 0, 0, 0);
	kevent(poll_fd, &ev, 1, NULL, 0, &timeout);

	return 0;
}


static uint16_t get_flags(unsigned int flags, unsigned int mask)
{
	uint16_t kflags = 0;

	if (!(flags & mask))
		return EV_DELETE;

	kflags = EV_ADD;
	if (flags & ULOOP_EDGE_TRIGGER)
		kflags |= EV_CLEAR;

	return kflags;
}

static struct kevent events[ULOOP_MAX_EVENTS];

static int register_kevent(struct uloop_fd *fd, unsigned int flags)
{
	struct timespec timeout = { 0, 0 };
	struct kevent ev[2];
	int nev = 0;
	unsigned int fl = 0;
	unsigned int changed;
	uint16_t kflags;

	if (flags & ULOOP_EDGE_DEFER)
		flags &= ~ULOOP_EDGE_TRIGGER;

	changed = flags ^ fd->flags;
	if (changed & ULOOP_EDGE_TRIGGER)
		changed |= flags;

	if (changed & ULOOP_READ) {
		kflags = get_flags(flags, ULOOP_READ);
		EV_SET(&ev[nev++], fd->fd, EVFILT_READ, kflags, 0, 0, fd);
	}

	if (changed & ULOOP_WRITE) {
		kflags = get_flags(flags, ULOOP_WRITE);
		EV_SET(&ev[nev++], fd->fd, EVFILT_WRITE, kflags, 0, 0, fd);
	}

	if (!flags)
		fl |= EV_DELETE;

	fd->flags = flags;
	if (kevent(poll_fd, ev, nev, NULL, fl, &timeout) == -1)
		return -1;

	return 0;
}

static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	if (flags & ULOOP_EDGE_TRIGGER)
		flags |= ULOOP_EDGE_DEFER;
	else
		flags &= ~ULOOP_EDGE_DEFER;

	return register_kevent(fd, flags);
}

int uloop_fd_delete(struct uloop_fd *sock)
{
	int i;

	for (i = cur_fd + 1; i < cur_nfds; i++) {
		if (events[i].udata != sock)
			continue;

		events[i].udata = NULL;
	}

	sock->registered = false;
	return register_poll(sock, 0);
}

static void uloop_run_events(int timeout)
{
	struct timespec ts;
	int nfds, n;

	if (timeout >= 0) {
		ts.tv_sec = timeout / 1000;
		ts.tv_nsec = (timeout % 1000) * 1000000;
	}

	nfds = kevent(poll_fd, NULL, 0, events, ARRAY_SIZE(events), timeout >= 0 ? &ts : NULL);
	for(n = 0; n < nfds; ++n)
	{
		struct uloop_fd *u = events[n].udata;
		unsigned int ev = 0;

		if (!u)
			continue;

		if (events[n].flags & EV_ERROR) {
			u->error = true;
			uloop_fd_delete(u);
		}

		if(events[n].filter == EVFILT_READ)
			ev |= ULOOP_READ;
		else if (events[n].filter == EVFILT_WRITE)
			ev |= ULOOP_WRITE;

		if (events[n].flags & EV_EOF)
			u->eof = true;
		else if (!ev)
			continue;

		if (u->cb) {
			cur_fd = n;
			cur_nfds = nfds;
			u->cb(u, ev);
			if (u->flags & ULOOP_EDGE_DEFER) {
				u->flags &= ~ULOOP_EDGE_DEFER;
				register_kevent(u, u->flags);
			}
		}
	}
	cur_nfds = 0;
}

#endif

#ifdef USE_EPOLL

/**
 * FIXME: uClibc < 0.9.30.3 does not define EPOLLRDHUP for Linux >= 2.6.17
 */
#ifndef EPOLLRDHUP
#define EPOLLRDHUP 0x2000
#endif

int uloop_init(void)
{
	if (poll_fd >= 0)
		return 0;

	poll_fd = epoll_create(32);
	if (poll_fd < 0)
		return -1;

	fcntl(poll_fd, F_SETFD, fcntl(poll_fd, F_GETFD) | FD_CLOEXEC);
	return 0;
}

static int register_poll(struct uloop_fd *fd, unsigned int flags)
{
	struct epoll_event ev;
	int op = fd->registered ? EPOLL_CTL_MOD : EPOLL_CTL_ADD;

	memset(&ev, 0, sizeof(struct epoll_event));

	if (flags & ULOOP_READ)
		ev.events |= EPOLLIN | EPOLLRDHUP;

	if (flags & ULOOP_WRITE)
		ev.events |= EPOLLOUT;

	if (flags & ULOOP_EDGE_TRIGGER)
		ev.events |= EPOLLET;

	ev.data.fd = fd->fd;
	ev.data.ptr = fd;

	return epoll_ctl(poll_fd, op, fd->fd, &ev);
}

static struct epoll_event events[ULOOP_MAX_EVENTS];

int uloop_fd_delete(struct uloop_fd *sock)
{
	int i;

	if (!sock->registered)
		return 0;

	for (i = cur_fd + 1; i < cur_nfds; i++) {
		if (events[i].data.ptr != sock)
			continue;

		events[i].data.ptr = NULL;
	}
	sock->registered = false;
	return epoll_ctl(poll_fd, EPOLL_CTL_DEL, sock->fd, 0);
}

static void uloop_run_events(int timeout)
{
	int n, nfds;

	nfds = epoll_wait(poll_fd, events, ARRAY_SIZE(events), timeout);
	for(n = 0; n < nfds; ++n)
	{
		struct uloop_fd *u = events[n].data.ptr;
		unsigned int ev = 0;

		if (!u)
			continue;

		if(events[n].events & (EPOLLERR|EPOLLHUP)) {
			u->error = true;
			uloop_fd_delete(u);
		}

		if(!(events[n].events & (EPOLLRDHUP|EPOLLIN|EPOLLOUT|EPOLLERR|EPOLLHUP)))
			continue;

		if(events[n].events & EPOLLRDHUP)
			u->eof = true;

		if(events[n].events & EPOLLIN)
			ev |= ULOOP_READ;

		if(events[n].events & EPOLLOUT)
			ev |= ULOOP_WRITE;

		if(u->cb) {
			cur_fd = n;
			cur_nfds = nfds;
			u->cb(u, ev);
		}
	}
	cur_nfds = 0;
}

#endif

int uloop_fd_add(struct uloop_fd *sock, unsigned int flags)
{
	unsigned int fl;
	int ret;

	if (!(flags & (ULOOP_READ | ULOOP_WRITE)))
		return uloop_fd_delete(sock);

	if (!sock->registered && !(flags & ULOOP_BLOCKING)) {
		fl = fcntl(sock->fd, F_GETFL, 0);
		fl |= O_NONBLOCK;
		fcntl(sock->fd, F_SETFL, fl);
	}

	ret = register_poll(sock, flags);
	if (ret < 0)
		goto out;

	sock->registered = true;
	sock->eof = false;

out:
	return ret;
}

static int tv_diff(struct timeval *t1, struct timeval *t2)
{
	return
		(t1->tv_sec - t2->tv_sec) * 1000 +
		(t1->tv_usec - t2->tv_usec) / 1000;
}

int uloop_timeout_add(struct uloop_timeout *timeout)
{
	struct uloop_timeout *tmp;
	struct list_head *h = &timeouts;

	if (timeout->pending)
		return -1;

	list_for_each_entry(tmp, &timeouts, list) {
		if (tv_diff(&tmp->time, &timeout->time) > 0) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&timeout->list, h);
	timeout->pending = true;

	return 0;
}

static void uloop_gettime(struct timeval *tv)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);
	tv->tv_sec = ts.tv_sec;
	tv->tv_usec = ts.tv_nsec / 1000;
}

int uloop_timeout_set(struct uloop_timeout *timeout, int msecs)
{
	struct timeval *time = &timeout->time;

	if (timeout->pending)
		uloop_timeout_cancel(timeout);

	uloop_gettime(&timeout->time);

	time->tv_sec += msecs / 1000;
	time->tv_usec += (msecs % 1000) * 1000;

	if (time->tv_usec > 1000000) {
		time->tv_sec++;
		time->tv_usec %= 1000000;
	}

	return uloop_timeout_add(timeout);
}

int uloop_timeout_cancel(struct uloop_timeout *timeout)
{
	if (!timeout->pending)
		return -1;

	list_del(&timeout->list);
	timeout->pending = false;

	return 0;
}

int uloop_timeout_remaining(struct uloop_timeout *timeout)
{
	struct timeval now;

	if (!timeout->pending)
		return -1;

	uloop_gettime(&now);

	return tv_diff(&timeout->time, &now);
}

int uloop_process_add(struct uloop_process *p)
{
	struct uloop_process *tmp;
	struct list_head *h = &processes;

	if (p->pending)
		return -1;

	list_for_each_entry(tmp, &processes, list) {
		if (tmp->pid > p->pid) {
			h = &tmp->list;
			break;
		}
	}

	list_add_tail(&p->list, h);
	p->pending = true;

	return 0;
}

int uloop_process_delete(struct uloop_process *p)
{
	if (!p->pending)
		return -1;

	list_del(&p->list);
	p->pending = false;

	return 0;
}

static void uloop_handle_processes(void)
{
	struct uloop_process *p, *tmp;
	pid_t pid;
	int ret;

	do_sigchld = false;

	while (1) {
		pid = waitpid(-1, &ret, WNOHANG);
		if (pid <= 0)
			return;

		list_for_each_entry_safe(p, tmp, &processes, list) {
			if (p->pid < pid)
				continue;

			if (p->pid > pid)
				break;

			uloop_process_delete(p);
			p->cb(p, ret);
		}
	}
}

#if !defined(EXT_SIGINT)
static void uloop_handle_sigint(int signo)
{
	uloop_cancelled = true;
}
#endif

static void uloop_sigchld(int signo)
{
	do_sigchld = true;
}

static void uloop_setup_signals(void)
{
	struct sigaction s;

	memset(&s, 0, sizeof(struct sigaction));
#if !defined(EXT_SIGINT)
	s.sa_handler = uloop_handle_sigint;
	sigaction(SIGINT, &s, NULL);
#endif
	if (uloop_handle_sigchld) {
		s.sa_handler = uloop_sigchld;
		sigaction(SIGCHLD, &s, NULL);
	}
}

static int uloop_get_next_timeout(struct timeval *tv)
{
	struct uloop_timeout *timeout;
	int diff;

	if (list_empty(&timeouts))
		return -1;

	timeout = list_first_entry(&timeouts, struct uloop_timeout, list);
	diff = tv_diff(&timeout->time, tv);
	if (diff < 0)
		return 0;

	return diff;
}

static void uloop_process_timeouts(struct timeval *tv)
{
	struct uloop_timeout *t;

	while (!list_empty(&timeouts)) {
		t = list_first_entry(&timeouts, struct uloop_timeout, list);

		if (tv_diff(&t->time, tv) > 0)
			break;

		uloop_timeout_cancel(t);
		if (t->cb)
			t->cb(t);
	}
}

static void uloop_clear_timeouts(void)
{
	struct uloop_timeout *t, *tmp;

	list_for_each_entry_safe(t, tmp, &timeouts, list)
		uloop_timeout_cancel(t);
}

static void uloop_clear_processes(void)
{
	struct uloop_process *p, *tmp;

	list_for_each_entry_safe(p, tmp, &processes, list)
		uloop_process_delete(p);
}

void uloop_run(void)
{
	struct timeval tv;

	uloop_setup_signals();
	while(!uloop_cancelled)
	{
		uloop_gettime(&tv);
		uloop_process_timeouts(&tv);
		if (uloop_cancelled)
			break;

		if (do_sigchld)
			uloop_handle_processes();
		uloop_run_events(uloop_get_next_timeout(&tv));
	}
}

void uloop_done(void)
{
	if (poll_fd < 0)
		return;

	close(poll_fd);
	poll_fd = -1;

	uloop_clear_timeouts();
	uloop_clear_processes();
}
