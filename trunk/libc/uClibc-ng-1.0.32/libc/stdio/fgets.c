/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#ifdef __DO_UNLOCKED


char *fgets_unlocked(char *__restrict s, int n,
					   register FILE * __restrict stream)
{
	register char *p;
	int c;

	__STDIO_STREAM_VALIDATE(stream);

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

	if (p > s) {
		*p = 0;
		return s;
	}

 ERROR:
	return NULL;
}
libc_hidden_def(fgets_unlocked)

#ifndef __UCLIBC_HAS_THREADS__
strong_alias(fgets_unlocked,fgets)
libc_hidden_def(fgets)
#endif

#elif defined __UCLIBC_HAS_THREADS__

char *fgets(char *__restrict s, int n,
			register FILE * __restrict stream)
{
	char *retval;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	retval = fgets_unlocked(s, n, stream);

	__STDIO_AUTO_THREADUNLOCK(stream);

	return retval;
}
libc_hidden_def(fgets)

#endif
