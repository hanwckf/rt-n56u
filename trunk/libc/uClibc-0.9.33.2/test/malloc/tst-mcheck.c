/* Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2005.

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
#include <stdio.h>
#include <stdlib.h>
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
  void *p, *q;

  errno = 0;

  p = malloc (-1);

  if (p != NULL)
    merror ("malloc (-1) succeeded.");
  else if (errno != ENOMEM)
    merror ("errno is not set correctly.");

  p = malloc (10);
  if (p == NULL)
    merror ("malloc (10) failed.");

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

  q = malloc (256);
  if (q == NULL)
    merror ("malloc (256) failed.");

  p = malloc (512);
  if (p == NULL)
    merror ("malloc (512) failed.");

  if (realloc (p, -256) != NULL)
    merror ("realloc (p, -256) succeeded.");
  else if (errno != ENOMEM)
    merror ("errno is not set correctly.");

  free (p);

  p = malloc (512);
  if (p == NULL)
    merror ("malloc (512) failed.");

  if (realloc (p, -1) != NULL)
    merror ("realloc (p, -1) succeeded.");
  else if (errno != ENOMEM)
    merror ("errno is not set correctly.");

  free (p);
  free (q);

  return errors != 0;
}
