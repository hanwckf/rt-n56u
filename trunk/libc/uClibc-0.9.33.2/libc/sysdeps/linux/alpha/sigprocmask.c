/* Copyright (C) 1993, 1995, 1997, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@azstarnet.com).

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
#include <errno.h>
#include <sys/syscall.h>
#include <signal.h>
#include <string.h>

/* When there is kernel support for more than 64 signals, we'll have to
   switch to a new system call convention here.  */

static __inline__ _syscall2(int, osf_sigprocmask, int, how, unsigned long int, setval)

int
sigprocmask (int how, const sigset_t *set, sigset_t *oset)
{
  unsigned long int setval;
  long result;

  if (set)
    setval = set->__val[0];
  else
    {
      setval = 0;
      how = SIG_BLOCK;	/* ensure blocked mask doesn't get changed */
    }

  result = osf_sigprocmask(how, setval);
  if (result == -1)
    /* If there are ever more than 64 signals, we need to recode this
       in assembler since we wouldn't be able to distinguish a mask of
       all 1s from -1, but for now, we're doing just fine... */
    return result;

  if (oset)
    {
      if (_SIGSET_NWORDS == 2) /* typical */
        oset->__val[1] = 0;
      if (_SIGSET_NWORDS > 2)
        memset(oset, 0, sizeof(*oset));
      oset->__val[0] = result;
    }
  return 0;
}
libc_hidden_def(sigprocmask)
