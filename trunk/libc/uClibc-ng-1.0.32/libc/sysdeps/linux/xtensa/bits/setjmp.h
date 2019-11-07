/* Copyright (C) 1997, 1998, 2007 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Define the machine-dependent type `jmp_buf'.  Xtensa version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

#if defined(__XTENSA_WINDOWED_ABI__)
/* The jmp_buf structure for Xtensa holds the following (where "proc"
   is the procedure that calls setjmp): 4-12 registers from the window
   of proc, the 4 words from the save area at proc's $sp (in case a
   subsequent alloca in proc moves $sp), and the return address within
   proc.  Everything else is saved on the stack in the normal save areas.  */

typedef int __jmp_buf[17];
#elif defined(__XTENSA_CALL0_ABI__)
/* The jmp_buf structure for Xtensa Call0 ABI holds the return address,
   the stack pointer and callee-saved registers (a12 - a15).  */

typedef int __jmp_buf[6];
#else
#error Unsupported Xtensa ABI
#endif

#endif	/* bits/setjmp.h */
