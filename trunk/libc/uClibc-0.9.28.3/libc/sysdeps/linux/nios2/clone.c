/*
 * libc/sysdeps/linux/nios2/clone.c -- `clone' syscall for linux/nios2
 *
 *  Copyright (C) 2004,05  Microtronix Datacom Ltd
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Nios2 port by Wentao Xu
 */

#include <errno.h>
#include <sys/syscall.h>

int clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg)
{
  register unsigned long rval asm ("r2") = -EINVAL;

  if (fn && child_stack) {
      register unsigned long syscall asm ("r3");
      register unsigned long arg0 asm ("r4");
      register unsigned long arg1 asm ("r5");

      /* Clone this thread.  */
      rval = TRAP_ID_SYSCALL;
      syscall = __NR_clone;
      arg0 = flags;
      arg1 = (unsigned long)child_stack;
      asm volatile ("trap "
         : "=r" (rval), "=r" (syscall)
         : "0" (rval),"1" (syscall), "r" (arg0), "r" (arg1)
         );

      if (rval == 0) {
         /* In child thread, call fn and exit.  */
         arg0 = (*fn) (arg);
         syscall = __NR_exit;
         asm volatile ("trap "
          : "=r" (rval), "=r" (syscall)
          : "1" (syscall), "r" (arg0));
      }
   }

  __syscall_return (int, rval);
}
