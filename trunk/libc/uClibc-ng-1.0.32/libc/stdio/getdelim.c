/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <features.h>

#ifdef __USE_GNU
#include "_stdio.h"



/* Note: There is a defect in this function.  (size_t vs ssize_t). */

/* glibc function --
 * Return -1 if error or EOF prior to any chars read.
 * Return number of chars read (including possible delimiter but not
 *   the terminating nul) otherwise.
 *
 * NOTE: If we need to allocate a buffer, we do so prior to attempting
 * a reading.  So space may be allocated even if initially at EOF.
 */

#define GETDELIM_GROWBY		64

ssize_t getdelim(char **__restrict lineptr, size_t *__restrict n,
				   int delimiter, register FILE *__restrict stream)
{
	register char *buf;
	ssize_t pos = -1;
	int c;
	__STDIO_AUTO_THREADLOCK_VAR;

	if (!lineptr || !n || !stream) { /* Be compatable with glibc... even */
		__set_errno(EINVAL); /* though I think we should assert here */
	} else {
		__STDIO_AUTO_THREADLOCK(stream);

		if (!(buf = *lineptr)) { /* If passed NULL for buffer, */
			*n = 0;		/* ignore value passed and treat size as 0. */
		}

		/* Within the loop, pos is actually the current buffer index + 2,
		 * because we want to make sure we have enough space to store
		 * an additional char plus a nul terminator.
		 */
		pos = 1;

		do {
			if (pos >= *n) {
				if (!(buf = realloc(buf, *n + GETDELIM_GROWBY))) {
					pos = -1;
					break;
				}
				*n += GETDELIM_GROWBY;
				*lineptr = buf;
			}

			if ((c = __GETC_UNLOCKED(stream)) != EOF) {
				buf[++pos - 2] = c;
				if (c != delimiter) {
					continue;
				}
			}

			/* We're done, so correct pos back to being the current index. */
			if ((pos -= 2) >= 0) {
				buf[++pos] = 0;
			}
			break;

		} while (1);

		__STDIO_AUTO_THREADUNLOCK(stream);
	}

	return pos;
}
libc_hidden_def(getdelim)
#endif
