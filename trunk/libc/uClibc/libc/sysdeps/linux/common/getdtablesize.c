/* Copyright (C) 1991, 1993, 1995, 1996, 1997 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <limits.h>

#define __LOCAL_OPEN_MAX	    256

/* Return the maximum number of file descriptors
   the current process could possibly have.  */
int getdtablesize (void)
{
  struct rlimit ru;

  /* This should even work if `getrlimit' is not implemented.  POSIX.1
     does not define this function but we will generate a stub which
     returns -1.  */
  return getrlimit (RLIMIT_NOFILE, &ru) < 0 ? __LOCAL_OPEN_MAX : ru.rlim_cur;
}

