/* Copyright (C) 1991,1993,1996-1999,2000 Free Software Foundation, Inc.
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
#include <string.h>
#include "../misc/internals/tempname.h"


static char tmpnam_buffer[L_tmpnam];

/* Generate a unique filename in P_tmpdir.

   This function is *not* thread safe!  */
char *
tmpnam (char *s)
{
  /* By using two buffers we manage to be thread safe in the case
     where S != NULL.  */
  char tmpbufmem[L_tmpnam];
  char *tmpbuf = s ?: tmpbufmem;

  /* In the following call we use the buffer pointed to by S if
     non-NULL although we don't know the size.  But we limit the size
     to L_tmpnam characters in any case.  */
  if (__builtin_expect (__path_search (tmpbuf, L_tmpnam, NULL, NULL, 0),
			0))
    return NULL;

  if (__builtin_expect (__gen_tempname (tmpbuf, __GT_NOCREATE, 0), 0))
    return NULL;

  if (s == NULL)
    return (char *) memcpy (tmpnam_buffer, tmpbuf, L_tmpnam);

  return s;
}

link_warning (tmpnam, "the use of `tmpnam' is dangerous, better use `mkstemp'")
