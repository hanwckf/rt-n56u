/* Copyright (C) 1991, 1995, 1996, 1997, 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <string.h>


/* Find the last occurrence of C in S.  */
char *strrchr (const char *s, int c)
{
  register const char *found, *p;

  c = (unsigned char) c;

  /* Since strchr is fast, we use it rather than the obvious loop.  */

  if (c == '\0')
    return strchr (s, '\0');

  found = NULL;
  while ((p = strchr (s, c)) != NULL)
    {
      found = p;
      s = p + 1;
    }

  return (char *) found;
}
libc_hidden_weak(strrchr)
#ifdef __UCLIBC_SUSV3_LEGACY__
weak_alias(strrchr,rindex)
#endif
