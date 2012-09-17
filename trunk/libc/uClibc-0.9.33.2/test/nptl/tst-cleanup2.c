/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Bao Duong <bduong@progress.com>, 2003.

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

#include <setjmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

static sigjmp_buf jmpbuf;

static void
sig_handler (int signo)
{
  siglongjmp (jmpbuf, 1);
}

static int
do_test (void)
{
  char *p = NULL;
  struct sigaction sa;

  sa.sa_handler = sig_handler;
  sigemptyset (&sa.sa_mask);
  sa.sa_flags = SA_SIGINFO;

  if (sigaction (SIGSEGV, &sa, 0))
    {
      perror ("installing SIGSEGV handler\n");
      exit (1);
    }

  puts ("Attempting to sprintf to null ptr");
  if (setjmp (jmpbuf))
    {
      puts ("Exiting main...");
      return 0;
    }

  sprintf (p, "This should segv\n");

  return 1;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
