/* Copyright (C) 1994,1996,1997,1998,1999,2001,2002
   Free Software Foundation, Inc.
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

#include <sys/syscall.h>
#include <sys/poll.h>
#include <bits/kernel-features.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
#include <sysdep-cancel.h>
#else
#define SINGLE_THREAD_P 1
#endif

libc_hidden_proto(poll)

#if defined __ASSUME_POLL_SYSCALL && defined __NR_poll

#define __NR___syscall_poll __NR_poll
static __always_inline _syscall3(int, __syscall_poll, struct pollfd *, fds,
			unsigned long int, nfds, int, timeout);

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    if (SINGLE_THREAD_P)
	return __syscall_poll(fds, nfds, timeout);

#ifdef __UCLIBC_HAS_THREADS_NATIVE__
    int oldtype = LIBC_CANCEL_ASYNC ();
    int result = __syscall_poll(fds, nfds, timeout);
    LIBC_CANCEL_RESET (oldtype);
    return result;
#endif
}
#else /* !__NR_poll */

#include <alloca.h>
#include <sys/types.h>
#include <errno.h>
#include <string.h>
#include <sys/time.h>
#include <sys/param.h>
#include <unistd.h>

libc_hidden_proto(getdtablesize)
libc_hidden_proto(select)

/* uClinux 2.0 doesn't have poll, emulate it using select */

/* Poll the file descriptors described by the NFDS structures starting at
   FDS.  If TIMEOUT is nonzero and not -1, allow TIMEOUT milliseconds for
   an event to occur; if TIMEOUT is -1, block until an event occurs.
   Returns the number of file descriptors with events, zero if timed out,
   or -1 for errors.  */

int poll(struct pollfd *fds, nfds_t nfds, int timeout)
{
    static int max_fd_size;
    struct timeval tv;
    fd_set *rset, *wset, *xset;
    struct pollfd *f;
    int ready;
    int maxfd = 0;
    int bytes;

    if (!max_fd_size)
	max_fd_size = getdtablesize ();

    bytes = howmany (max_fd_size, __NFDBITS);
    rset = alloca (bytes);
    wset = alloca (bytes);
    xset = alloca (bytes);

    /* We can't call FD_ZERO, since FD_ZERO only works with sets
       of exactly __FD_SETSIZE size.  */
    memset (rset, 0, bytes);
    memset (wset, 0, bytes);
    memset (xset, 0, bytes);

    for (f = fds; f < &fds[nfds]; ++f)
    {
	f->revents = 0;
	if (f->fd >= 0)
	{
	    if (f->fd >= max_fd_size)
	    {
		/* The user provides a file descriptor number which is higher
		   than the maximum we got from the `getdtablesize' call.
		   Maybe this is ok so enlarge the arrays.  */
		fd_set *nrset, *nwset, *nxset;
		int nbytes;

		max_fd_size = roundup (f->fd, __NFDBITS);
		nbytes = howmany (max_fd_size, __NFDBITS);

		nrset = alloca (nbytes);
		nwset = alloca (nbytes);
		nxset = alloca (nbytes);

		memset ((char *) nrset + bytes, 0, nbytes - bytes);
		memset ((char *) nwset + bytes, 0, nbytes - bytes);
		memset ((char *) nxset + bytes, 0, nbytes - bytes);

		rset = memcpy (nrset, rset, bytes);
		wset = memcpy (nwset, wset, bytes);
		xset = memcpy (nxset, xset, bytes);

		bytes = nbytes;
	    }

	    if (f->events & POLLIN)
		FD_SET (f->fd, rset);
	    if (f->events & POLLOUT)
		FD_SET (f->fd, wset);
	    if (f->events & POLLPRI)
		FD_SET (f->fd, xset);
	    if (f->fd > maxfd && (f->events & (POLLIN|POLLOUT|POLLPRI)))
		maxfd = f->fd;
	}
    }

    tv.tv_sec = timeout / 1000;
    tv.tv_usec = (timeout % 1000) * 1000;

    while (1)
    {
	ready = select (maxfd + 1, rset, wset, xset,
		timeout == -1 ? NULL : &tv);

	/* It might be that one or more of the file descriptors is invalid.
	   We now try to find and mark them and then try again.  */
	if (ready == -1 && errno == EBADF)
	{
	    fd_set *sngl_rset = alloca (bytes);
	    fd_set *sngl_wset = alloca (bytes);
	    fd_set *sngl_xset = alloca (bytes);
	    struct timeval sngl_tv;

	    /* Clear the original set.  */
	    memset (rset, 0, bytes);
	    memset (wset, 0, bytes);
	    memset (xset, 0, bytes);

	    /* This means we don't wait for input.  */
	    sngl_tv.tv_sec = 0;
	    sngl_tv.tv_usec = 0;

	    maxfd = -1;

	    /* Reset the return value.  */
	    ready = 0;

	    for (f = fds; f < &fds[nfds]; ++f)
		if (f->fd != -1 && (f->events & (POLLIN|POLLOUT|POLLPRI))
			&& (f->revents & POLLNVAL) == 0)
		{
		    int n;

		    memset (sngl_rset, 0, bytes);
		    memset (sngl_wset, 0, bytes);
		    memset (sngl_xset, 0, bytes);

		    if (f->events & POLLIN)
			FD_SET (f->fd, sngl_rset);
		    if (f->events & POLLOUT)
			FD_SET (f->fd, sngl_wset);
		    if (f->events & POLLPRI)
			FD_SET (f->fd, sngl_xset);

		    n = select (f->fd + 1, sngl_rset, sngl_wset, sngl_xset,
			    &sngl_tv);
		    if (n != -1)
		    {
			/* This descriptor is ok.  */
			if (f->events & POLLIN)
			    FD_SET (f->fd, rset);
			if (f->events & POLLOUT)
			    FD_SET (f->fd, wset);
			if (f->events & POLLPRI)
			    FD_SET (f->fd, xset);
			if (f->fd > maxfd)
			    maxfd = f->fd;
			if (n > 0)
			    /* Count it as being available.  */
			    ++ready;
		    }
		    else if (errno == EBADF)
			f->revents |= POLLNVAL;
		}
	    /* Try again.  */
	    continue;
	}

	break;
    }

    if (ready > 0)
	for (f = fds; f < &fds[nfds]; ++f)
	{
	    if (f->fd >= 0)
	    {
		if (FD_ISSET (f->fd, rset))
		    f->revents |= POLLIN;
		if (FD_ISSET (f->fd, wset))
		    f->revents |= POLLOUT;
		if (FD_ISSET (f->fd, xset))
		    f->revents |= POLLPRI;
	    }
	}

    return ready;
}

#endif
libc_hidden_def(poll)
