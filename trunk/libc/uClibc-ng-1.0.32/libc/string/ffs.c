/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <limits.h>
#include <string.h>
  
int ffs(int i)
{
#if 1
	/* inlined binary search method */
	char n = 1;
#if UINT_MAX == 0xffffU
	/* nothing to do here -- just trying to avoiding possible problems */
#elif UINT_MAX == 0xffffffffU
	if (!(i & 0xffff)) {
		n += 16;
		i >>= 16;
	}
#else
#error ffs needs rewriting!
#endif

	if (!(i & 0xff)) {
		n += 8;
		i >>= 8;
	}
	if (!(i & 0x0f)) {
		n += 4;
		i >>= 4;
	}
	if (!(i & 0x03)) {
		n += 2;
		i >>= 2;
	}
	return (i) ? (n + ((i+1) & 0x01)) : 0;

#else
	/* linear search -- slow, but small */
	int n;

	for (n = 0 ; i ; ++n) {
		i >>= 1;
	}

	return n;
#endif
}
libc_hidden_def(ffs)
#if ULONG_MAX == UINT_MAX
strong_alias_untyped(ffs, ffsl)
#endif
