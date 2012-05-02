/*
 * libc/sysdeps/linux/v850/clone.c -- `clone' syscall for linux/v850
 *
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#include <errno.h>
#include <sys/syscall.h>

int
clone (int (*fn)(void *arg), void *child_stack, int flags, void *arg)
{
  register unsigned long rval asm (SYSCALL_RET) = -EINVAL;

  if (fn && child_stack)
    {
      register unsigned long syscall asm (SYSCALL_NUM);
      register unsigned long arg0 asm (SYSCALL_ARG0);
      register unsigned long arg1 asm (SYSCALL_ARG1);

      /* Clone this thread.  */
      arg0 = flags;
      arg1 = (unsigned long)child_stack;
      syscall = __NR_clone;
      asm volatile ("trap " SYSCALL_SHORT_TRAP
		    : "=r" (rval), "=r" (syscall)
		    : "1" (syscall), "r" (arg0), "r" (arg1)
		    : SYSCALL_SHORT_CLOBBERS);

      if (rval == 0)
	/* In child thread, call FN and exit.  */
	{
	  arg0 = (*fn) (arg);
	  syscall = __NR_exit;
	  asm volatile ("trap " SYSCALL_SHORT_TRAP
			: "=r" (rval), "=r" (syscall)
			: "1" (syscall), "r" (arg0)
			: SYSCALL_SHORT_CLOBBERS);
	}
    }

  __syscall_return (int, rval);
}
