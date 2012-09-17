/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

/* Define the machine-dependent type `jmp_buf'.  Nios version.  */

#ifndef _SETJMP_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#ifndef _ASM

#include <signal.h>

typedef struct
  {
    /* There are eight 4-byte local registers saved.  */
    long int __lregs[8];

    /* There are six 4-byte input registers saved.  */
    long int __iregs[6];

    /* The SP, return address to caller (also for longjmp)
       and return address of caller are saved.  */
    int *__sp;
    int *__jmpret;
    int *__callersret;

  } __jmp_buf[1];

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)->__sp)

#else /* _ASM */

#define jmpbuf_l0 0x00
#define jmpbuf_l1 0x01
#define jmpbuf_l2 0x02
#define jmpbuf_l3 0x03
#define jmpbuf_l4 0x04
#define jmpbuf_l5 0x05
#define jmpbuf_l6 0x06
#define jmpbuf_l7 0x07

#define jmpbuf_i0 0x08
#define jmpbuf_i1 0x09
#define jmpbuf_i2 0x0a
#define jmpbuf_i3 0x0b
#define jmpbuf_i4 0x0c
#define jmpbuf_i5 0x0d

#define jmpbuf_sp 0x0e
#define jmpbuf_jmpret 0x0f
#define jmpbuf_callersret 0x10

#endif /* _ASM */


