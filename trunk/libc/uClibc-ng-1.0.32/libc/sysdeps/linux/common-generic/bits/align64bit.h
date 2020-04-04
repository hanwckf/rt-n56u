/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#ifndef _ALIGN_64_BIT_H
#define _ALIGN_64_BIT_H

/* Simple macro for getting the 64-bit struct arch alignment */

struct __longlong_aligned { long long x; };

#define __ARCH_64BIT_ALIGNMENT__ \
	__attribute__((aligned(__alignof__(struct __longlong_aligned))))

#endif /* bits/align64bit.h */
