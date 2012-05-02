/* Copyright (C) 1991, 1996, 1997, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stddef.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define CS_PATH "/bin:/usr/bin"

/* If BUF is not NULL and LEN > 0, fill in at most LEN - 1 bytes
   of BUF with the value corresponding to NAME and zero-terminate BUF.
   Return the number of bytes required to hold NAME's entire value.  */
size_t confstr ( int name, char *buf, size_t len)
{
  const char *string;
  size_t string_len;

  switch (name)
    {
    case _CS_PATH:
      {
	static const char cs_path[] = CS_PATH;
	string = cs_path;
	string_len = sizeof (cs_path);
      }
      break;
    default:
      __set_errno (EINVAL);
      return 0;
    }

  if (len > 0 && buf != NULL)
    {
      if (string_len <= len)
	memcpy (buf, string, string_len);
      else
	{
	  memcpy (buf, string, len - 1);
	  buf[len - 1] = '\0';
	}
    }
  return string_len;
}
