/* Handle locking of password file.
   Copyright (C) 1996,98,2000,02 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>
#include <paths.h>
#include <shadow.h>

/* How long to wait for getting the lock before returning with an error.  */
#define TIMEOUT 15 /* sec */

/* File descriptor for lock file.  */
static int lock_fd = -1;

/* Prevent problems in multithreaded program by using mutex.  */
#include <bits/uClibc_mutex.h>
__UCLIBC_MUTEX_STATIC(mylock, PTHREAD_MUTEX_INITIALIZER);

/* Prototypes for local functions.  */
static void noop_handler (int __sig);


int
lckpwdf (void)
{
  sigset_t saved_set;			/* Saved set of caught signals.  */
  struct sigaction saved_act;		/* Saved signal action.  */
  sigset_t new_set;			/* New set of caught signals.  */
  struct sigaction new_act;		/* New signal action.  */
  struct flock fl;			/* Information struct for locking.  */
  int result;
  int rv = -1;

  if (lock_fd != -1)
    /* Still locked by own process.  */
    return -1;

  /* Prevent problems caused by multiple threads.  */
  __UCLIBC_MUTEX_LOCK(mylock);

  lock_fd = open (_PATH_PASSWD, O_WRONLY | O_CLOEXEC);
  if (lock_fd == -1) {
    goto DONE;
  }
#ifndef __ASSUME_O_CLOEXEC
  /* Make sure file gets correctly closed when process finished.  */
  fcntl (lock_fd, F_SETFD, FD_CLOEXEC);
#endif

  /* Now we have to get exclusive write access.  Since multiple
     process could try this we won't stop when it first fails.
     Instead we set a timeout for the system call.  Once the timer
     expires it is likely that there are some problems which cannot be
     resolved by waiting. (sa_flags have no SA_RESTART. Thus SIGALRM
     will EINTR fcntl(F_SETLKW)

     It is important that we don't change the signal state.  We must
     restore the old signal behaviour.  */
  memset (&new_act, '\0', sizeof (new_act));
  new_act.sa_handler = noop_handler;
  __sigfillset (&new_act.sa_mask);

  /* Install new action handler for alarm and save old.
   * This never fails in Linux.  */
  sigaction (SIGALRM, &new_act, &saved_act);

  /* Now make sure the alarm signal is not blocked.  */
  __sigemptyset (&new_set);
  __sigaddset (&new_set, SIGALRM);
  sigprocmask (SIG_UNBLOCK, &new_set, &saved_set);

  /* Start timer.  If we cannot get the lock in the specified time we
     get a signal.  */
  alarm (TIMEOUT);

  /* Try to get the lock.  */
  memset (&fl, '\0', sizeof (fl));
  if (F_WRLCK)
    fl.l_type = F_WRLCK;
  if (SEEK_SET)
    fl.l_whence = SEEK_SET;
  result = fcntl (lock_fd, F_SETLKW, &fl);

  /* Clear alarm.  */
  alarm (0);

  sigprocmask (SIG_SETMASK, &saved_set, NULL);
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


int
ulckpwdf (void)
{
  int result;

  if (lock_fd == -1)
    /* There is no lock set.  */
    result = -1;
  else
    {
      /* Prevent problems caused by multiple threads.  */
      __UCLIBC_MUTEX_LOCK(mylock);

      result = close (lock_fd);

      /* Mark descriptor as unused.  */
      lock_fd = -1;

      /* Clear mutex.  */
      __UCLIBC_MUTEX_UNLOCK(mylock);
    }

  return result;
}


static void
noop_handler (int sig attribute_unused) {

  /* We simply return which makes the `fcntl' call return with an error.  */
}
