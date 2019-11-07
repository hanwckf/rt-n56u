/*
 * Copyright (C) 2002     Manuel Novoa III
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* OpenBSD function:
 * Append at most n-1-strlen(dst) chars from src to dst and nul-terminate dst.
 * Returns strlen(src) + strlen({original} dst), so truncation occurred if the
 * return val is >= n.
 * Note: If dst doesn't contain a nul in the first n chars, strlen(dst) is
 *       taken as n. */

#include "_string.h"

size_t strlcat(register char *__restrict dst,
			   register const char *__restrict src,
			   size_t n)
{
	size_t len;
	char dummy[1];

	len = 0;

	while (1) {
		if (len >= n) {
			dst = dummy;
			break;
		}
		if (!*dst) {
			break;
		}
		++dst;
		++len;
	}

	while ((*dst = *src) != 0) {
		if (++len < n) {
			++dst;
		}
		++src;
	}

	return len;
}
libc_hidden_def(strlcat)
