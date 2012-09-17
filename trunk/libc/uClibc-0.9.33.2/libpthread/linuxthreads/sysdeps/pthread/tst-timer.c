/* Tests for POSIX timer implementation.
   Copyright (C) 2000, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Kaz Kylheku <kaz@ashi.footprints.net>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation; either version 2.1 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>


static void
notify_func (union sigval sigval)
{
  puts ("notify_func");
}


static void
signal_func (int sig)
{
  static const char text[] = "signal_func\n";
  signal (sig, signal_func);
  write (STDOUT_FILENO, text, sizeof text - 1);
}

static void
intr_sleep (int sec)
{
  struct timespec ts;

  ts.tv_sec = sec;
  ts.tv_nsec = 0;

  while (nanosleep (&ts, &ts) == -1 && errno == EINTR)
    ;
}

#define ZSIGALRM 14


int
main (void)
{
  struct timespec ts;
  timer_t timer_sig, timer_thr1, timer_thr2;
  int retval;
  struct sigevent sigev1 =
  {
    .sigev_notify = SIGEV_SIGNAL,
    .sigev_signo = ZSIGALRM
  };
  struct sigevent sigev2;
  struct itimerspec itimer1 = { { 0, 200000000 }, { 0, 200000000 } };
  struct itimerspec itimer2 = { { 0, 100000000 }, { 0, 500000000 } };
  struct itimerspec itimer3 = { { 0, 150000000 }, { 0, 300000000 } };
  struct itimerspec old;

  retval = clock_gettime (CLOCK_REALTIME, &ts);

  sigev2.sigev_notify = SIGEV_THREAD;
  sigev2.sigev_notify_function = notify_func;
  sigev2.sigev_notify_attributes = NULL;

  setvbuf (stdout, 0, _IOLBF, 0);

  printf ("clock_gettime returned %d, timespec = { %ld, %ld }\n",
	  retval, ts.tv_sec, ts.tv_nsec);

  retval = clock_getres (CLOCK_REALTIME, &ts);

  printf ("clock_getres returned %d, timespec = { %ld, %ld }\n",
	  retval, ts.tv_sec, ts.tv_nsec);

  timer_create (CLOCK_REALTIME, &sigev1, &timer_sig);
  timer_create (CLOCK_REALTIME, &sigev2, &timer_thr1);
  timer_create (CLOCK_REALTIME, &sigev2, &timer_thr2);

  timer_settime (timer_thr1, 0, &itimer2, &old);
  timer_settime (timer_thr2, 0, &itimer3, &old);

  signal (ZSIGALRM, signal_func);

  timer_settime (timer_sig, 0, &itimer1, &old);

  timer_delete (-1);

  intr_sleep (3);

  timer_delete (timer_sig);
  timer_delete (timer_thr1);

  intr_sleep (3);

  timer_delete (timer_thr2);

  return 0;
}
