/*
 * libc/sysdeps/linux/v850/bits/setjmp.h -- v850 version of `jmp_buf' type
 *
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

#ifndef _ASM
typedef struct
  {
    /* Stack pointer.  */
    void *__sp;

    /* Link pointer.  */
    void *__lp;

    /* Callee-saved registers r2 and r20-r29.  */
    int __regs[11];
  } __jmp_buf[1];
#endif

#define JB_SIZE		(4 * 13)

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[0].__sp)

#endif	/* bits/setjmp.h */
