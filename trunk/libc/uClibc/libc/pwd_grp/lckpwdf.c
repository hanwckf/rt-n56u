/* vi: set sw=4 ts=4: */
/* Handle locking of password file.
   Copyright (C) 1996, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1996.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <features.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <paths.h>

#include <bits/uClibc_mutex.h>

__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

/* How long to wait for getting the lock before returning with an
   error.  */
#define TIMEOUT 15 /* sec */

/* File descriptor for lock file.  */
static int lock_fd = -1;

/* Prototypes for local functions.  */
static void noop_handler __P ((int __sig));


int lckpwdf (void)
{
	int flags;
	sigset_t saved_set;         /* Saved set of caught signals.  */
	struct sigaction saved_act; /* Saved signal action.  */
	sigset_t new_set;           /* New set of caught signals.  */
	struct sigaction new_act;   /* New signal action.  */
	struct flock fl;            /* Information struct for locking.  */
	int result;
	int rv = -1;

	if (lock_fd != -1)
		/* Still locked by own process.  */
		return -1;

	__UCLIBC_MUTEX_LOCK(mylock);

	lock_fd = open (_PATH_PASSWD, O_WRONLY);
	if (lock_fd == -1) {
		/* Cannot create lock file.  */
		goto DONE;
	}

	/* Make sure file gets correctly closed when process finished.  */
	flags = fcntl (lock_fd, F_GETFD, 0);
	if (flags == -1) {
		/* Cannot get file flags.  */
		close(lock_fd);
		lock_fd = -1;
		goto DONE;
	}
	flags |= FD_CLOEXEC;		/* Close on exit.  */
	if (fcntl (lock_fd, F_SETFD, flags) < 0) {
		/* Cannot set new flags.  */
		close(lock_fd);
		lock_fd = -1;
		goto DONE;
	}

	/* Now we have to get exclusive write access.  Since multiple
	   process could try this we won't stop when it first fails.
	   Instead we set a timeout for the system call.  Once the timer
	   expires it is likely that there are some problems which cannot be
	   resolved by waiting.

	   It is important that we don't change the signal state.  We must
	   restore the old signal behaviour.  */
	memset (&new_act, '\0', sizeof (struct sigaction));
	new_act.sa_handler = noop_handler;
	sigfillset (&new_act.sa_mask);
	new_act.sa_flags = 0ul;

	/* Install new action handler for alarm and save old.  */
	if (sigaction (SIGALRM, &new_act, &saved_act) < 0) {
		/* Cannot install signal handler.  */
		close(lock_fd);
		lock_fd = -1;
		goto DONE;
	}

	/* Now make sure the alarm signal is not blocked.  */
	sigemptyset (&new_set);
	sigaddset (&new_set, SIGALRM);
	if (sigprocmask (SIG_UNBLOCK, &new_set, &saved_set) < 0) {
		sigaction (SIGALRM, &saved_act, NULL);
		close(lock_fd);
		lock_fd = -1;
		goto DONE;
	}

	/* Start timer.  If we cannot get the lock in the specified time we
	   get a signal.  */
	alarm (TIMEOUT);

	/* Try to get the lock.  */
	memset (&fl, '\0', sizeof (struct flock));
	fl.l_type = F_WRLCK;
	fl.l_whence = SEEK_SET;
	result = fcntl (lock_fd, F_SETLKW, &fl);

	/* Clear alarm.  */
	alarm (0);

	/* Restore old set of handled signals.  We don't need to know
	   about the current one.*/
	sigprocmask (SIG_SETMASK, &saved_set, NULL);

	/* Restore old action handler for alarm.  We don't need to know
	   about the current one.  */
	sigaction (SIGALRM, &saved_act, NULL);

	if (result < 0) {
		close(lock_fd);
		lock_fd = -1;
		goto DONE;
	}

	rv = 0;

 DONE:
	__UCLIBC_MUTEX_UNLOCK(mylock);
	return rv;
}


int ulckpwdf (void)
{
	int result;

	if (lock_fd == -1) {
		/* There is no lock set.  */
		result = -1;
	}
	else {
		__UCLIBC_MUTEX_LOCK(mylock);
		result = close (lock_fd);
		/* Mark descriptor as unused.  */
		lock_fd = -1;
		__UCLIBC_MUTEX_UNLOCK(mylock);
	}

	return result;
}


static void noop_handler (int sig)
{
	/* We simply return which makes the `fcntl' call return with an error.  */
}
