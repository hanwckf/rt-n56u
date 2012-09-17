/*
 * libc/sysdeps/linux/v850/bits/byteswap.h -- Macros to swap the order
 * 	of bytes in integer values
 *
 *  Copyright (C) 2001  NEC Corporation
 *  Copyright (C) 2001  Miles Bader <miles@gnu.org>
 *  Copyright (C) 1997,1998,2001  Free Software Foundation, Inc.
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License.  See the file COPYING.LIB in the main
 * directory of this archive for more details.
 */

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#define __bswap_non_constant_16(x) \
    (__extension__							      \
     ({ unsigned long int __bswap_16_v;					      \
	__asm__ ("bsh %1, %0" : "=r" (__bswap_16_v) : "r" (x));		      \
	__bswap_16_v; }))

# define __bswap_non_constant_32(x) \
    (__extension__							      \
     ({ unsigned long int __bswap_32_v;					      \
	__asm__ ("bsw %1, %0" : "=r" (__bswap_32_v) : "r" (x));		      \
	__bswap_32_v; }))

#endif

#include <bits/byteswap-common.h>
