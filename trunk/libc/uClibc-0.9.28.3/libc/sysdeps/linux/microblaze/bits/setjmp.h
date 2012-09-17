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

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM
typedef struct
  {
    /* Stack pointer.  */
    void *__sp;

    /* Link pointer.  */
    void *__lp;

    /* Callee-saved registers r18-r30.  */
    int __regs[13];
  } __jmp_buf[1];
#endif

#define JB_SIZE		(4 * 15)

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((__ptr_t) (address) < &(jmpbuf)[0].__sp)
