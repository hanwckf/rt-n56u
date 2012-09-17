/* Macros to swap the order of bytes in integer values.  H8/300 version.
   Copyright (C) 1997 Free Software Foundation, Inc.
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

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#define __bswap_non_constant_32(x) \
  __extension__						\
  ({ unsigned int __v;					\
     __asm__ __volatile__ ("mov.l %0,er0\n\t"		\
			   "mov.b r0l,r1h\n\t"		\
			   "mov.b r0h,r1l\n\t"		\
			   "mov.w r1,e1\n\t"		\
			   "mov.w e0,r0\n\t"		\
			   "mov.b r0l,r1h\n\t"		\
			   "mov.b r0h,r1l\n\t"		\
			   "mov.l er1,%0"		\
			   : "=d" (__v)			\
			   : "0" (x): "er0","er1");	\
     __v; })

#endif

#include <bits/byteswap-common.h>
