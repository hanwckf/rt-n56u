/*
 * libc/sysdeps/linux/microblaze/bits/setjmp.h -- microblaze version of `jmp_buf' type
 *
 *  Copyright (C) 2003  John Williams <jwilliams@itee.uq.edu.au>
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 *
 * Written by Miles Bader <miles@gnu.org>
 */

#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

typedef struct
  {
    /* Stack pointer.  */
    void *__sp;

    /* Link pointer.  */
    void *__lp;

    /* SDA pointers */
    void *__SDA;
    void *__SDA2;

    /* Callee-saved registers r18-r31.  */
    int __regs[14];
  } __jmp_buf[1];

#endif	/* bits/setjmp.h */
