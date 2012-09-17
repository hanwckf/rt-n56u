/* Copyright (C) 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de>, 1999.

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
#include <malloc.h>
#include <stdio.h>
#include <features.h>

static int errors = 0;

static void
merror (const char *msg)
{
  ++errors;
  printf ("Error: %s\n", msg);
}

int
main (void)
{
  void *p;
  int save;

  errno = 0;

  p = malloc (-1);
  save = errno;

  if (p != NULL)
    merror ("malloc (-1) succeeded.");

  if (p == NULL && save != ENOMEM)
    merror ("errno is not set correctly");

  p = malloc (10);
  if (p == NULL)
    merror ("malloc (10) failed.");

  /* realloc (p, 0) == free (p).  */
  p = realloc (p, 0);
  if (p != NULL)
    merror ("realloc (p, 0) failed.");

  p = malloc (0);
#if !defined(__UCLIBC__) || defined(__MALLOC_GLIBC_COMPAT__)
  if (p == NULL)
#else
  if (p != NULL)
#endif
    merror ("malloc (0) failed.");

  p = realloc (p, 0);
  if (p != NULL)
    merror ("realloc (p, 0) failed.");

  return errors != 0;
}
