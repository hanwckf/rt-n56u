/* Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

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
#include <unistd.h>
#include <tls.h>

#if HAVE___THREAD && defined HAVE_TLS_MODEL_ATTRIBUTE

static __thread unsigned char foo [32]
  __attribute__ ((tls_model ("initial-exec"), aligned (sizeof (void *))));

void
test1 (void)
{
  size_t s;

  for (s = 0; s < sizeof (foo); ++s)
    {
      if (foo [s])
        abort ();
      foo [s] = s;
    }
}

void
test2 (void)
{
  size_t s;

  for (s = 0; s < sizeof (foo); ++s)
    {
      if (foo [s] != s)
        abort ();
      foo [s] = sizeof (foo) - s;
    }
}

#endif
