/* Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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

#include <pthreadP.h>
#include <signal.h>

/* We use the libc implementation but we tell it to not allow
   SIGCANCEL or SIGTIMER to be handled.  */

extern __typeof(sigaction) __libc_sigaction;
int
__sigaction (int sig, const struct sigaction *act, struct sigaction *oact)
{
  if (unlikely (sig == SIGCANCEL || sig == SIGSETXID))
    {
      __set_errno (EINVAL);
      return -1;
    }

  return __libc_sigaction (sig, act, oact);
}
libc_hidden_proto(sigaction)
weak_alias (__sigaction, sigaction)
libc_hidden_weak(sigaction)
