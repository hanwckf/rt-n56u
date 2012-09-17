/*
 * libc/sysdeps/linux/nios2/clone.c -- `clone' syscall for linux/nios2
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 *
 *
 *  Copyright (C) 2004,05  Microtronix Datacom Ltd
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * Written by Miles Bader <miles@gnu.org>
 * Nios2 port by Wentao Xu
 */

#include <errno.h>
#include <sched.h>
#include <sys/syscall.h>

int clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg, ...)
{
  register unsigned long rval __asm__ ("r2") = -EINVAL;

  if (fn && child_stack) {
      register unsigned long syscall __asm__ ("r3");
      register unsigned long arg0 __asm__ ("r4");
      register unsigned long arg1 __asm__ ("r5");

      /* Clone this thread.  */
      rval = TRAP_ID_SYSCALL;
      syscall = __NR_clone;
      arg0 = flags;
      arg1 = (unsigned long)child_stack;
      __asm__ __volatile__ ("trap "
         : "=r" (rval), "=r" (syscall)
         : "0" (rval),"1" (syscall), "r" (arg0), "r" (arg1)
         );

      if (rval == 0) {
         /* In child thread, call fn and exit.  */
         arg0 = (*fn) (arg);
         syscall = __NR_exit;
         __asm__ __volatile__ ("trap "
          : "=r" (rval), "=r" (syscall)
          : "1" (syscall), "r" (arg0));
      }
   }

  __syscall_return (int, rval);
}
