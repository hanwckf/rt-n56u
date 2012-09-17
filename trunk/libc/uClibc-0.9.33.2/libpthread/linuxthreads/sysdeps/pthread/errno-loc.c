/* MT support function to get address of `errno' variable, linuxthreads
   version.
   Copyright (C) 1996, 1998, 2002, 2003, 2004 Free Software Foundation, Inc.
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

#include <errno.h>
#include <linuxthreads/internals.h>
#include <sysdep-cancel.h>

#if ! USE___THREAD && !RTLD_PRIVATE_ERRNO
#undef errno
extern int errno;
#endif

int *
#if ! USE___THREAD
weak_const_function
#endif
__errno_location (void)
{
#if ! USE___THREAD && !defined NOT_IN_libc
  if (! SINGLE_THREAD_P)
    {
      pthread_descr self = thread_self();
      return LIBC_THREAD_GETMEM (self, p_errnop);
    }
#endif
  return &errno;
}
libc_hidden_def (__errno_location)
