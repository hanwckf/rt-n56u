#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

/* CRIS specific byte swap operations: 16, 32 and 64-bit */

#define __bswap_non_constant_16(x) \
	__extension__ 							\
	({ unsigned short __bswap_16_v; 				\
	   __asm__ ("swapb %0" : "=r" (__bswap_16_v) : "0" (x)); 	\
	   __bswap_16_v; })

#define __bswap_non_constant_32(x) \
	__extension__ 							\
	({ unsigned int __bswap_32_v; 					\
	   __asm__ ("swapwb %0" : "=r" (__bswap_32_v) : "0" (x)); 	\
	   __bswap_32_v; })

#endif

#include <bits/byteswap-common.h>
