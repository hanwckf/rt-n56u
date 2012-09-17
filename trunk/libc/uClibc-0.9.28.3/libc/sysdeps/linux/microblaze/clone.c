/*
 * libc/sysdeps/linux/microblaze/clone.c -- `clone' syscall for linux/microblaze
 *
 *  Copyright (C) 2003     John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2002,03  NEC Electronics Corporation
 *  Copyright (C) 2002,03  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 * Microblaze port by John Williams
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
      asm volatile ("bralid r17, trap;nop;" 
		    : "=r" (rval), "=r" (syscall)
		    : "1" (syscall), "r" (arg0), "r" (arg1)
		    : SYSCALL_CLOBBERS);

      if (rval == 0)
	/* In child thread, call FN and exit.  */
	{
	  arg0 = (*fn) (arg);
	  syscall = __NR_exit;
	  asm volatile ("bralid r17, trap;nop;" 
			: "=r" (rval), "=r" (syscall)
			: "1" (syscall), "r" (arg0)
			: SYSCALL_CLOBBERS);
	}
    }

  __syscall_return (int, rval);
}
