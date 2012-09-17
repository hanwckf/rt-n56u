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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#define __bswap_non_constant_16(x) \
     (__extension__							      \
      ({ register unsigned short int __v;				      \
	 __asm__ __volatile__ ("shl %0 = %1, 48 ;;"			      \
				"mux1 %0 = %0, @rev ;;"			      \
				: "=r" (__v)				      \
				: "r" ((unsigned short int) (x)));	      \
	 __v; }))

#define __bswap_non_constant_32(x) \
     (__extension__							      \
      ({ register unsigned int __v;					      \
	 __asm__ __volatile__ ("shl %0 = %1, 32 ;;"			      \
				"mux1 %0 = %0, @rev ;;"			      \
				: "=r" (__v)				      \
				: "r" ((unsigned int) (x)));		      \
	 __v; }))

#define __bswap_non_constant_64(x) \
     (__extension__							      \
      ({ register unsigned long int __v;				      \
	 __asm__ __volatile__ ("mux1 %0 = %1, @rev ;;"			      \
				: "=r" (__v)				      \
				: "r" ((unsigned long int) (x)));	      \
         __v; }))
#endif

#endif

#include <bits/byteswap-common.h>
