/* Copyright (C) 2004       Manuel Novoa III    <mjn3@codepoet.org>
 *
 * GNU Library General Public License (LGPL) version 2 or later.
 *
 * Dedicated to Toni.  See uClibc/DEDICATION.mjn3 for details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <err.h>
#ifdef __UCLIBC_HAS_THREADS__
#include <pthread.h>
#endif

#ifdef __UCLIBC_MJN3_ONLY__
#warning REMINDER: Deal with wide oriented stderr case.
#endif

#if defined __USE_BSD



static void vwarn_work(const char *format, va_list args, int showerr)
{
	/*                         0123 45678 9 a b*/
	static const char fmt[] = "%s: \0: %s\n\0\n";
	const char *f;
	char buf[64];
	__STDIO_AUTO_THREADLOCK_VAR;

	/* Do this first, in case something below changes errno. */
	f = fmt + 11;				/* At 11. */
	if (showerr) {
		f -= 4;					/* At 7. */
		__xpg_strerror_r(errno, buf, sizeof(buf));
	}

	__STDIO_AUTO_THREADLOCK(stderr);

	fprintf(stderr, fmt, __uclibc_progname);
	if (format) {
		vfprintf(stderr, format, args);
		f -= 2;					/* At 5 (showerr) or 9. */
	}
	fprintf(stderr, f, buf);

	__STDIO_AUTO_THREADUNLOCK(stderr);
}

void vwarn(const char *format, va_list args)
{
	vwarn_work(format, args, 1);
}
libc_hidden_def(vwarn)

void warn(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vwarn(format, args);
	va_end(args);
}

void vwarnx(const char *format, va_list args)
{
	vwarn_work(format, args, 0);
}
libc_hidden_def(vwarnx)

void warnx(const char *format, ...)
{
	va_list args;

	va_start(args, format);
	vwarnx(format, args);
	va_end(args);
}

void verr(int status, const char *format, va_list args)
{
	vwarn(format, args);
	exit(status);
}
libc_hidden_def(verr)

void attribute_noreturn err(int status, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	verr(status, format, args);
	/* This should get optimized away.  We'll leave it now for safety. */
	/* The loop is added only to keep gcc happy. */
	while(1)
		va_end(args);
}

void verrx(int status, const char *format, va_list args)
{
	vwarnx(format, args);
	exit(status);
}
libc_hidden_def(verrx)

void attribute_noreturn errx(int status, const char *format, ...)
{
	va_list args;

	va_start(args, format);
	verrx(status, format, args);
	/* This should get optimized away.  We'll leave it now for safety. */
	/* The loop is added only to keep gcc happy. */
	while(1)
		va_end(args);
}
#endif
