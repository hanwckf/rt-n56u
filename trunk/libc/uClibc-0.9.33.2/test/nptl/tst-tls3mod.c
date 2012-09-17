/* Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthreaddef.h>
#include <tls.h>

#if HAVE___THREAD

static pthread_barrier_t* b = NULL;

#define TOTAL_SIGS 1000
static int* nsigs = NULL;

static sem_t* s = NULL;

static __thread void (*fp) (void);


#define THE_SIG SIGUSR1
void
handler (int sig)
{
  if (sig != THE_SIG)
    {
      write (STDOUT_FILENO, "wrong signal\n", 13);
      _exit (1);
    }

  fp ();

  if (sem_post (s) != 0)
    {
      write (STDOUT_FILENO, "sem_post failed\n", 16);
      _exit (1);
    }
}

void
setup_tf (pthread_barrier_t* t_b, int* t_nsigs, sem_t* t_s)
{
  b = t_b;
  nsigs = t_nsigs;
  s = t_s;
}

void *
tf (void *arg)
{
  if (!b || !s || !nsigs)
    {
      puts ("need to call setup_tf first");
      exit (1);
    }

  if ((uintptr_t) pthread_self () & (TCB_ALIGNMENT - 1))
    {
      puts ("thread's struct pthread not aligned enough");
      exit (1);
    }

  if (fp != NULL)
    {
      printf("fp=%p\n", (void *)&fp);
      puts ("fp not initially NULL");
      exit (1);
    }

  fp = arg;

  pthread_barrier_wait (b);

  pthread_barrier_wait (b);

  if (*nsigs != TOTAL_SIGS)
    {
      puts ("barrier_wait prematurely returns");
      exit (1);
    }

  return NULL;
}

#endif
