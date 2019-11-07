/*  Copyright (C) 2002-2004     Manuel Novoa III
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
 *  License along with this library; if not, see
 *  <http://www.gnu.org/licenses/>.
 */

/* Aug 1, 2003
 * New *scanf implementation with lots of bug fixes and *wscanf support.
 * Also now optionally supports hexadecimal float notation, positional
 * args, and glibc locale-specific digit grouping.  Should now be
 * standards compliant.
 *
 * Aug 18, 2003
 * Bug fix: scanf %lc,%ls,%l[ would always set mb_fail on eof or error,
 *   even when just starting a new mb char.
 * Bug fix: wscanf would incorrectly unget in certain situations.
 *
 * Sep 5, 2003
 * Bug fix: store flag wasn't respected if no positional args.
 * Implement vs{n}scanf for the non-buffered stdio no-wchar case.
 *
 * Sep 13, 2003
 * Bug fix: Fix a problem reported by Atsushi Nemoto <anemo@mba.ocn.ne.jp>
 * for environments where long and long long are the same.
 *
 * Sep 21, 2003
 * Ugh... EOF handling by scanf was completely broken.  :-(  Regretably,
 * I got my mind fixed in one mode and didn't comply with the standards.
 * Things should be fixed now, but comparision testing is difficult when
 * glibc's scanf is broken and they stubbornly refuse to even acknowledge
 * that it is... even when confronted by specific examples from the C99
 * standards and from an official C standard defect report.
 */

#include <features.h>
#include "_stdio.h"
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include <errno.h>
#include <printf.h>

#ifdef __UCLIBC_HAS_WCHAR__
#include <bits/uClibc_uwchar.h>
#include <wchar.h>
#include <wctype.h>
#endif /* __UCLIBC_HAS_WCHAR__ */

#include <langinfo.h>
#include <locale.h>

#include <assert.h>
#include <limits.h>

#ifdef __UCLIBC_HAS_THREADS__
#include <stdio_ext.h>
#include <pthread.h>
#endif /* __UCLIBC_HAS_THREADS__ */

#ifdef __UCLIBC_HAS_FLOATS__
#include <float.h>
#include <bits/uClibc_fpmax.h>
#endif /* __UCLIBC_HAS_FLOATS__ */

#undef __STDIO_HAS_VSSCANF
#if defined(__STDIO_BUFFERS) || !defined(__UCLIBC_HAS_WCHAR__) || defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)
#define __STDIO_HAS_VSSCANF 1

#if !defined(__STDIO_BUFFERS) && !defined(__UCLIBC_HAS_WCHAR__)
typedef struct {
	FILE f;
	unsigned char *bufread;		/* pointer to 1 past end of buffer */
	unsigned char *bufpos;
} __FILE_vsscanf;
#endif

#endif

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

extern unsigned long long
_stdlib_strto_ll(register const char * __restrict str,
				 char ** __restrict endptr, int base, int sflag);
#if (ULLONG_MAX == UINTMAX_MAX)
#define STRTOUIM(s,e,b,sf) _stdlib_strto_ll(s,e,b,sf)
#endif

#else  /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

extern unsigned long
_stdlib_strto_l(register const char * __restrict str,
				char ** __restrict endptr, int base, int sflag);

#if (ULONG_MAX == UINTMAX_MAX)
#define STRTOUIM(s,e,b,sf) _stdlib_strto_l(s,e,b,sf)
#endif

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#ifndef STRTOUIM
#error STRTOUIM conversion function is undefined!
#endif

/**********************************************************************/

/* The standards require EOF < 0. */
#if EOF >= CHAR_MIN
#define __isdigit_char_or_EOF(C)   __isdigit_char((C))
#else
#define __isdigit_char_or_EOF(C)   __isdigit_int((C))
#endif

/**********************************************************************/
#ifdef L_fscanf

int fscanf(FILE * __restrict stream, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfscanf(stream, format, arg);
	va_end(arg);

	return rv;
}
libc_hidden_def(fscanf)

#endif
/**********************************************************************/
#ifdef L_scanf

int scanf(const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfscanf(stdin, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_sscanf

#ifdef __STDIO_HAS_VSSCANF

int sscanf(const char * __restrict str, const char * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vsscanf(str, format, arg);
	va_end(arg);

	return rv;
}
libc_hidden_def(sscanf)

#else  /* __STDIO_HAS_VSSCANF */
#warning Skipping sscanf since no vsscanf!
#endif /* __STDIO_HAS_VSSCANF */

#endif
/**********************************************************************/
#ifdef L_vscanf

int vscanf(const char * __restrict format, va_list arg)
{
	return vfscanf(stdin, format, arg);
}

#endif
/**********************************************************************/
#ifdef L_vsscanf

#ifdef __STDIO_BUFFERS

int vsscanf(const char *sp, const char *fmt, va_list ap)
{
	FILE f;

	f.__filedes = __STDIO_STREAM_FAKE_VSSCANF_FILEDES;
	f.__modeflags = (__FLAG_NARROW|__FLAG_READONLY|__FLAG_READING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif

#ifdef __UCLIBC_HAS_THREADS__
	f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.__lock);
#endif
	f.__nextopen = NULL;

	/* Set these last since __bufgetc initialization depends on
	 * __user_locking and only gets set if user locking is on. */
	f.__bufstart =
	f.__bufpos = (unsigned char *) ((void *) sp);
	f.__bufread =
	f.__bufend = f.__bufstart + strlen(sp);
	__STDIO_STREAM_ENABLE_GETC(&f);
	__STDIO_STREAM_DISABLE_PUTC(&f);

	return vfscanf(&f, fmt, ap);
}
libc_hidden_def(vsscanf)

#elif !defined(__UCLIBC_HAS_WCHAR__)

int vsscanf(const char *sp, const char *fmt, va_list ap)
{
	__FILE_vsscanf f;

	f.bufpos = (unsigned char *) ((void *) sp);
	f.bufread = f.bufpos + strlen(sp);

	f.f.__filedes = __STDIO_STREAM_FAKE_VSSCANF_FILEDES_NB;
	f.f.__modeflags = (__FLAG_NARROW|__FLAG_READONLY|__FLAG_READING);

/* #ifdef __UCLIBC_HAS_WCHAR__ */
/* 	f.f.__ungot_width[0] = 0; */
/* #endif */
#ifdef __STDIO_MBSTATE
#error __STDIO_MBSTATE is defined!
/* 	__INIT_MBSTATE(&(f.f.__state)); */
#endif

#ifdef __UCLIBC_HAS_THREADS__
	f.f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.f.__lock);
#endif
	f.f.__nextopen = NULL;

	return vfscanf(&f.f, fmt, ap);
}
libc_hidden_def(vsscanf)

#elif defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)

int vsscanf(const char *sp, const char *fmt, va_list ap)
{
	FILE *f;
	int rv = EOF;

	if ((f = fmemopen((char *)sp, strlen(sp), "r")) != NULL) {
		rv = vfscanf(f, fmt, ap);
		fclose(f);
	}

	return rv;
}
libc_hidden_def(vsscanf)

#else
#warning Skipping vsscanf since no buffering, no custom streams, and wchar enabled!
#ifdef __STDIO_HAS_VSSCANF
#error WHOA! __STDIO_HAS_VSSCANF is defined!
#endif
#endif

#endif
/**********************************************************************/
#ifdef L_fwscanf

int fwscanf(FILE * __restrict stream, const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfwscanf(stream, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_wscanf

int wscanf(const wchar_t * __restrict format, ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vfwscanf(stdin, format, arg);
	va_end(arg);

	return rv;
}

#endif
/**********************************************************************/
#ifdef L_swscanf

#ifdef __STDIO_BUFFERS

int swscanf(const wchar_t * __restrict str, const wchar_t * __restrict format,
		   ...)
{
	va_list arg;
	int rv;

	va_start(arg, format);
	rv = vswscanf(str, format, arg);
	va_end(arg);

	return rv;
}
#else  /* __STDIO_BUFFERS */
#warning Skipping swscanf since no buffering!
#endif /* __STDIO_BUFFERS */

#endif
/**********************************************************************/
#ifdef L_vwscanf

int vwscanf(const wchar_t * __restrict format, va_list arg)
{
	return vfwscanf(stdin, format, arg);
}

#endif
/**********************************************************************/
#ifdef L_vswscanf

#ifdef __STDIO_BUFFERS

int vswscanf(const wchar_t * __restrict str, const wchar_t * __restrict format,
			va_list arg)
{
	FILE f;

	f.__bufstart =
	f.__bufpos = (unsigned char *) str;
	f.__bufread =
	f.__bufend = (unsigned char *)(str + wcslen(str));
	__STDIO_STREAM_DISABLE_GETC(&f);
	__STDIO_STREAM_DISABLE_PUTC(&f);

	f.__filedes = __STDIO_STREAM_FAKE_VSWSCANF_FILEDES;
	f.__modeflags = (__FLAG_WIDE|__FLAG_READONLY|__FLAG_READING);

#ifdef __UCLIBC_HAS_WCHAR__
	f.__ungot_width[0] = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */
#ifdef __STDIO_MBSTATE
	__INIT_MBSTATE(&(f.__state));
#endif /* __STDIO_MBSTATE */

#ifdef __UCLIBC_HAS_THREADS__
	f.__user_locking = 1;		/* Set user locking. */
	STDIO_INIT_MUTEX(f.__lock);
#endif
	f.__nextopen = NULL;

	return vfwscanf(&f, format, arg);
}
libc_hidden_def(vswscanf)
#else  /* __STDIO_BUFFERS */
#warning Skipping vswscanf since no buffering!
#endif /* __STDIO_BUFFERS */

#endif
/**********************************************************************/
/**********************************************************************/



/* float layout          0123456789012345678901  repeat n for "l[" */
#define SPEC_CHARS		"npxXoudifFeEgGaACSnmcs["
/*                       npxXoudif eEgG  CS  cs[ */
/* NOTE: the 'm' flag must come before any convs that support it */

/* NOTE: Ordering is important!  The CONV_{C,S,LEFTBRACKET} must map
   simply to their lowercase equivalents.  */

enum {
	CONV_n = 0,
	CONV_p,
	CONV_x, CONV_X,	CONV_o,	CONV_u,	CONV_d,	CONV_i,
	CONV_f, CONV_F, CONV_e, CONV_E, CONV_g, CONV_G, CONV_a, CONV_A,
	CONV_C, CONV_S, CONV_LEFTBRACKET, CONV_m, CONV_c, CONV_s, CONV_leftbracket,
	CONV_percent, CONV_whitespace /* not in SPEC_* and no flags */
};

#ifdef __UCLIBC_HAS_FLOATS__
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
/*                         p   x   X  o   u   d   i   f   F   e   E   g   G   a   A */
#define SPEC_BASE		{ 16, 16, 16, 8, 10, 10,  0,  0,  0,  0,  0,  0,  0,  0,  0 }
#else
/*                         p   x   X  o   u   d   i   f   F   e   E   g   G   a   A */
#define SPEC_BASE		{ 16, 16, 16, 8, 10, 10,  0, 10, 10, 10, 10, 10, 10, 10, 10 }
#endif
#else  /* __UCLIBC_HAS_FLOATS__ */
/*                         p   x   X  o   u   d   i   f   F   e   E   g   G   a   A */
#define SPEC_BASE		{ 16, 16, 16, 8, 10, 10,  0 }
#endif /* __UCLIBC_HAS_FLOATS__ */

#define SPEC_FLAGS		"*'I"

enum {
	FLAG_SURPRESS   =   0x10,	/* MUST BE 1ST!!  See DO_FLAGS. */
	FLAG_THOUSANDS	=	0x20,
	FLAG_I18N		=	0x40,	/* only works for d, i, u */
	FLAG_MALLOC     =   0x80,	/* only works for c, s, S, and [ (and l[)*/
};


#define SPEC_RANGES		{ CONV_n, CONV_p, CONV_i, CONV_A, \
						  CONV_C, CONV_LEFTBRACKET, \
						  CONV_c, CONV_leftbracket }

/* Note: We treat L and ll as synonymous... for ints and floats. */

#define SPEC_ALLOWED_FLAGS		{ \
	/* n */			(0x0f|FLAG_SURPRESS), \
	/* p */			(   0|FLAG_SURPRESS), \
	/* oxXudi */	(0x0f|FLAG_SURPRESS|FLAG_THOUSANDS|FLAG_I18N), \
	/* fFeEgGaA */	(0x0c|FLAG_SURPRESS|FLAG_THOUSANDS|FLAG_I18N), \
	/* C */			(   0|FLAG_SURPRESS), \
	/* S and l[ */	(   0|FLAG_SURPRESS|FLAG_MALLOC), \
	/* c */			(0x04|FLAG_SURPRESS|FLAG_MALLOC), \
	/* s and [ */	(0x04|FLAG_SURPRESS|FLAG_MALLOC), \
}


/**********************************************************************/
/*
 * In order to ease translation to what arginfo and _print_info._flags expect,
 * we map:  0:int  1:char  2:longlong 4:long  8:short
 * and then _flags |= (((q << 7) + q) & 0x701) and argtype |= (_flags & 0x701)
 */

/* TODO -- Fix the table below to take into account stdint.h. */
/*  #ifndef LLONG_MAX */
/*  #error fix QUAL_CHARS for no long long!  Affects 'L', 'j', 'q', 'll'. */
/*  #else */
/*  #if LLONG_MAX != INTMAX_MAX */
/*  #error fix QUAL_CHARS intmax_t entry 'j'! */
/*  #endif */
/*  #endif */

#ifdef PDS
#error PDS already defined!
#endif
#ifdef SS
#error SS already defined!
#endif
#ifdef IMS
#error IMS already defined!
#endif

#if PTRDIFF_MAX == INT_MAX
#define PDS		0
#elif PTRDIFF_MAX == LONG_MAX
#define PDS		4
#elif defined(LLONG_MAX) && (PTRDIFF_MAX == LLONG_MAX)
#define PDS		8
#else
#error fix QUAL_CHARS ptrdiff_t entry 't'!
#endif

#if SIZE_MAX == UINT_MAX
#define SS		0
#elif SIZE_MAX == ULONG_MAX
#define SS		4
#elif defined(LLONG_MAX) && (SIZE_MAX == ULLONG_MAX)
#define SS		8
#else
#error fix QUAL_CHARS size_t entries 'z', 'Z'!
#endif

#if INTMAX_MAX == INT_MAX
#define IMS		0
#elif INTMAX_MAX == LONG_MAX
#define IMS		4
#elif defined(LLONG_MAX) && (INTMAX_MAX == LLONG_MAX)
#define IMS		8
#else
#error fix QUAL_CHARS intmax_t entry 'j'!
#endif

#define QUAL_CHARS		{ \
	/* j:(u)intmax_t z:(s)size_t  t:ptrdiff_t  \0:int  q:long_long */ \
	'h',   'l',  'L',  'j',  'z',  't',  'q', 0, \
	 2,     4,    8,  IMS,   SS,  PDS,    8,  0, /* TODO -- fix!!! */ \
	 1,     8 \
}


/**********************************************************************/

#ifdef L_vfwscanf
#if WINT_MIN > WEOF
#error Unfortunately, we currently need wint_t to be able to store WEOF.  Sorry.
#endif
#define W_EOF WEOF
#define Wint wint_t
#define Wchar wchar_t
#define Wuchar __uwchar_t
#define ISSPACE(C) iswspace((C))
#define VFSCANF vfwscanf
#define GETC(SC) (SC)->sc_getc((SC))
#else
typedef unsigned char __uchar_t;
#define W_EOF EOF
#define Wint int
#define Wchar char
#define Wuchar __uchar_t
#define ISSPACE(C) isspace((C))
#define VFSCANF vfscanf
#ifdef __UCLIBC_HAS_WCHAR__
#define GETC(SC) (SC)->sc_getc((SC))
#else  /* __UCLIBC_HAS_WCHAR__ */
#define GETC(SC) getc_unlocked((SC)->fp)
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif

struct scan_cookie {
	Wint cc;
	Wint ungot_char;
	FILE *fp;
	int nread;
	int width;

#ifdef __UCLIBC_HAS_WCHAR__
	wchar_t app_ungot;			/* Match FILE struct member type. */
	unsigned char ungot_wchar_width;
#else  /* __UCLIBC_HAS_WCHAR__ */
	unsigned char app_ungot;	/* Match FILE struct member type. */
#endif /* __UCLIBC_HAS_WCHAR__ */

	char ungot_flag;

#ifdef __UCLIBC_HAS_WCHAR__
	char ungot_wflag;			/* vfwscanf */
	char mb_fail;				/* vfscanf */
	mbstate_t mbstate;			/* vfscanf */
	wint_t wc;
	wint_t ungot_wchar;			/* to support __scan_getc */
	int (*sc_getc)(struct scan_cookie *);
#endif /* __UCLIBC_HAS_WCHAR__ */

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	const char *grouping;
	const unsigned char *thousands_sep;
	int tslen;
#ifdef __UCLIBC_HAS_WCHAR__
	wchar_t thousands_sep_wc;
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */

#ifdef __UCLIBC_HAS_FLOATS__
	const unsigned char *decpt;
	int decpt_len;
#ifdef __UCLIBC_HAS_WCHAR__
	wchar_t decpt_wc;
#endif /* __UCLIBC_HAS_WCHAR__ */
	const unsigned char *fake_decpt;
#endif /* __UCLIBC_HAS_FLOATS__ */

};

typedef struct {
#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
#if NL_ARGMAX > 10
#warning NL_ARGMAX > 10, and space is allocated on the stack for positional args.
#endif
	void *pos_args[NL_ARGMAX];
	int num_pos_args;		/* Must start at -1. */
	int cur_pos_arg;
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */
	void *cur_ptr;
	const unsigned char *fmt;
	int cnt, dataargtype, conv_num, max_width;
	unsigned char store, flags;
} psfs_t;						/* parse scanf format state */


/**********************************************************************/
/**********************************************************************/

extern void __init_scan_cookie(register struct scan_cookie *sc,
							   register FILE *fp) attribute_hidden;
extern int __scan_getc(register struct scan_cookie *sc) attribute_hidden;
extern void __scan_ungetc(register struct scan_cookie *sc) attribute_hidden;

#ifdef __UCLIBC_HAS_FLOATS__
extern int __scan_strtold(long double *ld, struct scan_cookie *sc);
#endif /* __UCLIBC_HAS_FLOATS__ */

extern int __psfs_parse_spec(psfs_t *psfs) attribute_hidden;
extern int __psfs_do_numeric(psfs_t *psfs, struct scan_cookie *sc) attribute_hidden;

/**********************************************************************/
#ifdef L___scan_cookie

#ifndef __UCLIBC_HAS_LOCALE__
static const char decpt_str[] = ".";
#endif

void attribute_hidden __init_scan_cookie(register struct scan_cookie *sc,
						register FILE *fp)
{
	sc->fp = fp;
	sc->nread = 0;
	sc->ungot_flag = 0;
	sc->app_ungot = ((fp->__modeflags & __FLAG_UNGOT) ? fp->__ungot[1] : 0);
#ifdef __UCLIBC_HAS_WCHAR__
	sc->ungot_wflag = 0;		/* vfwscanf */
	sc->mb_fail = 0;
#endif /* __UCLIBC_HAS_WCHAR__ */

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	if (*(sc->grouping = __UCLIBC_CURLOCALE->grouping)) {
		sc->thousands_sep = (const unsigned char *) __UCLIBC_CURLOCALE->thousands_sep;
		sc->tslen = __UCLIBC_CURLOCALE->thousands_sep_len;
#ifdef __UCLIBC_HAS_WCHAR__
		sc->thousands_sep_wc = __UCLIBC_CURLOCALE->thousands_sep_wc;
#endif /* __UCLIBC_HAS_WCHAR__ */
	}
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */

#ifdef __UCLIBC_HAS_FLOATS__
#ifdef __UCLIBC_HAS_LOCALE__
	sc->decpt = (const unsigned char *) __UCLIBC_CURLOCALE->decimal_point;
	sc->decpt_len = __UCLIBC_CURLOCALE->decimal_point_len;
#else  /* __UCLIBC_HAS_LOCALE__ */
	sc->fake_decpt = sc->decpt = (unsigned char *) decpt_str;
	sc->decpt_len = 1;
#endif /* __UCLIBC_HAS_LOCALE__ */
#ifdef __UCLIBC_HAS_WCHAR__
#ifdef __UCLIBC_HAS_LOCALE__
	sc->decpt_wc = __UCLIBC_CURLOCALE->decimal_point_wc;
#else
	sc->decpt_wc = '.';
#endif
#endif /* __UCLIBC_HAS_WCHAR__ */
#endif /* __UCLIBC_HAS_FLOATS__ */

}

int attribute_hidden __scan_getc(register struct scan_cookie *sc)
{
	int c;

#ifdef __UCLIBC_HAS_WCHAR__
	assert(!sc->mb_fail);
#endif /* __UCLIBC_HAS_WCHAR__ */

	sc->cc = EOF;

	if (--sc->width < 0) {
		sc->ungot_flag |= 2;
		return -1;
	}

	if (sc->ungot_flag == 0) {
#if !defined(__STDIO_BUFFERS) && !defined(__UCLIBC_HAS_WCHAR__)
		if (!__STDIO_STREAM_IS_FAKE_VSSCANF_NB(sc->fp)) {
			c = GETC(sc);
		} else {
			__FILE_vsscanf *fv = (__FILE_vsscanf *)(sc->fp);
			if (fv->bufpos < fv->bufread) {
				c = *fv->bufpos++;
			} else {
				c = EOF;
				sc->fp->__modeflags |= __FLAG_EOF;
			}
		}
		if (c == EOF) {
			sc->ungot_flag |= 2;
			return -1;
		}
#else
		if ((c = GETC(sc)) == EOF) {
			sc->ungot_flag |= 2;
			return -1;
		}
#endif
		sc->ungot_char = c;
	} else {
		assert(sc->ungot_flag == 1);
		sc->ungot_flag = 0;
	}

	++sc->nread;
	return sc->cc = sc->ungot_char;
}

void attribute_hidden __scan_ungetc(register struct scan_cookie *sc)
{
	++sc->width;
	if (sc->ungot_flag == 2) {	/* last was EOF */
		sc->ungot_flag = 0;
		sc->cc = sc->ungot_char;
	} else if (sc->ungot_flag == 0) {
		sc->ungot_flag = 1;
		--sc->nread;
	} else {
		assert(0);
	}
}

#endif
/**********************************************************************/
#ifdef L___psfs_parse_spec

#ifdef SPEC_FLAGS
static const unsigned char spec_flags[] = SPEC_FLAGS;
#endif /* SPEC_FLAGS */
static const unsigned char spec_chars[] = SPEC_CHARS;
static const unsigned char qual_chars[] = QUAL_CHARS;
static const unsigned char spec_ranges[] = SPEC_RANGES;
static const unsigned short spec_allowed[] = SPEC_ALLOWED_FLAGS;

int attribute_hidden __psfs_parse_spec(register psfs_t *psfs)
{
	const unsigned char *p;
	const unsigned char *fmt0 = psfs->fmt;
	int i;
#ifdef SPEC_FLAGS
	int j;
#endif
#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
	unsigned char fail = 0;

	i = 0;						/* Do this here to avoid a warning. */

	if (!__isdigit_char(*psfs->fmt)) { /* Not a positional arg. */
		fail = 1;
		goto DO_FLAGS;
	}

	/* parse the positional arg (or width) value */
	do {
		if (i <= ((INT_MAX - 9)/10)) {
			i = (i * 10) + (*psfs->fmt++ - '0');
		}
	} while (__isdigit_char(*psfs->fmt));

	if (*psfs->fmt != '$') { /* This is a max field width. */
		if (psfs->num_pos_args >= 0) { /* Already saw a pos arg! */
			goto ERROR_EINVAL;
		}
		psfs->max_width = i;
		psfs->num_pos_args = -2;
		goto DO_QUALIFIER;
	}
	++psfs->fmt;			/* Advance past '$'. */
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */

#if defined(SPEC_FLAGS) || (defined(NL_ARGMAX) && (NL_ARGMAX > 0))
 DO_FLAGS:
#endif /* defined(SPEC_FLAGS) || (defined(NL_ARGMAX) && (NL_ARGMAX > 0)) */
#ifdef SPEC_FLAGS
	p = spec_flags;
	j = FLAG_SURPRESS;
	do {
		if (*p == *psfs->fmt) {
			++psfs->fmt;
			psfs->flags |= j;
			goto DO_FLAGS;
		}
		j += j;
	} while (*++p);

	if (psfs->flags & FLAG_SURPRESS) { /* Suppress assignment. */
		psfs->store = 0;
		goto DO_WIDTH;
	}
#else  /* SPEC_FLAGS */
	if (*psfs->fmt == '*') {	/* Suppress assignment. */
		++psfs->fmt;
		psfs->store = 0;
		goto DO_WIDTH;
	}
#endif /* SPEC_FLAGS */


#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
	if (fail) {
		/* Must be a non-positional arg */
		if (psfs->num_pos_args >= 0) { /* Already saw a pos arg! */
			goto ERROR_EINVAL;
		}
		psfs->num_pos_args = -2;
	} else {
		if ((psfs->num_pos_args == -2) || (((unsigned int)(--i)) >= NL_ARGMAX)) {
			/* Already saw a non-pos arg or (0-based) num too large. */
			goto ERROR_EINVAL;
		}
		psfs->cur_pos_arg = i;
	}
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */

 DO_WIDTH:
	for (i = 0 ; __isdigit_char(*psfs->fmt) ; ) {
		if (i <= ((INT_MAX - 9)/10)) {
			i = (i * 10) + (*psfs->fmt++ - '0');
			psfs->max_width = i;
		}
	}

#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
 DO_QUALIFIER:
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */
	p = qual_chars;
	do {
		if (*psfs->fmt == *p) {
			++psfs->fmt;
			break;
		}
	} while (*++p);
	if ((p - qual_chars < 2) && (*psfs->fmt == *p)) {
		p += ((sizeof(qual_chars)-2) / 2);
		++psfs->fmt;
	}
	psfs->dataargtype = ((int)(p[(sizeof(qual_chars)-2) / 2])) << 8;

	p = spec_chars;
	do {
		if (*psfs->fmt == *p) {
			int p_m_spec_chars = p - spec_chars;

			if (*p == 'm' &&
				(psfs->fmt[1] == '[' || psfs->fmt[1] == 'c' ||
				 /* Assumes ascii for 's' and 'S' test. */
				 (psfs->fmt[1] | 0x20) == 's'))
			{
				if (psfs->store)
					psfs->flags |= FLAG_MALLOC;
				++psfs->fmt;
				++p;
				continue; /* The related conversions follow 'm'. */
			}

			for (p = spec_ranges; p_m_spec_chars > *p ; ++p) {}
			if (((psfs->dataargtype >> 8) | psfs->flags)
				& ~spec_allowed[(int)(p - spec_ranges)]
				) {
				goto ERROR_EINVAL;
			}

			if (p_m_spec_chars == CONV_p) {
				/* a pointer has the same size as 'long int'  */
				psfs->dataargtype = PA_FLAG_LONG;
			} else if ((p_m_spec_chars >= CONV_c)
				&& (psfs->dataargtype & PA_FLAG_LONG)) {
				p_m_spec_chars -= CONV_c - CONV_C; /* lc -> C, ls -> S, l[ -> ?? */
			}

			psfs->conv_num = p_m_spec_chars;
			return psfs->fmt - fmt0;
		}
		if (!*++p) {
		ERROR_EINVAL:
			__set_errno(EINVAL);
			return -1;
		}
	} while(1);

	assert(0);
}

#endif
/**********************************************************************/
#if defined(L_vfscanf) || defined(L_vfwscanf)

#ifdef __UCLIBC_HAS_WCHAR__
#ifdef L_vfscanf
static int sc_getc(register struct scan_cookie *sc)
{
	return (getc_unlocked)(sc->fp);	/* Disable the macro. */
}

static int scan_getwc(register struct scan_cookie *sc)
{
	size_t r;
	int width;
	wchar_t wc[1];
	char b[1];

	if (--sc->width < 0) {
		sc->ungot_flag |= 2;
		return -1;
	}

	width = sc->width;			/* Preserve width. */
	sc->width = INT_MAX;		/* MB_CUR_MAX can invoke a function. */

	assert(!sc->mb_fail);

	r = (size_t)(-3);
	while (__scan_getc(sc) >= 0) {
		*b = sc->cc;

		r = mbrtowc(wc, b, 1, &sc->mbstate);
		if (((ssize_t) r) >= 0) { /* Successful completion of a wc. */
			sc->wc = *wc;
			goto SUCCESS;
		} else if (r == ((size_t) -2)) {
			/* Potentially valid but incomplete. */
			continue;
		}
		break;
	}

	if (r == ((size_t)(-3))) {	/* EOF or ERROR on first read */
		sc->wc = WEOF;
		r = (size_t)(-1);
	} else {
		/* If we reach here, either r == ((size_t)-1) and
		 * mbrtowc set errno to EILSEQ, or r == ((size_t)-2)
		 * and stream is in an error state or at EOF with a
		 * partially complete wchar. */
		__set_errno(EILSEQ);		/* In case of incomplete conversion. */
		sc->mb_fail = 1;
	}

 SUCCESS:
	sc->width = width;			/* Restore width. */

	return (int)((ssize_t) r);
}

#endif /* L_vfscanf */

#ifdef L_vfwscanf

/* This gets called by __scan_getc.  __scan_getc is called by vfwscanf
 * when the next wide char is expected to be valid ascii (digits).
 */
static int sc_getc(register struct scan_cookie *sc)
{
	wint_t wc;

	if (__STDIO_STREAM_IS_FAKE_VSWSCANF(sc->fp)) {
		if (sc->fp->__bufpos < sc->fp->__bufend) {
			wc = *((wchar_t *)(sc->fp->__bufpos));
			sc->fp->__bufpos += sizeof(wchar_t);
		} else {
			sc->fp->__modeflags |= __FLAG_EOF;
			return EOF;
		}
	} else if ((wc = fgetwc_unlocked(sc->fp)) == WEOF) {
		return EOF;
	}

	sc->ungot_wflag = 1;
	sc->ungot_wchar = wc;
	sc->ungot_wchar_width = sc->fp->__ungot_width[0];

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	if (wc == sc->thousands_sep_wc) {
		wc = ',';
	} else
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
#ifdef __UCLIBC_HAS_FLOATS__
	if (wc == sc->decpt_wc) {
		wc = '.';
	} else
#endif /* __UCLIBC_HAS_FLOATS__ */
	sc->wc = sc->ungot_char = wc;

	return (int) wc;
}

static int scan_getwc(register struct scan_cookie *sc)
{
	wint_t wc;

	sc->wc = WEOF;

	if (--sc->width < 0) {
		sc->ungot_flag |= 2;
		return -1;
	}

	if (sc->ungot_flag == 0) {
		if (__STDIO_STREAM_IS_FAKE_VSWSCANF(sc->fp)) {
			if (sc->fp->__bufpos < sc->fp->__bufend) {
				wc = *((wchar_t *)(sc->fp->__bufpos));
				sc->fp->__bufpos += sizeof(wchar_t);
			} else {
				sc->ungot_flag |= 2;
				return -1;
			}
		} else if ((wc = fgetwc_unlocked(sc->fp)) == WEOF) {
			sc->ungot_flag |= 2;
			return -1;
		}
		sc->ungot_wflag = 1;
		sc->ungot_char = wc;
		sc->ungot_wchar_width = sc->fp->__ungot_width[0];
	} else {
		assert(sc->ungot_flag == 1);
		sc->ungot_flag = 0;
	}

	++sc->nread;
	sc->wc = sc->ungot_char;

	return 0;
}


#endif /* L_vfwscanf */
#endif /* __UCLIBC_HAS_WCHAR__ */

static __inline void kill_scan_cookie(register struct scan_cookie *sc)
{
#ifdef L_vfscanf

	if (sc->ungot_flag & 1) {
#if !defined(__STDIO_BUFFERS) && !defined(__UCLIBC_HAS_WCHAR__)
		if (!__STDIO_STREAM_IS_FAKE_VSSCANF_NB(sc->fp)) {
			ungetc(sc->ungot_char, sc->fp);
		}
#else
		ungetc(sc->ungot_char, sc->fp);
#endif
		/* Deal with distiction between user and scanf ungots. */
		if (sc->nread == 0) {	/* Only one char was read... app ungot? */
			sc->fp->__ungot[1] = sc->app_ungot; /* restore ungot state. */
		} else {
			sc->fp->__ungot[1] = 0;
		}
	}

#else

	if ((sc->ungot_flag & 1) && (sc->ungot_wflag & 1)
		&& !__STDIO_STREAM_IS_FAKE_VSWSCANF(sc->fp)
		&& (sc->fp->__state.__mask == 0)
		) {
		ungetwc(sc->ungot_char, sc->fp);
		/* Deal with distiction between user and scanf ungots. */
		if (sc->nread == 0) {	/* Only one char was read... app ungot? */
			sc->fp->__ungot[1] = sc->app_ungot; /* restore ungot state. */
		} else {
			sc->fp->__ungot[1] = 0;
		}
		sc->fp->__ungot_width[1] = sc->ungot_wchar_width;
	}

#endif
}


int VFSCANF (FILE *__restrict fp, const Wchar *__restrict format, va_list arg)
{
	const Wuchar *fmt;
	unsigned char *b;

#ifdef L_vfwscanf
	wchar_t wbuf[1];
	wchar_t *wb;
#endif /* L_vfwscanf */

#if defined(__UCLIBC_HAS_LOCALE__) && !defined(L_vfwscanf) || !defined(L_vfscanf)
	mbstate_t mbstate;
#endif

	struct scan_cookie sc;
	psfs_t psfs;
	int i;

#define MAX_DIGITS 65			/* Allow one leading 0. */
	unsigned char buf[MAX_DIGITS+2];
#ifdef L_vfscanf
	unsigned char scanset[UCHAR_MAX + 1];
	unsigned char invert = 0;	/* Careful!  Meaning changes. */
#endif /* L_vfscanf */
	unsigned char fail;
	unsigned char zero_conversions = 1;
	__STDIO_AUTO_THREADLOCK_VAR;

	/* To support old programs, don't check mb validity if in C locale. */
#if defined(__UCLIBC_HAS_LOCALE__) && !defined(L_vfwscanf)
	/* ANSI/ISO C99 requires format string to be a valid multibyte string
	 * beginning and ending in its initial shift state. */
	if (__UCLIBC_CURLOCALE->encoding != __ctype_encoding_7_bit) {
		const char *p = format;
		mbstate.__mask = 0;		/* Initialize the mbstate. */
		if (mbsrtowcs(NULL, &p, SIZE_MAX, &mbstate) == ((size_t)(-1))) {
			__set_errno(EINVAL); /* Format string is invalid. */
			return 0;
		}
	}
#endif /* defined(__UCLIBC_HAS_LOCALE__) && !defined(L_vfwscanf) */

#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
	psfs.num_pos_args = -1;		/* Must start at -1. */
	/* Initialize positional arg ptrs to NULL. */
	memset(psfs.pos_args, 0, sizeof(psfs.pos_args));
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */

	__STDIO_AUTO_THREADLOCK(fp);

	__STDIO_STREAM_VALIDATE(fp);

	__init_scan_cookie(&sc,fp);
#ifdef __UCLIBC_HAS_WCHAR__
	sc.sc_getc = sc_getc;
	sc.ungot_wchar_width = sc.fp->__ungot_width[1];

#ifdef L_vfwscanf

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
	if (*sc.grouping) {
		sc.thousands_sep = (const unsigned char *) ",";
		sc.tslen = 1;
	}
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */

#ifdef __UCLIBC_HAS_FLOATS__
	sc.fake_decpt = (const unsigned char *) ".";
#endif /* __UCLIBC_HAS_FLOATS__ */

#else  /* L_vfwscanf */

#ifdef __UCLIBC_HAS_FLOATS__
	sc.fake_decpt = sc.decpt;
#endif /* __UCLIBC_HAS_FLOATS__ */

#endif /* L_vfwscanf */

#endif /* __UCLIBC_HAS_WCHAR__ */
	psfs.cnt = 0;

	/* Note: If we ever wanted to support non-nice codesets, we
	 * would really need to do a mb->wc conversion here in the
	 * vfscanf case.  Related changes would have to be made in
	 * the code that follows... basicly wherever fmt appears. */
	for (fmt = (const Wuchar *) format ; *fmt ; /* ++fmt */) {

		psfs.store = 1;
		psfs.flags = 0;
#ifndef NDEBUG
		psfs.cur_ptr = NULL;	/* Debugging aid. */
#endif /* NDEBUG */


		sc.ungot_flag &= 1;		/* Clear (possible fake) EOF. */
		sc.width = psfs.max_width = INT_MAX;

		/* Note: According to the standards, vfscanf does use isspace
		 * here. So, if we did a mb->wc conversion, we would have to do
		 * something like
		 *      ((((__uwchar_t)wc) < UCHAR_MAX) && isspace(wc))
		 * because wc might not be in the allowed domain. */
		if (ISSPACE(*fmt)) {
			do {
				++fmt;
			} while (ISSPACE(*fmt));
			--fmt;
			psfs.conv_num = CONV_whitespace;
			goto DO_WHITESPACE;
		}

		if (*fmt == '%') {		/* Conversion specification. */
			if (*++fmt == '%') { /* Remember, '%' eats whitespace too. */
				/* Note: The standard says no conversion occurs.
				 * So do not reset zero_conversions flag. */
				psfs.conv_num = CONV_percent;
				goto DO_CONVERSION;
			}


#ifdef L_vfscanf
			psfs.fmt = fmt;
#else  /* L_vfscanf */
			{
				const __uwchar_t *wf = fmt;
				psfs.fmt = b = buf;

				while (*wf && __isascii(*wf) && (b < buf + sizeof(buf) - 1)) {
					*b++ = *wf++;
				}
				*b = 0;
				if (b == buf) { /* Bad conversion specifier! */
					goto DONE;
				}
			}
#endif /* L_vfscanf */
			if ((i = __psfs_parse_spec(&psfs)) < 0) { /* Bad conversion specifier! */
				goto DONE;
			}
			fmt += i;

			if (psfs.store) {
#if defined(NL_ARGMAX) && (NL_ARGMAX > 0)
				if (psfs.num_pos_args == -2) {
					psfs.cur_ptr = va_arg(arg, void *);
				} else {
					while (psfs.cur_pos_arg > psfs.num_pos_args) {
						psfs.pos_args[++psfs.num_pos_args] = va_arg(arg, void *);
					}
					psfs.cur_ptr = psfs.pos_args[psfs.cur_pos_arg];
				}
#else  /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */
				psfs.cur_ptr = va_arg(arg, void *);
#endif /* defined(NL_ARGMAX) && (NL_ARGMAX > 0) */
			}

		DO_CONVERSION:
			/* First, consume white-space if not n, c, [, C, or l[. */
			if ((((1L << CONV_n)|(1L << CONV_C)|(1L << CONV_c)
				 |(1L << CONV_LEFTBRACKET)|(1L << CONV_leftbracket))
				 & (1L << psfs.conv_num)) == 0
				) {
			DO_WHITESPACE:
				while ((__scan_getc(&sc) >= 0)
#ifdef L_vfscanf
					   && isspace(sc.cc)
#else  /* L_vfscanf */
					   && iswspace(sc.wc)
#endif /* L_vfscanf */
					   ) {}
				__scan_ungetc(&sc);
				if (psfs.conv_num == CONV_whitespace) {
					goto NEXT_FMT;
				}
			}

			sc.width = psfs.max_width; /* Now limit the max width. */

			if (sc.width == 0) { /* 0 width is forbidden. */
				goto DONE;
			}


			if (psfs.conv_num == CONV_percent) {
				goto MATCH_CHAR;
			}

			if (psfs.conv_num == CONV_n) {
/* 				zero_conversions = 0; */
				if (psfs.store) {
					_store_inttype(psfs.cur_ptr, psfs.dataargtype,
								   (uintmax_t) sc.nread);
				}
				goto NEXT_FMT;
			}

			if (psfs.conv_num <= CONV_A) { /* pointer, integer, or float spec */
				int r = __psfs_do_numeric(&psfs, &sc);
#ifndef L_vfscanf
				if (sc.ungot_wflag == 1) {	/* fix up  '?', '.', and ',' hacks */
					sc.cc = sc.ungot_char = sc.ungot_wchar;
				}
#endif
				if (r != -1) {	/* Either success or a matching failure. */
					zero_conversions = 0;
				}
				if (r < 0) {
					goto DONE;
				}
				goto NEXT_FMT;
			}

			/* Do string conversions here since they are not common code. */


#ifdef L_vfscanf

			if
#ifdef __UCLIBC_HAS_WCHAR__
				(psfs.conv_num >= CONV_LEFTBRACKET)
#else  /* __UCLIBC_HAS_WCHAR__ */
				(psfs.conv_num >= CONV_c)
#endif /* __UCLIBC_HAS_WCHAR__ */
			{
				/* We might have to handle the allocation ourselves */
				int len;
				unsigned char **ptr;

				b = (psfs.store ? ((unsigned char *) psfs.cur_ptr) : buf);
				/* With 'm', we actually got a pointer to a pointer */
				ptr = (void *)b;

				if (psfs.flags & FLAG_MALLOC) {
					len = 0;
					b = NULL;
				} else
					len = -1;

				fail = 1;

				if (psfs.conv_num == CONV_c) {
					if (sc.width == INT_MAX) {
						sc.width = 1;
					}

					if (psfs.flags & FLAG_MALLOC)
						b = malloc(sc.width);

					i = 0;
					while (__scan_getc(&sc) >= 0) {
						zero_conversions = 0;
						b[i] = sc.cc;
						i += psfs.store;
					}
					__scan_ungetc(&sc);
					if (sc.width > 0) {	/* Failed to read all required. */
						goto DONE;
					}
					if (psfs.flags & FLAG_MALLOC)
						*ptr = b;
					psfs.cnt += psfs.store;
					goto NEXT_FMT;
				}

				if (psfs.conv_num == CONV_s) {

					i = 0;
					/* Yes, believe it or not, a %s conversion can store nuls. */
					while ((__scan_getc(&sc) >= 0) && !isspace(sc.cc)) {
						zero_conversions = 0;
						if (i == len) {
							/* Pick a size that won't trigger a lot of
							 * mallocs early on ... */
							len += 256;
							b = realloc(b, len + 1);
						}
						b[i] = sc.cc;
						i += psfs.store;
						fail = 0;
					}

				} else {
#ifdef __UCLIBC_HAS_WCHAR__
					assert((psfs.conv_num == CONV_LEFTBRACKET) || \
						   (psfs.conv_num == CONV_leftbracket));
#else /* __UCLIBC_HAS_WCHAR__ */
					assert((psfs.conv_num == CONV_leftbracket));
#endif /* __UCLIBC_HAS_WCHAR__ */

					invert = 0;

					if (*++fmt == '^') {
						++fmt;
						invert = 1;
					}
					memset(scanset, invert, sizeof(scanset));
					invert = 1-invert;

					if (*fmt == ']') {
						scanset[(int)(']')] = invert;
						++fmt;
					}

					while (*fmt != ']') {
						if (!*fmt) { /* No closing ']'. */
							goto DONE;
						}
						if ((*fmt == '-') && (fmt[1] != ']')
							&& (fmt[-1] < fmt[1]) /* sorted? */
							) {	/* range */
							++fmt;
							i = fmt[-2];
							/* Note: scanset[i] should already have been done
							 * in the previous iteration. */
							do {
								scanset[++i] = invert;
							} while (i < *fmt);
							/* Safe to fall through, and a bit smaller. */
						}
						/* literal char */
						scanset[(int) *fmt] = invert;
						++fmt;
					}

#ifdef __UCLIBC_HAS_WCHAR__
					if (psfs.conv_num == CONV_LEFTBRACKET) {
						goto DO_LEFTBRACKET;
					}
#endif /* __UCLIBC_HAS_WCHAR__ */


					i = 0;
					while (__scan_getc(&sc) >= 0) {
						zero_conversions = 0;
						if (!scanset[sc.cc]) {
							break;
						}
						if (i == len) {
							/* Pick a size that won't trigger a lot of
							 * mallocs early on ... */
							len += 256;
							b = realloc(b, len + 1);
						}
						b[i] = sc.cc;
						i += psfs.store;
						fail = 0;
					}
				}
				/* Common tail for processing of %s and %[. */

				__scan_ungetc(&sc);
				if (fail) {	/* nothing stored! */
					goto DONE;
				}
				if (psfs.flags & FLAG_MALLOC)
					*ptr = b;
				b += i;
				*b = 0;		/* Nul-terminate string. */
				psfs.cnt += psfs.store;
				goto NEXT_FMT;
			}

#ifdef __UCLIBC_HAS_WCHAR__
		DO_LEFTBRACKET:			/* Need to do common wide init. */
			if (psfs.conv_num >= CONV_C) {
				wchar_t wbuf[1];
				wchar_t *wb;

				sc.mbstate.__mask = 0;

				wb = (psfs.store ? ((wchar_t *) psfs.cur_ptr) : wbuf);
				fail = 1;

				if (psfs.conv_num == CONV_C) {
					if (sc.width == INT_MAX) {
						sc.width = 1;
					}

					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						assert(sc.width >= 0);
						*wb = sc.wc;
						wb += psfs.store;
					}

					__scan_ungetc(&sc);
					if (sc.width > 0) {	/* Failed to read all required. */
						goto DONE;
					}
					psfs.cnt += psfs.store;
					goto NEXT_FMT;
				}


				if (psfs.conv_num == CONV_S) {
					/* Yes, believe it or not, a %s conversion can store nuls. */
					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						if ((((__uwchar_t)(sc.wc)) <= UCHAR_MAX) && isspace(sc.wc)) {
							break;
						}
						*wb = sc.wc;
						wb += psfs.store;
						fail = 0;
					}
				} else {
					assert(psfs.conv_num == CONV_LEFTBRACKET);

					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						if (((__uwchar_t) sc.wc) <= UCHAR_MAX) {
							if (!scanset[sc.wc]) {
								break;
							}
						} else if (invert) {
							break;
						}
						*wb = sc.wc;
						wb += psfs.store;
						fail = 0;
					}
				}
				/* Common tail for processing of %ls and %l[. */

				__scan_ungetc(&sc);
				if (fail || sc.mb_fail) { /* Nothing stored or mb error. */
					goto DONE;
				}
				*wb = 0;		/* Nul-terminate string. */
				psfs.cnt += psfs.store;
				goto NEXT_FMT;

			}

#endif /* __UCLIBC_HAS_WCHAR__ */
#else  /* L_vfscanf */

			if (psfs.conv_num >= CONV_C) {
				b = buf;
				wb = wbuf;
				if (psfs.conv_num >= CONV_c) {
					mbstate.__mask = 0;		/* Initialize the mbstate. */
					if (psfs.store) {
						b = (unsigned char *) psfs.cur_ptr;
					}
				} else {
					if (psfs.store) {
						wb = (wchar_t *) psfs.cur_ptr;
					}
				}
				fail = 1;


				if ((psfs.conv_num == CONV_C) || (psfs.conv_num == CONV_c)) {
					if (sc.width == INT_MAX) {
						sc.width = 1;
					}

					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						if (psfs.conv_num == CONV_C) {
							*wb = sc.wc;
							wb += psfs.store;
						} else {
							i = wcrtomb((char*) b, sc.wc, &mbstate);
							if (i < 0) { /* Conversion failure. */
								goto DONE_DO_UNGET;
							}
							if (psfs.store) {
								b += i;
							}
						}
					}
					__scan_ungetc(&sc);
					if (sc.width > 0) {	/* Failed to read all required. */
						goto DONE;
					}
					psfs.cnt += psfs.store;
					goto NEXT_FMT;
				}

				if ((psfs.conv_num == CONV_S) || (psfs.conv_num == CONV_s)) {
					/* Yes, believe it or not, a %s conversion can store nuls. */
					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						if  (iswspace(sc.wc)) {
							break;
						}
						if (psfs.conv_num == CONV_S) {
							*wb = sc.wc;
							wb += psfs.store;
						} else {
							i = wcrtomb((char*) b, sc.wc, &mbstate);
							if (i < 0) { /* Conversion failure. */
								goto DONE_DO_UNGET;
							}
							if (psfs.store) {
								b += i;
							}
						}
						fail = 0;
					}
				} else {
					const wchar_t *sss;
					const wchar_t *ssp;
					unsigned char invert = 0;

					assert((psfs.conv_num == CONV_LEFTBRACKET)
						   || (psfs.conv_num == CONV_leftbracket));

					if (*++fmt == '^') {
						++fmt;
						invert = 1;
					}
					sss = (const wchar_t *) fmt;
					if (*fmt == ']') {
						++fmt;
					}
					while (*fmt != ']') {
						if (!*fmt) { /* No closing ']'. */
							goto DONE;
						}
						if ((*fmt == '-') && (fmt[1] != ']')
							&& (fmt[-1] < fmt[1]) /* sorted? */
							) {	/* range */
							++fmt;
						}
						++fmt;
					}
					/* Ok... a valid scanset spec. */

					while (scan_getwc(&sc) >= 0) {
						zero_conversions = 0;
						ssp = sss;
						do {	/* We know sss < fmt. */
							if (*ssp == '-') { /* possible range... */
								/* Note: We accept a-c-e (ordered) as
								 * equivalent to a-e. */
								if (ssp > sss) {
									if ((++ssp < (const wchar_t *) fmt)
										&& (ssp[-2] < *ssp)	/* sorted? */
										) { /* yes */
										if ((sc.wc >= ssp[-2])
											&& (sc.wc <= *ssp)) {
											break;
										}
										continue; /* not in range */
									}
									--ssp; /* oops... '-' at end, so back up */
								}
								/* false alarm... a literal '-' */
							}
							if (sc.wc == *ssp) { /* Matched literal char. */
								break;
							}
						} while (++ssp < (const wchar_t *) fmt);

						if ((ssp == (const wchar_t *) fmt) ^ invert) {
							/* no match and not inverting
							 * or match and inverting */
							break;
						}
						if (psfs.conv_num == CONV_LEFTBRACKET) {
							*wb = sc.wc;
							wb += psfs.store;
						} else {
							i = wcrtomb((char*) b, sc.wc, &mbstate);
							if (i < 0) { /* Conversion failure. */
								goto DONE_DO_UNGET;
							}
							if (psfs.store) {
								b += i;
							}
						}
						fail = 0;
					}
				}
				/* Common tail for processing of %s and %[. */

				__scan_ungetc(&sc);
				if (fail) {	/* nothing stored! */
					goto DONE;
				}
				*wb = 0;		/* Nul-terminate string. */
				*b = 0;
				psfs.cnt += psfs.store;
				goto NEXT_FMT;
			}

#endif /* L_vfscanf */

			assert(0);
			goto DONE;
		} /* conversion specification */

	MATCH_CHAR:
		if (__scan_getc(&sc) != *fmt) {
#ifdef L_vfwscanf
		DONE_DO_UNGET:
#endif /* L_vfwscanf */
			__scan_ungetc(&sc);
			goto DONE;
		}

	NEXT_FMT:
		++fmt;
		if (__FERROR_UNLOCKED(fp)) {
			break;
		}
	}

 DONE:
	if (__FERROR_UNLOCKED(fp) || (*fmt && zero_conversions && __FEOF_UNLOCKED(fp))) {
		psfs.cnt = EOF;			/* Yes, vfwscanf also returns EOF. */
	}

	kill_scan_cookie(&sc);

	__STDIO_STREAM_VALIDATE(fp);

	__STDIO_AUTO_THREADUNLOCK(fp);

	return psfs.cnt;
}
libc_hidden_def(VFSCANF)
#endif
/**********************************************************************/
#ifdef L___psfs_do_numeric

static const unsigned char spec_base[] = SPEC_BASE;
static const unsigned char nil_string[] = "(nil)";

int attribute_hidden __psfs_do_numeric(psfs_t *psfs, struct scan_cookie *sc)
{
	unsigned char *b;
	const unsigned char *p;

#ifdef __UCLIBC_HAS_FLOATS__
	int exp_adjust = 0;
#endif
#define MAX_DIGITS 65			/* Allow one leading 0. */
	unsigned char buf[MAX_DIGITS+2+ 100];
	unsigned char usflag, base;
	unsigned char nonzero = 0;
	unsigned char seendigit = 0;

#ifndef __UCLIBC_HAS_FLOATS__
	if (psfs->conv_num > CONV_i) { /* floating point */
		goto DONE;
	}
#endif

	base = spec_base[psfs->conv_num - CONV_p];
	usflag = (psfs->conv_num <= CONV_u); /* (1)0 if (un)signed */
	b = buf;


	if (psfs->conv_num == CONV_p) { /* Pointer */
		p = nil_string;
		do {
			if ((__scan_getc(sc) < 0) || (*p != sc->cc)) {
				__scan_ungetc(sc);
				if (p > nil_string) {
					/* We matched at least the '(' so even if we
					 * are at eof,  we can not match a pointer. */
					return -2;	/* Matching failure */
				}
				break;
			}
			if (!*++p) {   /* Matched (nil), so no unget necessary. */
				if (psfs->store) {
					++psfs->cnt;
					_store_inttype(psfs->cur_ptr, psfs->dataargtype,
								   (uintmax_t)0);
				}
				return 0;
			}
		} while (1);

	}

	__scan_getc(sc);
	if (sc->cc < 0) {
		return -1;				/* Input failure (nothing read yet). */
	}

	if ((sc->cc == '+') || (sc->cc == '-')) { /* Handle leading sign.*/
		*b++ = sc->cc;
		__scan_getc(sc);
	}

	if ((base & 0xef) == 0) { /* 0xef is ~16, so 16 or 0. */
		if (sc->cc == '0') {	/* Possibly set base and handle prefix. */
			__scan_getc(sc);
			if ((sc->cc|0x20) == 'x') { /* Assumes ascii.. x or X. */
				if (__scan_getc(sc) < 0) {
					/* Either EOF or error (including wc outside char range).
					 * If EOF or error, this is a matching failure (we read 0x).
					 * If wc outside char range, this is also a matching failure.
					 * Hence, we do an unget (although not really necessary here
					 * and fail. */
					goto DONE_DO_UNGET;	/* matching failure */
				}
				base = 16; /* Base 16 for sure now. */
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
				/* The prefix is required for hexadecimal floats. */
				*b++ = '0';
				*b++ = 'x';
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */
			} else { /* oops... back up */
				__scan_ungetc(sc);
				sc->cc = '0';	/* NASTY HACK! */

				base = (base >> 1) + 8;	/* 0->8, 16->16.  no 'if' */
#ifdef __UCLIBC_HAS_FLOATS__
				if (psfs->conv_num > CONV_i) { /* floating point */
					base = 10;
				}
#endif
			}
		} else if (!base) {
			base = 10;
		}
	}

	/***************** digit grouping **********************/
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__

	if ((psfs->flags & FLAG_THOUSANDS) && (base == 10)
		&& *(p = (const unsigned char *) sc->grouping)
		) {

		int nblk1, nblk2, nbmax, lastblock, pass, i;

		nbmax = nblk2 = nblk1 = *p;
		if (*++p) {
			nblk2 = *p;
			if (nbmax < nblk2) {
				nbmax = nblk2;
			}
			assert(!p[1]);
		}

		/* Note: for printf, if 0 and \' flags appear then
		 * grouping is done before 0-padding.  Should we
		 * strip leading 0's first?  Or add a 0 flag? */

		/* For vfwscanf, sc_getc translates, so the value of sc->cc is
		 * either EOF or a char. */

		if (!__isdigit_char_or_EOF(sc->cc)) { /* No starting digit! */
#ifdef __UCLIBC_HAS_FLOATS__
			if (psfs->conv_num > CONV_i) { /* floating point */
				goto NO_STARTING_DIGIT;
			}
#endif
			goto DONE_DO_UNGET;
		}

		if (sc->cc == '0') {
			seendigit = 1;
			*b++ = '0';			/* Store the first 0. */
#if 0
			do {				/* But ignore all subsequent 0s. */
				__scan_getc(sc);
			} while (sc->cc == '0');
#endif
		}
		pass = 0;
		lastblock = 0;
		do {
			i = 0;
			while (__isdigit_char_or_EOF(sc->cc)) {
				seendigit = 1;
				if (i == nbmax) { /* too many digits for a block */
#ifdef __UCLIBC_HAS_SCANF_LENIENT_DIGIT_GROUPING__
					if (!pass) { /* treat as nongrouped */
						if (nonzero) {
							goto DO_NO_GROUP;
						}
						goto DO_TRIM_LEADING_ZEROS;
					}
#endif
					if (nbmax > nblk1) {
						goto DONE_DO_UNGET;	/* matching failure */
					}
					goto DONE_GROUPING_DO_UNGET; /* nbmax == nblk1 */
				}
				++i;

				if (nonzero || (sc->cc != '0')) {
					if (b < buf + MAX_DIGITS) {
						*b++ = sc->cc;
						nonzero = 1;
#ifdef __UCLIBC_HAS_FLOATS__
					} else {
						++exp_adjust;
#endif
					}
				}

				__scan_getc(sc);
			}

			if (i) {			/* we saw digits digits */
				if ((i == nblk2) || ((i < nblk2) && !pass)) {
					/* (possible) outer grp */
					p = sc->thousands_sep;
					if (*p == sc->cc) {	/* first byte matches... */
						/* so check if grouping mb char */
						/* Since 1st matched, either match or fail now
						 * unless EOF (yuk) */
						__scan_getc(sc);
					MBG_LOOP:
						if (!*++p) { /* is a grouping mb char */
							lastblock = i;
							++pass;
							continue;
						}
						if (*p == sc->cc) {
							__scan_getc(sc);
							goto MBG_LOOP;
						}
						/* bad grouping mb char! */
						__scan_ungetc(sc);
						if ((sc->cc >= 0) || (p > sc->thousands_sep + 1)) {
#ifdef __UCLIBC_HAS_FLOATS__
							/* We failed to match a thousep mb char, and
							 * we've read too much to recover.  But if
							 * this is a floating point conversion and
							 * the initial portion of the decpt mb char
							 * matches, then we may still be able to
							 * recover. */
							int k = p - sc->thousands_sep - 1;

							if ((psfs->conv_num > CONV_i) /* float conversion */
								&& (!pass || (i == nblk1)) /* possible last */
								&& !memcmp(sc->thousands_sep, sc->fake_decpt, k)
								/* and prefix matched, so could be decpt */
								) {
								__scan_getc(sc);
								p = sc->fake_decpt + k;
								do {
									if (!*++p) {
										strcpy((char*) b, (char*) sc->decpt);
										b += sc->decpt_len;
										goto GOT_DECPT;
									}
									if (*p != sc->cc) {
										__scan_ungetc(sc);
										break; /* failed */
									}
									__scan_getc(sc);
								} while (1);
							}
#endif /* __UCLIBC_HAS_FLOATS__ */
							goto DONE;
						}
						/* was EOF and 1st, so recoverable. */
					}
				}
				if ((i == nblk1) || ((i < nblk1) && !pass)) {
					/* got an inner group */
					goto DONE_GROUPING_DO_UNGET;
				}
				goto DONE_DO_UNGET;	/* Matching failure. */
			} /* i != 0 */

			assert(pass);

			goto DONE_DO_UNGET;
		} while (1);

		assert(0);				/* Should never get here. */
	}

#endif /***************** digit grouping **********************/

	/* Not grouping so first trim all but one leading 0. */
#ifdef __UCLIBC_HAS_SCANF_LENIENT_DIGIT_GROUPING__
	DO_TRIM_LEADING_ZEROS:
#endif /* __UCLIBC_HAS_SCANF_LENIENT_DIGIT_GROUPING__ */
	if (sc->cc == '0') {
		seendigit = 1;
		*b++ = '0';				/* Store the first 0. */
		do {					/* But ignore all subsequent 0s. */
			__scan_getc(sc);
		} while (sc->cc == '0');
	}

#ifdef __UCLIBC_HAS_SCANF_LENIENT_DIGIT_GROUPING__
 DO_NO_GROUP:
#endif /* __UCLIBC_HAS_SCANF_LENIENT_DIGIT_GROUPING__ */
	/* At this point, we're ready to start reading digits. */

#define valid_digit(cc,base) (isxdigit(cc) && ((base == 16) || (cc - '0' < base)))

	while (valid_digit(sc->cc,base)) { /* Now for significant digits.*/
		if (b - buf < MAX_DIGITS) {
			nonzero = seendigit = 1; /* Set nonzero too 0s trimmed above. */
			*b++ = sc->cc;
#ifdef __UCLIBC_HAS_FLOATS__
		} else {
			++exp_adjust;
#endif
		}
		__scan_getc(sc);
	}

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
 DONE_GROUPING_DO_UNGET:
#endif /* __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__ */
	if (psfs->conv_num <= CONV_i) { /* integer conversion */
		__scan_ungetc(sc);
		*b = 0;						/* null-terminate */
		if (!seendigit) {
			goto DONE;				/* No digits! */
		}
		if (psfs->store) {
			if (*buf == '-') {
				usflag = 0;
			}
			++psfs->cnt;
			_store_inttype(psfs->cur_ptr, psfs->dataargtype,
						   (uintmax_t) STRTOUIM((char *) buf, NULL, base, 1-usflag));
		}
		return 0;
	}

#ifdef __UCLIBC_HAS_FLOATS__

	/* At this point, we have everything left of the decimal point or exponent. */
#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
 NO_STARTING_DIGIT:
#endif
	p = sc->fake_decpt;
	do {
		if (!*p) {
			strcpy((char *) b, (char *) sc->decpt);
			b += sc->decpt_len;
			break;
		}
		if (*p != sc->cc) {
			if (p > sc->fake_decpt) {
				goto DONE_DO_UNGET;	/* matching failure (read some of decpt) */
			}
			goto DO_DIGIT_CHECK;
		}
		++p;
		__scan_getc(sc);
	} while (1);

#ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
 GOT_DECPT:
#endif
	if (!nonzero) {
		if (sc->cc == '0') {
			assert(exp_adjust == 0);
			*b++ = '0';
			++exp_adjust;
			seendigit = 1;
			do {
				--exp_adjust;
				__scan_getc(sc);
			} while (sc->cc == '0');
		}
	}

	while (valid_digit(sc->cc,base)) { /* Process fractional digits.*/
		if (b - buf < MAX_DIGITS) {
			seendigit = 1;
			*b++ = sc->cc;
		}
		__scan_getc(sc);
	}

 DO_DIGIT_CHECK:
	/* Hmm... no decimal point.   */
	if (!seendigit) {
		static const unsigned char nan_inf_str[] = "an\0nfinity";

		if (base == 16) {		/* We had a prefix, but no digits! */
			goto DONE_DO_UNGET;	/* matching failure */
		}

		/* Avoid tolower problems for INFINITY in the tr_TR locale. (yuk)*/
#undef TOLOWER
#define TOLOWER(C)     ((C)|0x20)

		switch (TOLOWER(sc->cc)) {
			case 'i':
				p = nan_inf_str + 3;
				break;
			case 'n':
				p = nan_inf_str;
				break;
			default:
				/* No digits and not inf or nan. */
				goto DONE_DO_UNGET;
		}

		*b++ = sc->cc;

		do {
			__scan_getc(sc);
			if (TOLOWER(sc->cc) == *p) {
				*b++ = sc->cc;
				++p;
				continue;
			}
			if (!*p || (p == nan_inf_str + 5)) { /* match nan/infinity or inf */
				goto GOT_FLOAT;
			}
			/* Unrecoverable.  Even if on 1st char, we had no digits. */
			goto DONE_DO_UNGET;
		} while (1);
	}

	/* If we get here, we had some digits. */

	if (
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
		((base == 16) && (((sc->cc)|0x20) == 'p')) ||
#endif
		(((sc->cc)|0x20) == 'e')
		) {						/* Process an exponent. */
		*b++ = sc->cc;

		__scan_getc(sc);
		if (sc->cc < 0) {
			goto DONE_DO_UNGET;	/* matching failure.. no exponent digits */
		}

		if ((sc->cc == '+') || (sc->cc == '-')) { /* Signed exponent? */
			*b++ = sc->cc;
			__scan_getc(sc);
		}

#define MAX_EXP_DIGITS 20
		assert(seendigit);
		seendigit = 0;
		nonzero = 0;

		if (sc->cc == '0') {
			seendigit = 1;
			*b++ = '0';
			do {
				__scan_getc(sc);
			} while (sc->cc == '0');
		}

		while (__isdigit_char_or_EOF(sc->cc)) { /* Exponent digits (base 10).*/
			if (seendigit < MAX_EXP_DIGITS) {
				++seendigit;
				*b++ = sc->cc;
			}
			__scan_getc(sc);
		}

		if (!seendigit) {		/* No digits.  Unrecoverable. */
			goto DONE_DO_UNGET;
		}
	}


 GOT_FLOAT:
	*b = 0;
	{
		__fpmax_t x;
		char *e;
		x = __strtofpmax((char *) buf, &e, exp_adjust);
		assert(!*e);
		if (psfs->store) {
			if (psfs->dataargtype & PA_FLAG_LONG_LONG) {
				*((long double *)psfs->cur_ptr) = (long double) x;
			} else if (psfs->dataargtype & PA_FLAG_LONG) {
				*((double *)psfs->cur_ptr) = (double) x;
			} else {
				*((float *)psfs->cur_ptr) = (float) x;
			}
			++psfs->cnt;
		}
		__scan_ungetc(sc);
		return 0;
	}
#endif /* __UCLIBC_HAS_FLOATS__ */

 DONE_DO_UNGET:
	__scan_ungetc(sc);
 DONE:
	return -2;					/* Matching failure. */

}
#endif
/**********************************************************************/
