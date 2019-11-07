/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>

#ifdef __USE_GNU
#include "_stdio.h"


/* NOTE: GLIBC difference!!! -- fcloseall
 * According to the info pages, glibc actually fclose()s all open files.
 * Apparently, glibc's new version only fflush()s and unbuffers all
 * writing streams to cope with unordered destruction of c++ static
 * objects.
 */

int fcloseall (void)
{
#ifdef __STDIO_HAS_OPENLIST

	int retval = 0;
	FILE *f;

	__STDIO_OPENLIST_INC_USE;

	__STDIO_THREADLOCK_OPENLIST_ADD;
	f = _stdio_openlist;
	__STDIO_THREADUNLOCK_OPENLIST_ADD;

	while (f) {
		FILE *n = f->__nextopen;
		__STDIO_AUTO_THREADLOCK_VAR;

		__STDIO_AUTO_THREADLOCK(f);
		/* Only call fclose on the stream if it is not already closed. */
		if ((f->__modeflags & (__FLAG_READONLY|__FLAG_WRITEONLY))
		    != (__FLAG_READONLY|__FLAG_WRITEONLY)
		    ) {
			if (fclose(f)) {
				retval = EOF;
			}
		}
		__STDIO_AUTO_THREADUNLOCK(f);

		f = n;
	}

	__STDIO_OPENLIST_DEC_USE;

	return retval;

#else

#warning Always fails in this configuration because no open file list.

	return EOF;

#endif
}
#endif
