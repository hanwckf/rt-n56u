/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2003.

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

#include <unistd.h>
#include <tls.h>
#include <sysdep.h>

#ifdef __NR_getxpid
# undef __NR_getpid
# define __NR_getpid __NR_getxpid
#endif

static pid_t
__getpid (void)
{
  INTERNAL_SYSCALL_DECL (err);
  pid_t result = INTERNAL_SYSCALL (getpid, err, 0);
  return result;
}
weak_alias(__getpid, getpid)
libc_hidden_weak(getpid)
#if !defined NOT_IN_libc && !defined __NR_getppid
strong_alias(getpid,getppid)
#endif
