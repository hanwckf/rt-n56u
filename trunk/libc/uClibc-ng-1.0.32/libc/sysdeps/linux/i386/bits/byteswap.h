/* Macros to swap the order of bytes in integer values.
   Copyright (C) 1997, 1998, 2000, 2002, 2003 Free Software Foundation, Inc.
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

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#define __bswap_non_constant_16(x) \
     (__extension__							      \
      ({ register unsigned short int __v;				      \
	 __asm__ ("rorw $8, %w0"					      \
		  : "=r" (__v)						      \
		  : "0" (x)						      \
		  : "cc");						      \
	 __v; }))

/* To swap the bytes in a word the i486 processors and up provide the
   `bswap' opcode.  On i386 we have to use three instructions.  */
#if !defined __i486__ && !defined __pentium__ && !defined __pentiumpro__ \
      && !defined __pentium4__
# define __bswap_non_constant_32(x) \
     (__extension__							      \
      ({ register unsigned int __v;					      \
	 __asm__ ("rorw $8, %w0;"					      \
		  "rorl $16, %0;"					      \
		  "rorw $8, %w0"					      \
		  : "=r" (__v)						      \
		  : "0" (x)						      \
		  : "cc");						      \
	 __v; }))
#else
# define __bswap_non_constant_32(x) \
     (__extension__							      \
      ({ register unsigned int __v;					      \
	 __asm__ ("bswap %0" : "=r" (__v) : "0" (x));			      \
	 __v; }))
#endif

#endif

#include <bits/byteswap-common.h>
