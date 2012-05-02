/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"

#ifdef __DO_UNLOCKED

weak_alias(__fgets_unlocked,fgets_unlocked);
#ifndef __UCLIBC_HAS_THREADS__
weak_alias(__fgets_unlocked,fgets);
#endif


char *__fgets_unlocked(char *__restrict s, int n,
					   register FILE * __restrict stream)
{
	register char *p;
	int c;

	__STDIO_STREAM_VALIDATE(stream);

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: What should fgets do if n <= 0?
#endif /* __UCLIBC_MJN3_ONLY__ */
	/* Should we assert here?  Or set errno?  Or just fail... */
	if (n <= 0) {
/* 		__set_errno(EINVAL); */
		goto ERROR;
	}

	p = s;

	while (--n) {
		if (__STDIO_STREAM_CAN_USE_BUFFER_GET(stream)) {
			if ((*p++ = __STDIO_STREAM_BUFFER_GET(stream)) == '\n') {
				break;
			}
		} else {
			if ((c = __fgetc_unlocked(stream)) == EOF) {
				if (__FERROR_UNLOCKED(stream)) {
					goto ERROR;
				}
				break;
			}
			if ((*p++ = c) == '\n') {
				break;
			}
		}
	}

#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: If n==1 and not at EOF, should fgets return an empty string?
#endif /* __UCLIBC_MJN3_ONLY__ */
	if (p > s) {
		*p = 0;
		return s;
	}

 ERROR:
	return NULL;
}

#elif defined __UCLIBC_HAS_THREADS__

char *fgets(char *__restrict s, int n,
			register FILE * __restrict stream)
{
	char *retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = __fgets_unlocked(s, n, stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}

#endif
