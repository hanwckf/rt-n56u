/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <tls.h>

#if HAVE___THREAD

#define N 10
static pthread_t th[N];


#define CB(n) \
static void								      \
cb##n (void)								      \
{									      \
  if (th[n] != pthread_self ())						      \
    {									      \
      write (STDOUT_FILENO, "wrong callback\n", 15);			      \
      _exit (1);							      \
    }									      \
}
CB (0)
CB (1)
CB (2)
CB (3)
CB (4)
CB (5)
CB (6)
CB (7)
CB (8)
CB (9)
static void (*cbs[]) (void) =
{
  cb0, cb1, cb2, cb3, cb4, cb5, cb6, cb7, cb8, cb9
};


static __thread void (*fp) (void) __attribute__ ((tls_model ("local-exec")));


static sem_t s;


#define THE_SIG SIGUSR1
static void
handler (int sig)
{
  if (sig != THE_SIG)
    {
      write (STDOUT_FILENO, "wrong signal\n", 13);
      _exit (1);
    }

  fp ();

  if (sem_post (&s) != 0)
    {
      write (STDOUT_FILENO, "sem_post failed\n", 16);
      _exit (1);
    }
}


static pthread_barrier_t b;

#define TOTAL_SIGS 1000
static int nsigs;


static void *
tf (void *arg)
{
  fp = arg;

  pthread_barrier_wait (&b);

  pthread_barrier_wait (&b);

  if (nsigs != TOTAL_SIGS)
    {
      puts ("barrier_wait prematurely returns");
      exit (1);
    }

  return NULL;
}
#endif

int
do_test (void)
{
#if !HAVE___THREAD

  puts ("No __thread support in compiler, test skipped.");

  return 0;
#else

  if (pthread_barrier_init (&b, NULL, N + 1) != 0)
    {
      puts ("barrier_init failed");
      exit (1);
    }

  if (sem_init (&s, 0, 0) != 0)
    {
      puts ("sem_init failed");
      exit (1);
    }

  struct sigaction sa;
  sa.sa_handler = handler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = 0;
  if (sigaction (THE_SIG, &sa, NULL) != 0)
    {
      puts ("sigaction failed");
      exit (1);
    }

  pthread_attr_t a;

  if (pthread_attr_init (&a) != 0)
    {
      puts ("attr_init failed");
      exit (1);
    }

  if (pthread_attr_setstacksize (&a, 1 * 1024 * 1024) != 0)
    {
      puts ("attr_setstacksize failed");
      return 1;
    }

  int i;
  for (i = 0; i < N; ++i)
    if (pthread_create (&th[i], &a, tf, cbs[i]) != 0)
      {
	puts ("pthread_create failed");
	exit (1);
      }

  if (pthread_attr_destroy (&a) != 0)
    {
      puts ("attr_destroy failed");
      exit (1);
    }

  pthread_barrier_wait (&b);

  sigset_t ss;
  sigemptyset (&ss);
  sigaddset (&ss, THE_SIG);
  if (pthread_sigmask (SIG_BLOCK, &ss, NULL) != 0)
    {
      puts ("pthread_sigmask failed");
      exit (1);
    }

  /* Start sending signals.  */
  for (i = 0; i < TOTAL_SIGS; ++i)
    {
      if (kill (getpid (), THE_SIG) != 0)
	{
	  puts ("kill failed");
	  exit (1);
	}

      if (TEMP_FAILURE_RETRY (sem_wait (&s)) != 0)
	{
	  puts ("sem_wait failed");
	  exit (1);
	}

      ++nsigs;
    }

  pthread_barrier_wait (&b);

  for (i = 0; i < N; ++i)
    if (pthread_join (th[i], NULL) != 0)
      {
	puts ("join failed");
	exit (1);
      }

  return 0;
#endif
}


#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
