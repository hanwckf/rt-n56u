
/*  Copyright (C) 2002, 2003, 2004     Manuel Novoa III
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  Besides uClibc, I'm using this code in my libc for elks, which is
 *  a 16-bit environment with a fairly limited compiler.  It would make
 *  things much easier for me if this file isn't modified unnecessarily.
 *  In particular, please put any new or replacement functions somewhere
 *  else, and modify the makefile to use your version instead.
 *  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */


/* May 23, 2002     Initial Notes:
 *
 * I'm still tweaking this stuff, but it passes the tests I've thrown
 * at it, and Erik needs it for the gcc port.  The glibc extension
 * __wcsnrtombs() hasn't been tested, as I didn't find a test for it
 * in the glibc source.  I also need to fix the behavior of
 * _wchar_utf8sntowcs() if the max number of wchars to convert is 0.
 *
 * UTF-8 -> wchar -> UTF-8 conversion tests on Markus Kuhn's UTF-8-demo.txt
 * file on my platform (x86) show about 5-10% faster conversion speed than
 * glibc with mbsrtowcs()/wcsrtombs() and almost twice as fast as glibc with
 * individual mbrtowc()/wcrtomb() calls.
 *
 * If 'DECODER' is defined, then _wchar_utf8sntowcs() will be compiled
 * as a fail-safe UTF-8 decoder appropriate for a terminal, etc.  which
 * needs to deal gracefully with whatever is sent to it.  In that mode,
 * it passes Markus Kuhn's UTF-8-test.txt stress test.  I plan to add
 * an arg to force that behavior, so the interface will be changing.
 *
 * I need to fix the error checking for 16-bit wide chars.  This isn't
 * an issue for uClibc, but may be for ELKS.  I'm currently not sure
 * if I'll use 16-bit, 32-bit, or configureable wchars in ELKS.
 *
 * July 1, 2002
 *
 * Fixed _wchar_utf8sntowcs() for the max number of wchars == 0 case.
 * Fixed nul-char bug in btowc(), and another in __mbsnrtowcs() for 8-bit
 *    locales.
 * Enabled building of a C/POSIX-locale-only version, so full locale support
 *    no longer needs to be enabled.
 *
 * Nov 4, 2002
 *
 * Fixed a bug in _wchar_wcsntoutf8s().  Don't store wcs position if dst is NULL.
 * Also, introduce an awful hack into _wchar_wcsntoutf8s() and wcsrtombs() in
 *   order to support %ls in printf.  See comments below for details.
 * Change behaviour of wc<->mb functions when in the C locale.  Now they do
 *   a 1-1 map for the range 0x80-UCHAR_MAX.  This is for backwards compatibility
 *   and consistency with the stds requirements that a printf format string by
 *   a valid multibyte string beginning and ending in it's initial shift state.
 *
 * Nov 5, 2002
 *
 * Forgot to change btowc and wctob when I changed the wc<->mb functions yesterday.
 *
 * Nov 7, 2002
 *
 * Add wcwidth and wcswidth, based on Markus Kuhn's wcwidth of 2002-05-08.
 *   Added some size/speed optimizations and integrated it into my locale
 *   framework.  Minimally tested at the moment, but the stub C-locale
 *   version (which most people would probably be using) should be fine.
 *
 * Nov 21, 2002
 *
 * Revert the wc<->mb changes from earlier this month involving the C-locale.
 * Add a couple of ugly hacks to support *wprintf.
 * Add a mini iconv() and iconv implementation (requires locale support).
 *
 * Aug 1, 2003
 * Bug fix for mbrtowc.
 *
 * Aug 18, 2003
 * Bug fix: _wchar_utf8sntowcs and _wchar_wcsntoutf8s now set errno if EILSEQ.
 *
 * Feb 11, 2004
 * Bug fix: Fix size check for remaining output space in iconv().
 *
 * Manuel
 */

#include "porting.h"
#include <string.h>
#include <iconv.h>
#include <stdarg.h>
#include <libgen.h>
#include <wchar.h>
#include "wchar.c" /* for _UC_iconv_t and __iconv_codesets */

extern const unsigned char __iconv_codesets[];

#define IBUF BUFSIZ
#define OBUF BUFSIZ

static char *progname;
static int hide_errors;

static void error_msg(const char *fmt, ...)
	 __attribute__ ((noreturn, format (printf, 1, 2)));

static void error_msg(const char *fmt, ...)
{
	va_list arg;

	if (!hide_errors) {
		fprintf(stderr, "%s: ", progname);
		va_start(arg, fmt);
		vfprintf(stderr, fmt, arg);
		va_end(arg);
	}

	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	FILE *ifile;
	FILE *ofile = stdout;
	const char *p;
	const char *s;
	static const char opt_chars[] = "tfocsl";
	                              /* 012345 */
	const char *opts[sizeof(opt_chars)]; /* last is infile name */
	iconv_t ic;
	char ibuf[IBUF];
	char obuf[OBUF];
	char *pi;
	char *po;
	size_t ni, no, r, pos;

	hide_errors = 0;

	for (s = opt_chars ; *s ; s++) {
		opts[ s - opt_chars ] = NULL;
	}

	progname = *argv;
	while (--argc) {
		p = *++argv;
		if ((*p != '-') || (*++p == 0)) {
			break;
		}
		do {
			if ((s = strchr(opt_chars,*p)) == NULL) {
			USAGE:
				s = basename(progname);
				fprintf(stderr,
						"%s [-cs] -f fromcode -t tocode [-o outputfile] [inputfile ...]\n"
						"  or\n%s -l\n", s, s);
				return EXIT_FAILURE;
			}
			if ((s - opt_chars) < 3) {
				if ((--argc == 0) || opts[s - opt_chars]) {
					goto USAGE;
				}
				opts[s - opt_chars] = *++argv;
			} else {
				opts[s - opt_chars] = p;
			}
		} while (*++p);
	}

	if (opts[5]) {				/* -l */
		fprintf(stderr, "Recognized codesets:\n");
		for (s = (char *)__iconv_codesets ; *s ; s += *s) {
			fprintf(stderr,"  %s\n", s+2);
		}
		s = __LOCALE_DATA_CODESET_LIST;
		do {
			fprintf(stderr,"  %s\n", __LOCALE_DATA_CODESET_LIST+ (unsigned char)(*s));
		} while (*++s);

		return EXIT_SUCCESS;
	}

	if (opts[4]) {
		hide_errors = 1;
	}

	if (!opts[0] || !opts[1]) {
		goto USAGE;
	}
	if ((ic = iconv_open(opts[0],opts[1])) == ((iconv_t)(-1))) {
		error_msg( "unsupported codeset in %s -> %s conversion\n", opts[1], opts[0]);
	}
	if (opts[3]) {				/* -c */
		((_UC_iconv_t *) ic)->skip_invalid_input = 1;
	}

	if ((s = opts[2]) != NULL) {
		if (!(ofile = fopen(s, "w"))) {
			error_msg( "couldn't open %s for writing\n", s);
		}
	}

	pos = ni = 0;
	do {
		if (!argc || ((**argv == '-') && !((*argv)[1]))) {
			ifile = stdin;		/* we don't check for duplicates */
		} else if (!(ifile = fopen(*argv, "r"))) {
			error_msg( "couldn't open %s for reading\n", *argv);
		}

		while ((r = fread(ibuf + ni, 1, IBUF - ni, ifile)) > 0) {
			pos += r;
			ni += r;
			no = OBUF;
			pi = ibuf;
			po = obuf;
			if ((r = iconv(ic, &pi, &ni, &po, &no)) == ((size_t)(-1))) {
				if ((errno != EINVAL) && (errno != E2BIG)) {
					error_msg( "iconv failed at pos %lu : %m\n", (unsigned long) (pos - ni));
				}
			}
			if ((r = OBUF - no) > 0) {
				if (fwrite(obuf, 1, OBUF - no, ofile) < r) {
					error_msg( "write error\n");
				}
			}
			if (ni) {			/* still bytes in buffer! */
				memmove(ibuf, pi, ni);
			}
		}

		if (ferror(ifile)) {
			error_msg( "read error\n");
		}

		++argv;

		if (ifile != stdin) {
			fclose(ifile);
		}

	} while (--argc > 0);

	iconv_close(ic);

	if (ni) {
		error_msg( "incomplete sequence\n");
	}

	return (((_UC_iconv_t *) ic)->skip_invalid_input < 2)
		? EXIT_SUCCESS : EXIT_FAILURE;
}
