/* Copyright (C) 1997, 1998, 2005 Free Software Foundation, Inc.
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

/* Define the machine-dependent type `jmp_buf', CRIS version.  */
#ifndef _BITS_SETJMP_H
#define _BITS_SETJMP_H	1

#if !defined _SETJMP_H && !defined _PTHREAD_H
# error "Never include <bits/setjmp.h> directly; use <setjmp.h> instead."
#endif

/* Note that saving and restoring CCR has no meaning in user mode, so we
   don't actually do it; the slot is now reserved.

   jmp_buf[0] - PC
   jmp_buf[1] - SP (R14)
   jmp_buf[2] - R13
   jmp_buf[3] - R12
   jmp_buf[4] - R11
   jmp_buf[5] - R10
   jmp_buf[6] - R9
   jmp_buf[7] - R8
   jmp_buf[8] - R7
   jmp_buf[9] - R6
   jmp_buf[10] - R5
   jmp_buf[11] - R4
   jmp_buf[12] - R3
   jmp_buf[13] - R2
   jmp_buf[14] - R1
   jmp_buf[15] - R0
   jmp_buf[16] - SRP
   jmp_buf[17] - CCR  */

typedef int __jmp_buf[18];

#endif	/* bits/setjmp.h */
