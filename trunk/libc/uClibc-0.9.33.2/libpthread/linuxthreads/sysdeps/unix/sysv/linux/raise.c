/* Copyright (C) 1991, 1996, 2002, 2003, 2005 Free Software Foundation, Inc.
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

#include <signal.h>
#include <unistd.h>

#ifndef IS_IN_rtld
# include <bits/libc-lock.h>

# ifndef SHARED
weak_extern (__pthread_raise)
# endif
#endif

/* Raise the signal SIG.  */
int
raise (sig)
     int sig;
{
#ifdef IS_IN_rtld
  return __kill (__getpid (), sig);
#else
  return __libc_maybe_call2 (pthread_raise, (sig),
			     __kill (__getpid (), sig));
#endif
}
libc_hidden_def (raise)
weak_alias (raise, gsignal)
