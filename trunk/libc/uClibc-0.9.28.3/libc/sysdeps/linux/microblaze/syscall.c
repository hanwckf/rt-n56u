/*
 * libc/sysdeps/linux/microblaze/syscall.c -- generic syscall function for linux/microblaze
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2002  NEC Corporation
 *  Copyright (C) 2002  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 * 
 * Written by Miles Bader <miles@gnu.org>
 */

#include <errno.h>
#include <sys/syscall.h>

typedef unsigned long arg_t;

/* Invoke `system call' NUM, passing it the remaining arguments.
   This is completely system-dependent, and not often useful.  */
long
syscall (long num, arg_t a1, arg_t a2, arg_t a3, arg_t a4, arg_t a5, arg_t a6)
{
  /* We don't know how many arguments are valid, so A5 and A6 are fetched
     off the stack even for (the majority of) system calls with fewer
     arguments; hopefully this won't cause any problems.  A1-A4 are in
     registers, so they're OK.  */
  register arg_t a asm (SYSCALL_ARG0) = a1;
  register arg_t b asm (SYSCALL_ARG1) = a2;
  register arg_t c asm (SYSCALL_ARG2) = a3;
  register arg_t d asm (SYSCALL_ARG3) = a4;
  register arg_t e asm (SYSCALL_ARG4) = a5;
  register arg_t f asm (SYSCALL_ARG5) = a6;
  register unsigned long syscall asm (SYSCALL_NUM) = num;
  register unsigned long ret asm (SYSCALL_RET);
	unsigned long ret_sav;

  *((unsigned long *)0xFFFF4004) = (unsigned int)('+');
  asm ("brlid r17, 08x; nop;" 
       : "=r" (ret)
       : "r" (syscall), "r" (a), "r" (b), "r" (c), "r" (d), "r" (e), "r" (f)
       : SYSCALL_CLOBBERS);
	
  ret_sav=ret;
  *((unsigned long *)0xFFFF4004) = (unsigned int)('-');



  __syscall_return (long, ret);
}
