/* Copyright (C) 2000 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf'.  HPPA version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* The previous bits/setjmp.h had __jmp_buf defined as a structure.
   We use an array of 'double' instead, to make writing the assembler
   easier, and to ensure proper alignment. Naturally, user code should
   not depend on either representation. */

#if defined __USE_MISC || defined _ASM
#define JB_SP (76/4)
#endif

#ifndef	_ASM
typedef double __jmp_buf[21];
#endif

/* Test if longjmp to JMPBUF would unwind the frame containing a local
   variable at ADDRESS.  */
#define _JMPBUF_UNWINDS(_jmpbuf, _address)				\
     ((void *)(_address) > (void *)(((unsigned long *) _jmpbuf)[JB_SP]))

#endif	/* bits/setjmp.h */
