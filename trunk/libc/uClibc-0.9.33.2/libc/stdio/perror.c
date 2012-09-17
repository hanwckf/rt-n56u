/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include "_stdio.h"


#ifdef __UCLIBC_MJN3_ONLY__
#warning CONSIDER: Increase buffer size for error message (non-%m case)?
#endif

void perror(register const char *s)
{
	/* If the program is calling perror, it's a safe bet that printf and
	 * friends are used as well.  It is also possible that the calling
	 * program could buffer stderr, or reassign it. */

	register const char *sep;

	sep = ": ";
	if (!(s && *s)) {			/* Caller did not supply a prefix message */
		s = (sep += 2);			/* or passed an empty string. */
	}

#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
	fprintf(stderr, "%s%s%m\n", s, sep); /* Use the gnu %m feature. */
#else
	{
		char buf[64];
		fprintf(stderr, "%s%s%s\n", s, sep,
				__glibc_strerror_r(errno, buf, sizeof(buf)));
	}
#endif
}
libc_hidden_def(perror)
