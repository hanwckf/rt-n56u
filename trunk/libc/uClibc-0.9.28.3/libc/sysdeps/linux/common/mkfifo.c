/* Copyright (C) 1991, 1996, 1997 Free Software Foundation, Inc.
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
   Boston, MA 02111-1307, USA.  
 
   June 28, 2000 -- swiped from GNU libc 2.1.3 and adjusted for uClibc
                    by Erik Andersen <andersee@debian.org>
 */

#include <errno.h>
#include <stddef.h>
#include <sys/stat.h>
#include <sys/types.h>

/* Create a named pipe (FIFO) named PATH with protections MODE.  */
int
mkfifo (const char *path, mode_t mode)
{
  return mknod (path, mode | S_IFIFO, 0);
}
