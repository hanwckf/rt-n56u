/* Copyright (C) 1996, 97, 98, 2002, 2003 Free Software Foundation, Inc.
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

#include <netdb.h>
#include <tls.h>
#include <linuxthreads/internals.h>
#include <sysdep-cancel.h>

#if ! USE___THREAD
# undef h_errno
extern int h_errno;
#endif

/* When threaded, h_errno may be a per-thread variable.  */
int *
weak_const_function
__h_errno_location (void)
{
#if ! USE___THREAD
  if (! SINGLE_THREAD_P)
    {
      pthread_descr self = thread_self();
      return LIBC_THREAD_GETMEM (self, p_h_errnop);
    }
#endif
  return &h_errno;
}
libc_hidden_def (__h_errno_location)
