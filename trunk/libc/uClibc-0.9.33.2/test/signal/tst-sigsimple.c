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

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>


static int
do_test (void)
{
  int result = 0;
  int e;

#define RUN(test) \
  errno = 0;								      \
  e = test;								      \
  if (e != -1)								      \
    {									      \
      printf ("%s returned %d\n", #test, e);				      \
      result = 1;							      \
    }									      \
  else if (errno != EINVAL)						      \
    {									      \
      printf ("%s didn't set errno to EINVAL (%s instead)\n",		      \
	      #test, strerror (errno));					      \
      result = 1;							      \
    }

  RUN (sighold (-1));
  RUN (sighold (_NSIG + 100));

  RUN (sigrelse (-1));
  RUN (sigrelse (_NSIG + 100));

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
