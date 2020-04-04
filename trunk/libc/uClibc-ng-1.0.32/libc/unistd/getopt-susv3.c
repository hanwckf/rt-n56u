/*  Copyright (C) 2003     Manuel Novoa III
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
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

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <getopt.h>

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
libc_hidden_def(getopt)
