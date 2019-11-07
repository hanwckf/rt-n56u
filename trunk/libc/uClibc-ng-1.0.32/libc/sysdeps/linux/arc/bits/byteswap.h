/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 *
 */

#ifndef _ASM_BITS_BYTESWAP_H
#define _ASM_BITS_BYTESWAP_H 1

#ifdef __Xswape	/* gcc defined if -mswape is enabled */

#define __bswap_non_constant_32(x)			\
	__extension__ 					\
	({ unsigned int __bswap_32_v = x; 		\
	__asm__ ("swape %0, %0" : "+r" (__bswap_32_v));	\
	__bswap_32_v; })

#endif	/* __Xswape */

#endif	/* _ASM_BITS_BYTESWAP_H */

#include <bits/byteswap-common.h>
