/* File: libc/sysdeps/linux/bfin/bits/byteswap.h
 *
 * Copyright 2004-2006 Analog Devices Inc.
 *
 * Enter bugs at http://blackfin.uclinux.org/
 *
 * Licensed under the GPL-2 or later.
 */

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#define __bswap_non_constant_16(x) \
     (__extension__							      \
      ({ register unsigned short int __v;				      \
	 __asm__ ("%0 = PACK (%1.L, %1.L);"				      \
		  "%0 >>= 8;"						      \
		  : "=d" (__v)						      \
		  : "d" (x));						      \
	 __v; }))

#define __bswap_non_constant_32(x) \
     (__extension__							      \
      ({ register unsigned int __v;					      \
	 __asm__ ("%1 = %0 >> 8 (V);"					      \
		  "%0 = %0 << 8 (V);"					      \
		  "%0 = %0 | %1;"					      \
		  "%1 = PACK(%0.L, %0.H);"				      \
		  : "+d"(x), "=&d"(__v));				      \
	 __v; }))

#endif

#include <bits/byteswap-common.h>
