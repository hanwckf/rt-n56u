/*  Copyright (C) 2003     Manuel Novoa III
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

/* Sep 7, 2003
 *   Initial version of a SUSv3 compliant getopt().
 */

#define _GNU_SOURCE
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifdef __UCLIBC_MJN3_ONLY__
#warning TODO: Enable gettext awareness.
#endif /* __UCLIBC_MJN3_ONLY__ */

#undef _
#define _(X)   X

#ifdef __BCC__
static const char missing[] = "option requires an argument";
static const char illegal[] = "illegal option";
#else
static const char missing[] = "%s: option requires an argument -- %c\n";
static const char illegal[] = "%s: illegal option -- %c\n";
#endif

int opterr = 1;
int optind = 1;
int optopt = 0;
char *optarg = NULL;

int getopt(int argc, char * const argv[], const char *optstring)
{
	static const char *o;		/* multi opt position */
	register const char *p;
	register const char *s;
	int retval = -1;

	optopt = 0;
	optarg = NULL;

	if (!o) {				/* Not in a multi-option arg. */
		if ((optind >= argc)	/* No more args? */
			|| ((p = argv[optind]) == NULL) /* Missing? */
			|| (*p != '-')		/* Not an option? */
			|| (!*++p)			/* "-" case? */
			) {
			goto DONE;
		}
		if ((*p == '-') && (p[1] == 0)) { /* "--" case. */
/* 			++optind; */
/* 			goto DONE; */
			goto NEXTOPT;		/* Less code generated... */
		}
		o = p;
	}

#ifdef __BCC__
	p = o;						/* Sigh... Help out bcc. */
#define o p
#endif
	retval = (unsigned char) *o; /* Avoid problems for char val of -1. */

	if ((*o == ':') || !(s = strchr(optstring, *o))) { /* Illegal option? */
		s = illegal;
		retval = '?';
		goto BAD;
	}
	
	if (s[1] == ':') {			/* Option takes an arg? */
		if (o[1]) {					/* No space between option and arg? */
			optarg = (char *)(o + 1);
			goto NEXTOPT;
		}

		if (optind + 1 < argc) {	/* Space between option and arg? */
			optarg = argv[++optind];
		} else {				/* Out of args! */
			s = missing;
			retval = ':';
		BAD:
			optopt = *o;
			if (*optstring != ':') {
				retval = '?';
				if (opterr) {
#ifdef __BCC__
					fprintf(stderr, "%s: %s -- %c\n", argv[0], s, *o);
#else
					fprintf(stderr, _(s), argv[0], *o);
#endif
				}
			}
		}
	}

#ifdef __BCC__
#undef o
#endif

	if (!*++o) {
	NEXTOPT:
		o = NULL;
		++optind;
	}
 DONE:
	return retval;
}
