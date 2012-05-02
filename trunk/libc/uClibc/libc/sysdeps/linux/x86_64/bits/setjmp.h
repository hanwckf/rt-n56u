/* Copyright (C) 2001, 2002, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* Define the machine-dependent type `jmp_buf'.  x86-64 version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H  1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#include <bits/wordsize.h>

#if __WORDSIZE == 64

/* We only need to save callee-saved registers plus stackpointer and
   program counter.  */
# if defined __USE_MISC || defined _ASM
#  define JB_RBX	0
#  define JB_RBP	1
#  define JB_R12	2
#  define JB_R13	3
#  define JB_R14	4
#  define JB_R15	5
#  define JB_RSP	6
#  define JB_PC	7
#  define JB_SIZE (8*8)
# endif

#else

# if defined __USE_MISC || defined _ASM
#  define JB_BX	0
#  define JB_SI	1
#  define JB_DI	2
#  define JB_BP	3
#  define JB_SP	4
#  define JB_PC	5
#  define JB_SIZE 24
# endif

#endif

#ifndef _ASM

# if __WORDSIZE == 64
typedef long int __jmp_buf[8];
# else
typedef int __jmp_buf[6];
# endif

/* Test if longjmp to JMPBUF would unwind the frame
   containing a local variable at ADDRESS.  */
# if __WORDSIZE == 64
#  define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[JB_RSP])
# else
#  define _JMPBUF_UNWINDS(jmpbuf, address) \
  ((void *) (address) < (void *) (jmpbuf)[JB_SP])
# endif
#endif

#endif  /* bits/setjmp.h */
