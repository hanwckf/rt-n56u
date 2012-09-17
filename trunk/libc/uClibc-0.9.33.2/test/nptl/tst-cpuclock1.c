/* Test program for process CPU clocks.
   Copyright (C) 2004 Free Software Foundation, Inc.
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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/wait.h>

/* This function is intended to rack up both user and system time.  */
static void
chew_cpu (void)
{
  while (1)
    {
      static volatile char buf[4096];
      for (int i = 0; i < 100; ++i)
	for (size_t j = 0; j < sizeof buf; ++j)
	  buf[j] = 0xaa;
      int nullfd = open ("/dev/null", O_WRONLY);
      for (int i = 0; i < 100; ++i)
	for (size_t j = 0; j < sizeof buf; ++j)
	  buf[j] = 0xbb;
      write (nullfd, (char *) buf, sizeof buf);
      close (nullfd);
      if (getppid () == 1)
	_exit (2);
    }
}

static int
do_test (void)
{
  int result = 0;
  clockid_t cl;
  int e;
  pid_t dead_child, child;

  /* Fork a child and let it die, to give us a PID known not be valid
     (assuming PIDs don't wrap around during the test).  */
  {
    dead_child = fork ();
    if (dead_child == 0)
      _exit (0);
    if (dead_child < 0)
      {
	perror ("fork");
	return 1;
      }
    int x;
    if (wait (&x) != dead_child)
      {
	perror ("wait");
	return 2;
      }
  }

  /* POSIX says we should get ESRCH for this.  */
  e = clock_getcpuclockid (dead_child, &cl);
  if (e != ENOSYS && e != ESRCH && e != EPERM)
    {
      printf ("clock_getcpuclockid on dead PID %d => %s\n",
	      dead_child, strerror (e));
      result = 1;
    }

  /* Now give us a live child eating up CPU time.  */
  child = fork ();
  if (child == 0)
    {
      chew_cpu ();
      _exit (1);
    }
  if (child < 0)
    {
      perror ("fork");
      return 1;
    }

  e = clock_getcpuclockid (child, &cl);
  if (e == EPERM)
    {
      puts ("clock_getcpuclockid does not support other processes");
      goto done;
    }
  if (e != 0)
    {
      printf ("clock_getcpuclockid on live PID %d => %s\n",
	      child, strerror (e));
      result = 1;
      goto done;
    }

  const clockid_t child_clock = cl;
  struct timespec res;
  if (clock_getres (child_clock, &res) < 0)
    {
      printf ("clock_getres on live PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
      result = 1;
      goto done;
    }
  printf ("live PID %d clock %lx resolution %lu.%.9lu\n",
	  child, (unsigned long int) child_clock, res.tv_sec, res.tv_nsec);

  struct timespec before, after;
  if (clock_gettime (child_clock, &before) < 0)
    {
      printf ("clock_gettime on live PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
      result = 1;
      goto done;
    }
  printf ("live PID %d before sleep => %lu.%.9lu\n",
	  child, before.tv_sec, before.tv_nsec);

  struct timespec sleeptime = { .tv_nsec = 500000000 };
  nanosleep (&sleeptime, NULL);

  if (clock_gettime (child_clock, &after) < 0)
    {
      printf ("clock_gettime on live PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
      result = 1;
      goto done;
    }
  printf ("live PID %d after sleep => %lu.%.9lu\n",
	  child, after.tv_sec, after.tv_nsec);

  struct timespec diff = { .tv_sec = after.tv_sec - before.tv_sec,
			   .tv_nsec = after.tv_nsec - before.tv_nsec };
  if (diff.tv_nsec < 0)
    {
      --diff.tv_sec;
      diff.tv_nsec += 1000000000;
    }
  if (diff.tv_sec != 0
      || diff.tv_nsec > 600000000
      || diff.tv_nsec < 100000000)
    {
      printf ("before - after %lu.%.9lu outside reasonable range\n",
	      diff.tv_sec, diff.tv_nsec);
      result = 1;
    }

  sleeptime.tv_nsec = 100000000;
  e = clock_nanosleep (child_clock, 0, &sleeptime, NULL);
  if (e == EINVAL || e == ENOTSUP || e == ENOSYS)
    {
      printf ("clock_nanosleep not supported for other process clock: %s\n",
	      strerror (e));
    }
  else if (e != 0)
    {
      printf ("clock_nanosleep on other process clock: %s\n", strerror (e));
      result = 1;
    }
  else
    {
      struct timespec afterns;
      if (clock_gettime (child_clock, &afterns) < 0)
	{
	  printf ("clock_gettime on live PID %d clock %lx => %s\n",
		  child, (unsigned long int) child_clock, strerror (errno));
	  result = 1;
	}
      else
	{
	  struct timespec d = { .tv_sec = afterns.tv_sec - after.tv_sec,
				.tv_nsec = afterns.tv_nsec - after.tv_nsec };
	  if (d.tv_nsec < 0)
	    {
	      --d.tv_sec;
	      d.tv_nsec += 1000000000;
	    }
	  if (d.tv_sec > 0
	      || d.tv_nsec < sleeptime.tv_nsec
	      || d.tv_nsec > sleeptime.tv_nsec * 2)
	    {
	      printf ("nanosleep time %lu.%.9lu outside reasonable range\n",
		      d.tv_sec, d.tv_nsec);
	      result = 1;
	    }
	}
    }

  if (kill (child, SIGKILL) != 0)
    {
      perror ("kill");
      result = 2;
      goto done;
    }

  /* Wait long enough to let the child finish dying.  */

  sleeptime.tv_nsec = 200000000;
  nanosleep (&sleeptime, NULL);

  struct timespec dead;
  if (clock_gettime (child_clock, &dead) < 0)
    {
      printf ("clock_gettime on dead PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
      result = 1;
      goto done;
    }
  printf ("dead PID %d => %lu.%.9lu\n",
	  child, dead.tv_sec, dead.tv_nsec);

  diff.tv_sec = dead.tv_sec - after.tv_sec;
  diff.tv_nsec = dead.tv_nsec - after.tv_nsec;
  if (diff.tv_nsec < 0)
    {
      --diff.tv_sec;
      diff.tv_nsec += 1000000000;
    }
  if (diff.tv_sec != 0 || diff.tv_nsec > 200000000)
    {
      printf ("dead - after %lu.%.9lu outside reasonable range\n",
	      diff.tv_sec, diff.tv_nsec);
      result = 1;
    }

  /* Now reap the child and verify that its clock is no longer valid.  */
  {
    int x;
    if (waitpid (child, &x, 0) != child)
      {
	perror ("waitpid");
	result = 1;
      }
  }

  if (clock_gettime (child_clock, &dead) == 0)
    {
      printf ("clock_gettime on reaped PID %d clock %lx => %lu%.9lu\n",
	      child, (unsigned long int) child_clock,
	      dead.tv_sec, dead.tv_nsec);
      result = 1;
    }
  else
    {
      if (errno != EINVAL)
	result = 1;
      printf ("clock_gettime on reaped PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
    }

  if (clock_getres (child_clock, &dead) == 0)
    {
      printf ("clock_getres on reaped PID %d clock %lx => %lu%.9lu\n",
	      child, (unsigned long int) child_clock,
	      dead.tv_sec, dead.tv_nsec);
      result = 1;
    }
  else
    {
      if (errno != EINVAL)
	result = 1;
      printf ("clock_getres on reaped PID %d clock %lx => %s\n",
	      child, (unsigned long int) child_clock, strerror (errno));
    }

  return result;

 done:
  {
    if (kill (child, SIGKILL) != 0 && errno != ESRCH)
      {
	perror ("kill");
	return 2;
      }
    int x;
    if (waitpid (child, &x, 0) != child && errno != ECHILD)
      {
	perror ("waitpid");
	return 2;
      }
  }

  return result;
}


#define TIMEOUT 5
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
