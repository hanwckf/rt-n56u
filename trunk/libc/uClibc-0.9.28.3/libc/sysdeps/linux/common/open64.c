/* Copyright (C) 1991, 1995-1997, 1999, 2000 Free Software Foundation, Inc.
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


#include <features.h>
#include <fcntl.h>
#include <stdarg.h>

#ifndef O_LARGEFILE
#define O_LARGEFILE	0100000
#endif

#ifdef __UCLIBC_HAS_LFS__
extern int __libc_open (__const char *file, int oflag, mode_t mode);

/* Open FILE with access OFLAG.  If OFLAG includes O_CREAT,
   a third argument is the file protection.  */
int __libc_open64 (const char *file, int oflag, ...)
{
  int mode = 0;

  if (oflag & O_CREAT)
    {
      va_list arg;
      va_start (arg, oflag);
      mode = va_arg (arg, int);
      va_end (arg);
    }

  return __libc_open(file, oflag | O_LARGEFILE, mode);
}
weak_alias (__libc_open64, open64);
#endif /* __UCLIBC_HAS_LFS__ */
