/* Macros to swap the order of bytes in integer values.  m68k version.
   Copyright (C) 1997, 2002 Free Software Foundation, Inc.
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

#if !defined __mcoldfire__
# define __bswap_non_constant_32(x) \
  __extension__							\
  ({ unsigned int __bswap_32_v;					\
     __asm__ __volatile__ ("ror%.w %#8, %0;"			\
			   "swap %0;"				\
			   "ror%.w %#8, %0"			\
			   : "=d" (__bswap_32_v)		\
			   : "0" ((unsigned int) (x)));		\
     __bswap_32_v; })
#endif

#endif

#include <bits/byteswap-common.h>
