/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <limits.h>
#include <signal.h>
#include <sysdep.h>
#include <internal-signals.h>

int
raise (int sig)
{
  /* rt_sigprocmask may fail if:

     1. sigsetsize != sizeof (sigset_t) (EINVAL)
     2. a failure in copy from/to user space (EFAULT)
     3. an invalid 'how' operation (EINVAL)

     The first case is already handle in glibc syscall call by using the arch
     defined _NSIG.  Second case is handled by using a stack allocated mask.
     The last one should be handled by the block/unblock functions.  */

  sigset_t set;
  __libc_signal_block_app (&set);

  INTERNAL_SYSCALL_DECL (err);
  pid_t pid = INTERNAL_SYSCALL (getpid, err, 0);
  pid_t tid = INTERNAL_SYSCALL (gettid, err, 0);

  int ret = INLINE_SYSCALL (tgkill, 3, pid, tid, sig);

  __libc_signal_restore_set (&set);

  return ret;
}
libc_hidden_def (raise)
