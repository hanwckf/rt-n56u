
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
 *  License along with this library; if not, see
 *  <http://www.gnu.org/licenses/>.
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
#ifdef _LIBC
#include <errno.h>
#include <stddef.h>
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <locale.h>
#include <wchar.h>
#include <bits/uClibc_uwchar.h>

/**********************************************************************/
#ifdef __UCLIBC_HAS_LOCALE__

#define ENCODING		(__UCLIBC_CURLOCALE->encoding)

#define Cc2wc_IDX_SHIFT		__LOCALE_DATA_Cc2wc_IDX_SHIFT
#define Cc2wc_ROW_LEN		__LOCALE_DATA_Cc2wc_ROW_LEN
#define Cwc2c_DOMAIN_MAX	__LOCALE_DATA_Cwc2c_DOMAIN_MAX
#define Cwc2c_TI_SHIFT		__LOCALE_DATA_Cwc2c_TI_SHIFT
#define Cwc2c_TT_SHIFT		__LOCALE_DATA_Cwc2c_TT_SHIFT
#define Cwc2c_TI_LEN		__LOCALE_DATA_Cwc2c_TI_LEN

#ifndef __CTYPE_HAS_UTF_8_LOCALES
#warning __CTYPE_HAS_UTF_8_LOCALES not set!
#endif

#else  /* __UCLIBC_HAS_LOCALE__ */

#define ENCODING (__ctype_encoding_7_bit)
#ifdef __CTYPE_HAS_8_BIT_LOCALES
#error __CTYPE_HAS_8_BIT_LOCALES is defined!
#endif
#ifdef __CTYPE_HAS_UTF_8_LOCALES
#error __CTYPE_HAS_UTF_8_LOCALES is defined!
#endif
#undef L__wchar_utf8sntowcs
#undef L__wchar_wcsntoutf8s

#endif /* __UCLIBC_HAS_LOCALE__ */
/**********************************************************************/

#if WCHAR_MAX > 0xffffUL
#define UTF_8_MAX_LEN 6
#else
#define UTF_8_MAX_LEN 3
#endif

#define KUHN 1

/* Implementation-specific work functions. */

extern size_t _wchar_utf8sntowcs(wchar_t *__restrict pwc, size_t wn,
					const char **__restrict src, size_t n,
					mbstate_t *ps, int allow_continuation) attribute_hidden;

extern size_t _wchar_wcsntoutf8s(char *__restrict s, size_t n,
					const wchar_t **__restrict src, size_t wn) attribute_hidden;
#endif
/**********************************************************************/
#ifdef L_btowc


wint_t btowc(int c)
{
#ifdef __CTYPE_HAS_8_BIT_LOCALES

	wchar_t wc;
	unsigned char buf[1];
	mbstate_t mbstate;

	if (c != EOF) {
		*buf = (unsigned char) c;
		mbstate.__mask = 0;		/* Initialize the mbstate. */
		if (mbrtowc(&wc, (char*) buf, 1, &mbstate) <= 1) {
			return wc;
		}
	}
	return WEOF;

#else  /* !__CTYPE_HAS_8_BIT_LOCALES */

#ifdef __UCLIBC_HAS_LOCALE__
	assert((ENCODING == __ctype_encoding_7_bit)
		   || (ENCODING == __ctype_encoding_utf8));
#endif

	/* If we don't have 8-bit locale support, then this is trivial since
	 * anything outside of 0-0x7f is illegal in C/POSIX and UTF-8 locales. */
	return (((unsigned int)c) < 0x80) ? c : WEOF;

#endif /* !__CTYPE_HAS_8_BIT_LOCALES */
}
libc_hidden_def(btowc)

#endif
/**********************************************************************/
#ifdef L_wctob

/* Note: We completely ignore ps in all currently supported conversions. */


int wctob(wint_t c)
{
#ifdef __CTYPE_HAS_8_BIT_LOCALES

	unsigned char buf[MB_LEN_MAX];

	return (wcrtomb((char*) buf, c, NULL) == 1) ? *buf : EOF;

#else  /*  __CTYPE_HAS_8_BIT_LOCALES */

#ifdef __UCLIBC_HAS_LOCALE__
	assert((ENCODING == __ctype_encoding_7_bit)
		   || (ENCODING == __ctype_encoding_utf8));
#endif /* __UCLIBC_HAS_LOCALE__ */

	/* If we don't have 8-bit locale support, then this is trivial since
	 * anything outside of 0-0x7f is illegal in C/POSIX and UTF-8 locales. */

	/* TODO: need unsigned version of wint_t... */
/*  	return (((unsigned int)c) < 0x80) ? c : WEOF; */
	return ((c >= 0) && (c < 0x80)) ? c : EOF;

#endif /*  __CTYPE_HAS_8_BIT_LOCALES */
}

#endif
/**********************************************************************/
#ifdef L_mbsinit

int mbsinit(const mbstate_t *ps)
{
	return !ps || !ps->__mask;
}
libc_hidden_def(mbsinit)

#endif
/**********************************************************************/
#ifdef L_mbrlen


size_t mbrlen(const char *__restrict s, size_t n, mbstate_t *__restrict ps)
{
	static mbstate_t mbstate;	/* Rely on bss 0-init. */

	return mbrtowc(NULL, s, n, (ps != NULL) ? ps : &mbstate);
}
libc_hidden_def(mbrlen)

#endif
/**********************************************************************/
#ifdef L_mbrtowc


size_t mbrtowc(wchar_t *__restrict pwc, const char *__restrict s,
			   size_t n, mbstate_t *__restrict ps)
{
	static mbstate_t mbstate;	/* Rely on bss 0-init. */
	wchar_t wcbuf[1];
	const char *p;
	size_t r;
	char empty_string[1];		/* Avoid static to be fPIC friendly. */

	if (!ps) {
		ps = &mbstate;
	}

	if (!s) {
		pwc = (wchar_t *) s;	/* NULL */
		empty_string[0] = 0;	/* Init the empty string when necessary. */
		s = empty_string;
		n = 1;
	} else if (*s == '\0') {
		if (pwc)
			*pwc = '\0';
	/* According to the ISO C 89 standard this is the expected behaviour.  */
		return 0;
	} else if (!n) {
		/* TODO: change error code? */
#if 0
		return (ps->__mask && (ps->__wc == 0xffffU))
			? ((size_t) -1) : ((size_t) -2);
#else
		return 0;
#endif
	}

	p = s;

#ifdef __CTYPE_HAS_UTF_8_LOCALES
	/* Need to do this here since mbsrtowcs doesn't allow incompletes. */
	if (ENCODING == __ctype_encoding_utf8) {
		if (!pwc) {
			pwc = wcbuf;
		}
		r = _wchar_utf8sntowcs(pwc, 1, &p, n, ps, 1);
		return (r == 1) ? (p-s) : r; /* Need to return 0 if nul char. */
	}
#endif

	r = mbsnrtowcs(wcbuf, &p, SIZE_MAX, 1, ps);

	if (((ssize_t) r) >= 0) {
		if (pwc) {
			*pwc = *wcbuf;
		}
	}
	return (size_t) r;
}
libc_hidden_def(mbrtowc)

#endif
/**********************************************************************/
#ifdef L_wcrtomb


/* Note: We completely ignore ps in all currently supported conversions. */
/* TODO: Check for valid state anyway? */

size_t wcrtomb(register char *__restrict s, wchar_t wc,
			   mbstate_t *__restrict ps)
{
	wchar_t wcbuf[1];
	const wchar_t *pwc;
	size_t r;
	char buf[MB_LEN_MAX];

	if (!s) {
		s = buf;
		wc = 0;
	}

	pwc = wcbuf;
	wcbuf[0] = wc;

	r = wcsnrtombs(s, &pwc, 1, MB_LEN_MAX, ps);
	return (r != 0) ? r : 1;
}
libc_hidden_def(wcrtomb)

#endif
/**********************************************************************/
#ifdef L_mbsrtowcs


size_t mbsrtowcs(wchar_t *__restrict dst, const char **__restrict src,
				 size_t len, mbstate_t *__restrict ps)
{
	static mbstate_t mbstate;	/* Rely on bss 0-init. */

	return mbsnrtowcs(dst, src, SIZE_MAX, len,
						((ps != NULL) ? ps : &mbstate));
}
libc_hidden_def(mbsrtowcs)

#endif
/**********************************************************************/
#ifdef L_wcsrtombs

/* Note: We completely ignore ps in all currently supported conversions.

 * TODO: Check for valid state anyway? */


size_t wcsrtombs(char *__restrict dst, const wchar_t **__restrict src,
				 size_t len, mbstate_t *__restrict ps)
{
	return wcsnrtombs(dst, src, SIZE_MAX, len, ps);
}
libc_hidden_def(wcsrtombs)

#endif
/**********************************************************************/
#ifdef L__wchar_utf8sntowcs

/* Define DECODER to generate a UTF-8 decoder which passes Markus Kuhn's
 * UTF-8-test.txt strss test.
 */
/*  #define DECODER */

#ifdef DECODER
#ifndef KUHN
#define KUHN
#endif
#endif

size_t attribute_hidden _wchar_utf8sntowcs(wchar_t *__restrict pwc, size_t wn,
						  const char **__restrict src, size_t n,
						  mbstate_t *ps, int allow_continuation)
{
	register const char *s;
	__uwchar_t mask;
	__uwchar_t wc;
	wchar_t wcbuf[1];
	size_t count;
	int incr;

	s = *src;

	assert(s != NULL);
	assert(ps != NULL);

	incr = 1;
	/* NOTE: The following is an AWFUL HACK!  In order to support %s in
	 * wprintf, we need to be able to compute the number of wchars needed
	 * for the mbs conversion, not to exceed the precision specified.
	 * But if dst is NULL, the return value is the length assuming a
	 * sufficiently sized buffer.  So, we allow passing of (wchar_t *) ps
	 * as pwc in order to flag that we really want the length, subject
	 * to the restricted buffer size and no partial conversions.
	 * See mbsnrtowcs() as well. */
	if (!pwc || (pwc == ((wchar_t *)ps))) {
		if (!pwc) {
			wn = SIZE_MAX;
		}
		pwc = wcbuf;
		incr = 0;
	}

	/* This is really here only to support the glibc extension function
	 * __mbsnrtowcs which apparently returns 0 if wn == 0 without any
	 * check on the validity of the mbstate. */
	if (!(count = wn)) {
		return 0;
	}

	if ((mask = (__uwchar_t) ps->__mask) != 0) { /* A continuation... */
#ifdef DECODER
		wc = (__uwchar_t) ps->__wc;
		if (n) {
			goto CONTINUE;
		}
		goto DONE;
#else
		if ((wc = (__uwchar_t) ps->__wc) != 0xffffU) {
			/* TODO: change error code here and below? */
			if (n) {
				goto CONTINUE;
			}
			goto DONE;
		}
		__set_errno(EILSEQ);
		return (size_t) -1;		/* We're in an error state. */
#endif
	}

	do {
		if (!n) {
			goto DONE;
		}
		--n;
		if ((wc = ((unsigned char) *s++)) >= 0x80) { /* Not ASCII... */
			mask = 0x40;
			if (( ((unsigned char)(s[-1] - 0xc0)) < (0xfe - 0xc0) ) &&
			(((unsigned char)s[-1] != 0xc0 ) && ((unsigned char)s[-1] != 0xc1 ))) {
				goto START;
			}
		BAD:
#ifdef DECODER
			wc = 0xfffdU;
			goto COMPLETE;
#else
			ps->__mask = mask;
			ps->__wc = 0xffffU;
			__set_errno(EILSEQ);
			return (size_t) -1;	/* Illegal start byte! */
#endif

		CONTINUE:
			while (n) {
				--n;
				if ((*s & 0xc0) != 0x80) {
					goto BAD;
				}
				mask <<= 5;
				wc <<= 6;
				wc += (*s & 0x3f);	/* keep seperate for bcc (smaller code) */
				++s;
			START:
				wc &= ~(mask << 1);

				if ((wc & mask) == 0) {	/* Character completed. */
					if ((mask >>= 5) == 0x40) {
						mask += mask;
					}
					/* Check for invalid sequences (longer than necessary)
					 * and invalid chars.  */
					if ( (wc < mask) /* Sequence not minimal length. */
#ifdef KUHN
#if UTF_8_MAX_LEN == 3
#error broken since mask can overflow!!
						 /* For plane 0, these are the only defined values.*/
						 || (wc > 0xfffdU)
#else
						 /* Note that we don't need to worry about exceeding */
						 /* 31 bits as that is the most that UTF-8 provides. */
						 || ( ((__uwchar_t)(wc - 0xfffeU)) < 2)
#endif
						 || ( ((__uwchar_t)(wc - 0xd800U)) < (0xe000U - 0xd800U) )
#endif /* KUHN */
						 ) {
						goto BAD;
					}
					goto COMPLETE;
				}
			}
			/* Character potentially valid but incomplete. */
			if (!allow_continuation) {
				if (count != wn) {
					return 0;
				}
				/* NOTE: The following can fail if you allow and then disallow
				 * continuation!!! */
#if UTF_8_MAX_LEN == 3
#error broken since mask can overflow!!
#endif
				/* Need to back up... */
				do {
					--s;
				} while ((mask >>= 5) >= 0x40);
				goto DONE;
			}
			ps->__mask = (wchar_t) mask;
			ps->__wc = (wchar_t) wc;
			*src = s;
			return (size_t) -2;
		}
	COMPLETE:
		*pwc = wc;
		pwc += incr;
	}
#ifdef DECODER
	while (--count);
#else
	while (wc && --count);

	if (!wc) {
		s = NULL;
	}
#endif

 DONE:
	/* ps->__wc is irrelavent here. */
	ps->__mask = 0;
	if (pwc != wcbuf) {
		*src = s;
	}

	return wn - count;
}

#endif
/**********************************************************************/
#ifdef L__wchar_wcsntoutf8s

size_t attribute_hidden _wchar_wcsntoutf8s(char *__restrict s, size_t n,
						  const wchar_t **__restrict src, size_t wn)
{
	register char *p;
	size_t len, t;
	__uwchar_t wc;
	const __uwchar_t *swc;
	int store;
	char buf[MB_LEN_MAX];
	char m;

	store = 1;
	/* NOTE: The following is an AWFUL HACK!  In order to support %ls in
	 * printf, we need to be able to compute the number of bytes needed
	 * for the mbs conversion, not to exceed the precision specified.
	 * But if dst is NULL, the return value is the length assuming a
	 * sufficiently sized buffer.  So, we allow passing of (char *) src
	 * as dst in order to flag that we really want the length, subject
	 * to the restricted buffer size and no partial conversions.
	 * See wcsnrtombs() as well. */
	if (!s || (s == ((char *) src))) {
		if (!s) {
			n = SIZE_MAX;
		}
		s = buf;
		store = 0;
	}

	t = n;
	swc = (const __uwchar_t *) *src;

	assert(swc != NULL);

	while (wn && t) {
		wc = *swc;

		*s = wc;
		len = 1;

		if (wc >= 0x80) {
#ifdef KUHN
			if (
#if UTF_8_MAX_LEN == 3
				/* For plane 0, these are the only defined values.*/
				/* Note that we don't need to worry about exceeding */
				/* 31 bits as that is the most that UTF-8 provides. */
				(wc > 0xfffdU)
#else
				/* UTF_8_MAX_LEN == 6 */
				(wc > 0x7fffffffUL)
				|| ( ((__uwchar_t)(wc - 0xfffeU)) < 2)
#endif
				|| ( ((__uwchar_t)(wc - 0xd800U)) < (0xe000U - 0xd800U) )
				) {
				__set_errno(EILSEQ);
				return (size_t) -1;
			}
#else  /* KUHN */
#if UTF_8_MAX_LEN != 3
			if (wc > 0x7fffffffUL) { /* Value too large. */
				__set_errno(EILSEQ);
				return (size_t) -1;
			}
#endif
#endif /* KUHN */

			wc >>= 1;
			p = s;
			do {
				++p;
			} while (wc >>= 5);
			wc = *swc;
			if ((len = p - s) > t) { /* Not enough space. */
				break;
			}

			m = 0x80;
			while( p>s ) {
				m = (m >> 1) | 0x80;
				*--p = (wc & 0x3f) | 0x80;
				wc >>= 6;
			}
			*s |= (m << 1);
		} else if (wc == 0) {	/* End of string. */
			swc = NULL;
			break;
		}

		++swc;
		--wn;
		t -= len;
		if (store) {
			s += len;
		}
	}

	if (store) {
		*src = (const wchar_t *) swc;
	}

	return n - t;
}


#endif
/**********************************************************************/
#ifdef L_mbsnrtowcs

/* WARNING: We treat len as SIZE_MAX when dst is NULL! */

size_t mbsnrtowcs(wchar_t *__restrict dst, const char **__restrict src,
					size_t NMC, size_t len, mbstate_t *__restrict ps)
{
	static mbstate_t mbstate;	/* Rely on bss 0-init. */
	wchar_t wcbuf[1];
	const char *s;
	size_t count;
	int incr;

	if (!ps) {
		ps = &mbstate;
	}

#ifdef __CTYPE_HAS_UTF_8_LOCALES
	if (ENCODING == __ctype_encoding_utf8) {
		size_t r;
		return ((r = _wchar_utf8sntowcs(dst, len, src, NMC, ps, 1))
				!= (size_t) -2) ? r : 0;
	}
#endif
	incr = 1;
	/* NOTE: The following is an AWFUL HACK!  In order to support %s in
	 * wprintf, we need to be able to compute the number of wchars needed
	 * for the mbs conversion, not to exceed the precision specified.
	 * But if dst is NULL, the return value is the length assuming a
	 * sufficiently sized buffer.  So, we allow passing of ((wchar_t *)ps)
	 * as dst in order to flag that we really want the length, subject
	 * to the restricted buffer size and no partial conversions.
	 * See _wchar_utf8sntowcs() as well. */
	if (!dst || (dst == ((wchar_t *)ps))) {
		if (!dst) {
			len = SIZE_MAX;
		}
		dst = wcbuf;
		incr = 0;
	}

	/* Since all the following encodings are single-byte encodings... */
	if (len > NMC) {
		len = NMC;
	}

	count = len;
	s = *src;

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	if (ENCODING == __ctype_encoding_8_bit) {
		wchar_t wc;
		while (count) {
			if ((wc = ((unsigned char)(*s))) >= 0x80) {	/* Non-ASCII... */
				wc -= 0x80;
				wc = __UCLIBC_CURLOCALE->tbl8c2wc[
						  (__UCLIBC_CURLOCALE->idx8c2wc[wc >> Cc2wc_IDX_SHIFT]
						   << Cc2wc_IDX_SHIFT) + (wc & (Cc2wc_ROW_LEN - 1))];
				if (!wc) {
					goto BAD;
				}
			}
			if (!(*dst = wc)) {
				s = NULL;
				break;
			}
			dst += incr;
			++s;
			--count;
		}
		if (dst != wcbuf) {
			*src = s;
		}
		return len - count;
	}
#endif

#ifdef __UCLIBC_HAS_LOCALE__
	assert(ENCODING == __ctype_encoding_7_bit);
#endif

	while (count) {
		if ((*dst = (unsigned char) *s) == 0) {
			s = NULL;
			break;
		}
		if (*dst >= 0x80) {
#ifdef __CTYPE_HAS_8_BIT_LOCALES
		BAD:
#endif
			__set_errno(EILSEQ);
			return (size_t) -1;
		}
		++s;
		dst += incr;
		--count;
	}
	if (dst != wcbuf) {
		*src = s;
	}
	return len - count;
}
libc_hidden_def(mbsnrtowcs)

#endif
/**********************************************************************/
#ifdef L_wcsnrtombs

/* WARNING: We treat len as SIZE_MAX when dst is NULL! */

/* Note: We completely ignore ps in all currently supported conversions.
 * TODO: Check for valid state anyway? */

size_t wcsnrtombs(char *__restrict dst, const wchar_t **__restrict src,
					size_t NWC, size_t len, mbstate_t *__restrict ps)
{
	const __uwchar_t *s;
	size_t count;
	int incr;
	char buf[MB_LEN_MAX];

#ifdef __CTYPE_HAS_UTF_8_LOCALES
	if (ENCODING == __ctype_encoding_utf8) {
		return _wchar_wcsntoutf8s(dst, len, src, NWC);
	}
#endif /* __CTYPE_HAS_UTF_8_LOCALES */

	incr = 1;
	/* NOTE: The following is an AWFUL HACK!  In order to support %ls in
	 * printf, we need to be able to compute the number of bytes needed
	 * for the mbs conversion, not to exceed the precision specified.
	 * But if dst is NULL, the return value is the length assuming a
	 * sufficiently sized buffer.  So, we allow passing of (char *) src
	 * as dst in order to flag that we really want the length, subject
	 * to the restricted buffer size and no partial conversions.
	 * See _wchar_wcsntoutf8s() as well. */
	if (!dst || (dst == ((char *) src))) {
		if (!dst) {
			len = SIZE_MAX;
		}
		dst = buf;
		incr = 0;
	}

	/* Since all the following encodings are single-byte encodings... */
	if (len > NWC) {
		len = NWC;
	}

	count = len;
	s = (const __uwchar_t *) *src;

#ifdef __CTYPE_HAS_8_BIT_LOCALES
	if (ENCODING == __ctype_encoding_8_bit) {
		__uwchar_t wc;
		__uwchar_t u;
		while (count) {
			if ((wc = *s) <= 0x7f) {
				if (!(*dst = (unsigned char) wc)) {
					s = NULL;
					break;
				}
			} else {
				u = 0;
				if (wc <= Cwc2c_DOMAIN_MAX) {
					u = __UCLIBC_CURLOCALE->idx8wc2c[wc >> (Cwc2c_TI_SHIFT
														+ Cwc2c_TT_SHIFT)];
					u = __UCLIBC_CURLOCALE->tbl8wc2c[(u << Cwc2c_TI_SHIFT)
									+ ((wc >> Cwc2c_TT_SHIFT)
									   & ((1 << Cwc2c_TI_SHIFT)-1))];
					u = __UCLIBC_CURLOCALE->tbl8wc2c[Cwc2c_TI_LEN
									+ (u << Cwc2c_TT_SHIFT)
									+ (wc & ((1 << Cwc2c_TT_SHIFT)-1))];
				}

#ifdef __WCHAR_REPLACEMENT_CHAR
				*dst = (unsigned char) ( u ? u : __WCHAR_REPLACEMENT_CHAR );
#else  /* __WCHAR_REPLACEMENT_CHAR */
				if (!u) {
					goto BAD;
				}
				*dst = (unsigned char) u;
#endif /* __WCHAR_REPLACEMENT_CHAR */
			}
			++s;
			dst += incr;
			--count;
		}
		if (dst != buf) {
			*src = (const wchar_t *) s;
		}
		return len - count;
	}
#endif /* __CTYPE_HAS_8_BIT_LOCALES */

#ifdef __UCLIBC_HAS_LOCALE__
	assert(ENCODING == __ctype_encoding_7_bit);
#endif

	while (count) {
		if (*s >= 0x80) {
#if defined(__CTYPE_HAS_8_BIT_LOCALES) && !defined(__WCHAR_REPLACEMENT_CHAR)
		BAD:
#endif
			__set_errno(EILSEQ);
			return (size_t) -1;
		}
		if ((*dst = (unsigned char) *s) == 0) {
			s = NULL;
			break;
		}
		++s;
		dst += incr;
		--count;
	}
	if (dst != buf) {
		*src = (const wchar_t *) s;
	}
	return len - count;
}
libc_hidden_def(wcsnrtombs)

#endif
/**********************************************************************/
#ifdef L_wcswidth

#if defined(__UCLIBC_HAS_LOCALE__) && \
( defined(__CTYPE_HAS_8_BIT_LOCALES) || defined(__CTYPE_HAS_UTF_8_LOCALES) )

static const unsigned char new_idx[] = {
	0,    5,    5,    6,   10,   15,   28,   39,
	48,   48,   71,   94,  113,  128,  139,  154,
	175,  186,  188,  188,  188,  188,  188,  188,
	203,  208,  208,  208,  208,  208,  208,  208,
	208,  219,  219,  219,  222,  222,  222,  222,
	222,  222,  222,  222,  222,  222,  222,  224,
	224,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  231,  231,  231,
	231,  231,  231,  231,  231,  233,  233,  233,
	233,  233,  233,  233,  234,  234,  234,  234,
	234,  234,  234,  234,  234,  234,  234,  234,
	234,  234,  234,  234,  234,  234,  234,  234,
	234,  234,  234,  234,  234,  234,  234,  234,
	234,  234,  234,  234,  234,  234,  234,  234,
	234,  234,  234,  234,  234,  234,  234,  234,
	236,  236,  236,  236,  236,  236,  236,  236,
	236,  236,  236,  236,  236,  236,  236,  236,
	236,  236,  236,  236,  236,  236,  236,  236,
	236,  236,  236,  236,  236,  236,  236,  236,
	236,  237,  237,  238,  241,  241,  242,  249,
	255,
};

static const unsigned char new_tbl[] = {
	0x00, 0x01, 0x20, 0x7f, 0xa0, 0x00, 0x00, 0x50,
	0x60, 0x70, 0x00, 0x83, 0x87, 0x88, 0x8a, 0x00,
	0x91, 0xa2, 0xa3, 0xba, 0xbb, 0xbe, 0xbf, 0xc0,
	0xc1, 0xc3, 0xc4, 0xc5, 0x00, 0x4b, 0x56, 0x70,
	0x71, 0xd6, 0xe5, 0xe7, 0xe9, 0xea, 0xee, 0x00,
	0x0f, 0x10, 0x11, 0x12, 0x30, 0x4b, 0xa6, 0xb1,
	0x00, 0x01, 0x03, 0x3c, 0x3d, 0x41, 0x49, 0x4d,
	0x4e, 0x51, 0x55, 0x62, 0x64, 0x81, 0x82, 0xbc,
	0xbd, 0xc1, 0xc5, 0xcd, 0xce, 0xe2, 0xe4, 0x00,
	0x02, 0x03, 0x3c, 0x3d, 0x41, 0x43, 0x47, 0x49,
	0x4b, 0x4e, 0x70, 0x72, 0x81, 0x83, 0xbc, 0xbd,
	0xc1, 0xc6, 0xc7, 0xc9, 0xcd, 0xce, 0x00, 0x01,
	0x02, 0x3c, 0x3d, 0x3f, 0x40, 0x41, 0x44, 0x4d,
	0x4e, 0x56, 0x57, 0x82, 0x83, 0xc0, 0xc1, 0xcd,
	0xce, 0x00, 0x3e, 0x41, 0x46, 0x49, 0x4a, 0x4e,
	0x55, 0x57, 0xbf, 0xc0, 0xc6, 0xc7, 0xcc, 0xce,
	0x00, 0x41, 0x44, 0x4d, 0x4e, 0xca, 0xcb, 0xd2,
	0xd5, 0xd6, 0xd7, 0x00, 0x31, 0x32, 0x34, 0x3b,
	0x47, 0x4f, 0xb1, 0xb2, 0xb4, 0xba, 0xbb, 0xbd,
	0xc8, 0xce, 0x00, 0x18, 0x1a, 0x35, 0x36, 0x37,
	0x38, 0x39, 0x3a, 0x71, 0x7f, 0x80, 0x85, 0x86,
	0x88, 0x90, 0x98, 0x99, 0xbd, 0xc6, 0xc7, 0x00,
	0x2d, 0x31, 0x32, 0x33, 0x36, 0x38, 0x39, 0x3a,
	0x58, 0x5a, 0x00, 0x60, 0x00, 0x12, 0x15, 0x32,
	0x35, 0x52, 0x54, 0x72, 0x74, 0xb7, 0xbe, 0xc6,
	0xc7, 0xc9, 0xd4, 0x00, 0x0b, 0x0f, 0xa9, 0xaa,
	0x00, 0x0b, 0x10, 0x2a, 0x2f, 0x60, 0x64, 0x6a,
	0x70, 0xd0, 0xeb, 0x00, 0x29, 0x2b, 0x00, 0x80,
	0x00, 0x2a, 0x30, 0x3f, 0x40, 0x99, 0x9b, 0x00,
	0xd0, 0x00, 0x00, 0xa4, 0x00, 0x00, 0x00, 0x1e,
	0x1f, 0x00, 0x00, 0x10, 0x20, 0x24, 0x30, 0x70,
	0xff, 0x00, 0x61, 0xe0, 0xe7, 0xf9, 0xfc,
};

static const signed char new_wtbl[] = {
	0,   -1,    1,   -1,    1,    1,    0,    1,
	0,    1,    1,    0,    1,    0,    1,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    0,    1,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    0,    1,    0,    1,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    1,    0,    1,    0,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    1,
	0,    1,    0,    1,    0,    1,    0,    1,
	0,    1,    2,    0,    1,    0,    1,    0,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    1,    0,    1,    0,    1,
	1,    0,    1,    0,    1,    0,    1,    0,
	1,    0,    1,    1,    2,    1,    1,    2,
	2,    0,    2,    1,    2,    0,    2,    2,
	1,    1,    2,    1,    1,    2,    1,    0,
	1,    1,    0,    1,    0,    1,    2,    1,
	0,    2,    1,    2,    1,    0,    1,
};


int wcswidth(const wchar_t *pwcs, size_t n)
{
	int h, l, m, count;
	wchar_t wc;
	unsigned char b;

	if (ENCODING == __ctype_encoding_7_bit) {
		size_t i;

		for (i = 0 ; (i < n) && pwcs[i] ; i++) {
			if (pwcs[i] != (pwcs[i] & 0x7f)) {
				return -1;
			}
		}
	}
#ifdef __CTYPE_HAS_8_BIT_LOCALES
	else if (ENCODING == __ctype_encoding_8_bit) {
		mbstate_t mbstate;

		mbstate.__mask = 0;			/* Initialize the mbstate. */
		if (wcsnrtombs(NULL, &pwcs, n, SIZE_MAX, &mbstate) == ((size_t) - 1)) {
			return -1;
		}
	}
#endif /* __CTYPE_HAS_8_BIT_LOCALES */
#if defined(__CTYPE_HAS_UTF_8_LOCALES) && defined(KUHN)
	/* For stricter handling of allowed unicode values... see comments above. */
	else if (ENCODING == __ctype_encoding_utf8) {
		size_t i;

		for (i = 0 ; (i < n) && pwcs[i] ; i++) {
			if ( (((__uwchar_t)((pwcs[i]) - 0xfffeU)) < 2)
				 || (((__uwchar_t)((pwcs[i]) - 0xd800U)) < (0xe000U - 0xd800U))
				) {
				return -1;
			}
		}
	}
#endif /* __CTYPE_HAS_UTF_8_LOCALES */

	for (count = 0 ; n && (wc = *pwcs++) ; n--) {
		if (wc <= 0xff) {
			/* If we're here, wc != 0. */
			if ((wc < 32) || ((wc >= 0x7f) && (wc < 0xa0))) {
				return -1;
			}
			++count;
			continue;
		}
		if (((unsigned int) wc) <= 0xffff) {
			b = wc & 0xff;
			h = (wc >> 8);
			l = new_idx[h];
			h = new_idx[h+1];
			while ((m = (l+h) >> 1) != l) {
				if (b >= new_tbl[m]) {
					l = m;
				} else {		/* wc < tbl[m] */
					h = m;
				}
			}
			count += new_wtbl[l]; /* none should be -1. */
			continue;
		}

		/* Redo this to minimize average number of compares?*/
		if (wc >= 0x1d167) {
			if (wc <= 0x1d1ad) {
				if ((wc <= 0x1d169
					 || (wc >= 0x1d173
						 && (wc <= 0x1d182
							 || (wc >= 0x1d185
								 && (wc <= 0x1d18b
									 || (wc >= 0x1d1aa))))))
					) {
					continue;
				}
			} else if (((wc >= 0xe0020) && (wc <= 0xe007f)) || (wc == 0xe0001)) {
				continue;
			} else if ((wc >= 0x20000) && (wc <= 0x2ffff)) {
				++count;		/* need 2.. add one here */
			}
#if (WCHAR_MAX > 0x7fffffffL)
			else if (wc > 0x7fffffffL) {
				return -1;
			}
#endif /* (WCHAR_MAX > 0x7fffffffL) */
		}

		++count;
	}

	return count;
}

#else  /*  __UCLIBC_HAS_LOCALE__ */

int wcswidth(const wchar_t *pwcs, size_t n)
{
	int count;
	wchar_t wc;
	size_t i;

	for (i = 0 ; (i < n) && pwcs[i] ; i++) {
		if (pwcs[i] != (pwcs[i] & 0x7f)) {
			return -1;
		}
	}

	for (count = 0 ; n && (wc = *pwcs++) ; n--) {
		if (wc <= 0xff) {
			/* If we're here, wc != 0. */
			if ((wc < 32) || ((wc >= 0x7f) && (wc < 0xa0))) {
				return -1;
			}
			++count;
			continue;
		} else {
			return -1;
		}
	}

	return count;
}

#endif /*  __UCLIBC_HAS_LOCALE__ */

libc_hidden_def(wcswidth)

#endif
/**********************************************************************/
#ifdef L_wcwidth


int wcwidth(wchar_t wc)
{
	return wcswidth(&wc, 1);
}

#endif
/**********************************************************************/


typedef struct {
	mbstate_t tostate;
	mbstate_t fromstate;
	int tocodeset;
	int fromcodeset;
	int frombom;
	int tobom;
	int fromcodeset0;
	int frombom0;
	int tobom0;
	int skip_invalid_input;		/* To support iconv -c option. */
} _UC_iconv_t;

/* For the multibyte
 * bit 0 means swap endian
 * bit 1 means 2 byte
 * bit 2 means 4 byte
 *
 */

#if defined L_iconv && defined _LIBC
/* Used externally only by iconv utility */
extern const unsigned char __iconv_codesets[];
libc_hidden_proto(__iconv_codesets)
#endif

#if defined L_iconv || defined L_iconv_main
# ifdef L_iconv_main
static
# endif
const unsigned char __iconv_codesets[] =
	"\x0a\xe0""WCHAR_T\x00"		/* superset of UCS-4 but platform-endian */
#if __BYTE_ORDER == __BIG_ENDIAN
	"\x08\xec""UCS-4\x00"		/* always BE */
	"\x0a\xec""UCS-4BE\x00"
	"\x0a\xed""UCS-4LE\x00"
	"\x09\xe4""UTF-32\x00"		/* platform endian with BOM */
	"\x0b\xe4""UTF-32BE\x00"
	"\x0b\xe5""UTF-32LE\x00"
	"\x08\xe2""UCS-2\x00"		/* always BE */
	"\x0a\xe2""UCS-2BE\x00"
	"\x0a\xe3""UCS-2LE\x00"
	"\x09\xea""UTF-16\x00"		/* platform endian with BOM */
	"\x0b\xea""UTF-16BE\x00"
	"\x0b\xeb""UTF-16LE\x00"
#elif __BYTE_ORDER == __LITTLE_ENDIAN
	"\x08\xed""UCS-4\x00"		/* always BE */
	"\x0a\xed""UCS-4BE\x00"
	"\x0a\xec""UCS-4LE\x00"
	"\x09\xf4""UTF-32\x00"		/* platform endian with BOM */
	"\x0b\xe5""UTF-32BE\x00"
	"\x0b\xe4""UTF-32LE\x00"
	"\x08\xe3""UCS-2\x00"		/* always BE */
	"\x0a\xe3""UCS-2BE\x00"
	"\x0a\xe2""UCS-2LE\x00"
	"\x09\xfa""UTF-16\x00"		/* platform endian with BOM */
	"\x0b\xeb""UTF-16BE\x00"
	"\x0b\xea""UTF-16LE\x00"
#endif
	"\x08\x02""UTF-8\x00"
	"\x0b\x01""US-ASCII\x00"
	"\x07\x01""ASCII";			/* Must be last! (special case to save a nul) */
#endif
#if defined L_iconv && defined _LIBC
libc_hidden_data_def(__iconv_codesets)
#endif


#ifdef L_iconv

#include <iconv.h>
#include <string.h>
#include <endian.h>
#include <byteswap.h>

#if (__BYTE_ORDER != __BIG_ENDIAN) && (__BYTE_ORDER != __LITTLE_ENDIAN)
#error unsupported endianness for iconv
#endif

#ifndef __CTYPE_HAS_8_BIT_LOCALES
#error currently iconv requires 8 bit locales
#endif
#ifndef __CTYPE_HAS_UTF_8_LOCALES
#error currently iconv requires UTF-8 locales
#endif


enum {
	IC_WCHAR_T = 0xe0,
	IC_MULTIBYTE = 0xe0,
#if __BYTE_ORDER == __BIG_ENDIAN
	IC_UCS_4 =	0xec,
	IC_UTF_32 = 0xe4,
	IC_UCS_2 =	0xe2,
	IC_UTF_16 = 0xea,
#else
	IC_UCS_4 =	0xed,
	IC_UTF_32 = 0xe5,
	IC_UCS_2 =	0xe3,
	IC_UTF_16 = 0xeb,
#endif
	IC_UTF_8 = 2,
	IC_ASCII = 1
};


static int find_codeset(const char *name)
{
	const unsigned char *s;
	int codeset;

	for (s = __iconv_codesets; *s; s += *s) {
		if (!strcasecmp((char*) (s + 2), name)) {
			return s[1];
		}
	}

	/* The following is ripped from find_locale in locale.c. */

	/* TODO: maybe CODESET_LIST + *s ??? */
	/* 7bit is 1, UTF-8 is 2, 8-bit is >= 3 */
	codeset = 2;
	s = (const unsigned char *) __LOCALE_DATA_CODESET_LIST;
	do {
		++codeset;		/* Increment codeset first. */
		if (!strcasecmp(__LOCALE_DATA_CODESET_LIST+*s, name)) {
			return codeset;
		}
	} while (*++s);

	return 0;			/* No matching codeset! */
}

iconv_t weak_function iconv_open(const char *tocode, const char *fromcode)
{
	register _UC_iconv_t *px;
	int tocodeset, fromcodeset;

	if (((tocodeset = find_codeset(tocode)) != 0)
		&& ((fromcodeset = find_codeset(fromcode)) != 0)) {
		if ((px = malloc(sizeof(_UC_iconv_t))) != NULL) {
			px->tocodeset = tocodeset;
			px->tobom0 = px->tobom = (tocodeset >= 0xe0) ? (tocodeset & 0x10) >> 4 : 0;
			px->fromcodeset0 = px->fromcodeset = fromcodeset;
			px->frombom0 = px->frombom = (fromcodeset >= 0xe0) ? (fromcodeset & 0x10) >> 4 : 0;
			px->skip_invalid_input = px->tostate.__mask
				= px->fromstate.__mask = 0;
			return (iconv_t) px;
		}
	} else {
		__set_errno(EINVAL);
	}
	return (iconv_t)(-1);
}

int weak_function iconv_close(iconv_t cd)
{
	free(cd);

	return 0;
}

size_t weak_function iconv(iconv_t cd, char **__restrict inbuf,
						   size_t *__restrict inbytesleft,
						   char **__restrict outbuf,
						   size_t *__restrict outbytesleft)
{
	_UC_iconv_t *px = (_UC_iconv_t *) cd;
	size_t nrcount, r;
	wchar_t wc, wc2;
	int inci, inco;

	assert(px != (_UC_iconv_t *)(-1));
	assert(sizeof(wchar_t) == 4);

	if (!inbuf || !*inbuf) {	/* Need to reinitialze conversion state. */
		/* Note: For shift-state encodings we possibly need to output the
		 * shift sequence to return to initial state! */
		if ((px->fromcodeset & 0xf0) == 0xe0) {
		}
		px->tostate.__mask = px->fromstate.__mask = 0;
		px->fromcodeset = px->fromcodeset0;
		px->tobom = px->tobom0;
		px->frombom = px->frombom0;
		return 0;
	}

	nrcount = 0;
	while (*inbytesleft) {
		if (!*outbytesleft) {
		TOO_BIG:
			__set_errno(E2BIG);
			return (size_t) -1;
		}

		inci = inco = 1;
		if (px->fromcodeset >= IC_MULTIBYTE) {
			inci = (px->fromcodeset == IC_WCHAR_T) ? 4: (px->fromcodeset & 6);
			if (*inbytesleft < inci) goto INVALID;
			wc = (((unsigned int)((unsigned char)((*inbuf)[0]))) << 8)
				+ ((unsigned char)((*inbuf)[1]));
			if (inci == 4) {
				wc = (((unsigned int)((unsigned char)((*inbuf)[2]))) << 8)
					+ ((unsigned char)((*inbuf)[3])) + (wc << 16);
				if (!(px->fromcodeset & 1)) wc = bswap_32(wc);
			} else {
				if (!(px->fromcodeset & 1)) wc = bswap_16(wc);
				if (((px->fromcodeset & IC_UTF_16) == IC_UTF_16)
					 && (((__uwchar_t)(wc - 0xd800U)) < (0xdc00U - 0xd800U))
					) {			/* surrogate */
					wc =- 0xd800U;
					if (*inbytesleft < 4) goto INVALID;
					wc2 = (((unsigned int)((unsigned char)((*inbuf)[2]))) << 8)
						+ ((unsigned char)((*inbuf)[3]));
					if (!(px->fromcodeset & 1)) wc = bswap_16(wc2);
					if (((__uwchar_t)(wc2 -= 0xdc00U)) < (0xe0000U - 0xdc00U)) {
						goto ILLEGAL;
					}
					inci = 4;	/* Change inci here in case skipping illegals. */
					wc = 0x10000UL + (wc << 10) + wc2;
				}
			}

			if (px->frombom) {
				px->frombom = 0;
				if ((wc == 0xfeffU)
					|| (wc == ((inci == 4)
							   ? (((wchar_t) 0xfffe0000UL))
							   : ((wchar_t)(0xfffeUL))))
					) {
					if (wc != 0xfeffU) {
						px->fromcodeset ^= 1; /* toggle endianness */
						wc = 0xfeffU;
					}
					if (!px->frombom) {
						goto BOM_SKIP_OUTPUT;
					}
					goto GOT_BOM;
				}
			}

			if (px->fromcodeset != IC_WCHAR_T) {
				if (((__uwchar_t) wc) > (((px->fromcodeset & IC_UCS_4) == IC_UCS_4)
										 ? 0x7fffffffUL : 0x10ffffUL)
#ifdef KUHN
					|| (((__uwchar_t)(wc - 0xfffeU)) < 2)
					|| (((__uwchar_t)(wc - 0xd800U)) < (0xe000U - 0xd800U))
#endif
					) {
					goto ILLEGAL;
				}
			}
		} else if (px->fromcodeset == IC_UTF_8) {
			const char *p = *inbuf;
			r = _wchar_utf8sntowcs(&wc, 1, &p, *inbytesleft, &px->fromstate, 0);
			if (((ssize_t) r) <= 0) { /* either EILSEQ or incomplete or nul */
				if (((ssize_t) r) < 0) { /* either EILSEQ or incomplete or nul */
					assert((r == (size_t)(-1)) || (r == (size_t)(-2)));
					if (r == (size_t)(-2)) {
					INVALID:
						__set_errno(EINVAL);
					} else {
						px->fromstate.__mask = 0;
						inci = 1;
					ILLEGAL:
						if (px->skip_invalid_input) {
							px->skip_invalid_input = 2;	/* flag for iconv utility */
							goto BOM_SKIP_OUTPUT;
						}
						__set_errno(EILSEQ);
					}
					return (size_t)(-1);
				}
				if (p != NULL) { /* incomplete char case */
					goto INVALID;
				}
				p = *inbuf + 1;	/* nul */
			}
			inci = p - *inbuf;
		} else if ((wc = ((unsigned char)(**inbuf))) >= 0x80) {	/* Non-ASCII... */
			if (px->fromcodeset == IC_ASCII) { /* US-ASCII codeset */
				goto ILLEGAL;
			} else {			/* some other 8-bit ascii-extension codeset */
				const __codeset_8_bit_t *c8b
					= __locale_mmap->codeset_8_bit + px->fromcodeset - 3;
				wc -= 0x80;
				wc = __UCLIBC_CURLOCALE->tbl8c2wc[
							 (c8b->idx8c2wc[wc >> Cc2wc_IDX_SHIFT]
							  << Cc2wc_IDX_SHIFT) + (wc & (Cc2wc_ROW_LEN - 1))];
				if (!wc) {
					goto ILLEGAL;
				}
			}
		}


		if (px->tobom) {
			inci = 0;
			wc = 0xfeffU;
	GOT_BOM:
			px->tobom = 0;
		}

		if (px->tocodeset >= IC_MULTIBYTE) {
			inco = (px->tocodeset == IC_WCHAR_T) ? 4: (px->tocodeset & 6);
			if (*outbytesleft < inco) goto TOO_BIG;
			if (px->tocodeset != IC_WCHAR_T) {
				if (((__uwchar_t) wc) > (((px->tocodeset & IC_UCS_4) == IC_UCS_4)
										 ? 0x7fffffffUL : 0x10ffffUL)
#ifdef KUHN
					|| (((__uwchar_t)(wc - 0xfffeU)) < 2)
					|| (((__uwchar_t)(wc - 0xd800U)) < (0xe000U - 0xd800U))
#endif
					) {
				REPLACE_32:
					wc = 0xfffd;
					++nrcount;
				}
			}
			if (inco == 4) {
				if (px->tocodeset & 1) wc = bswap_32(wc);
			} else {
				if (((__uwchar_t)wc ) > 0xffffU) {
					if ((px->tocodeset & IC_UTF_16) != IC_UTF_16) {
						goto REPLACE_32;
					}
					if (*outbytesleft < (inco = 4)) goto TOO_BIG;
					wc2 = 0xdc00U + (wc & 0x3ff);
					wc = 0xd800U + ((wc >> 10) & 0x3ff);
					if (px->tocodeset & 1) {
						wc = bswap_16(wc);
						wc2 = bswap_16(wc2);
					}
					wc += (wc2 << 16);
				} else if (px->tocodeset & 1) wc = bswap_16(wc);
			}
			(*outbuf)[0] = (char)((unsigned char)(wc));
			(*outbuf)[1] = (char)((unsigned char)(wc >> 8));
			if (inco == 4) {
				(*outbuf)[2] = (char)((unsigned char)(wc >> 16));
				(*outbuf)[3] = (char)((unsigned char)(wc >> 24));
			}
		} else if (px->tocodeset == IC_UTF_8) {
			const wchar_t *pw = &wc;
			do {
				r = _wchar_wcsntoutf8s(*outbuf, *outbytesleft, &pw, 1);
				if (r != (size_t)(-1)) {
					if (r == 0) {
						if (wc != 0) {
							goto TOO_BIG;
						}
						++r;
					}
					break;
				}
				wc = 0xfffdU;
				++nrcount;
			} while (1);
			inco = r;
		} else if (((__uwchar_t)(wc)) < 0x80) {
		CHAR_GOOD:
				**outbuf = wc;
		} else {
			if ((px->tocodeset != 0x01) && (wc <= Cwc2c_DOMAIN_MAX)) {
				const __codeset_8_bit_t *c8b
					= __locale_mmap->codeset_8_bit + px->tocodeset - 3;
				__uwchar_t u;
				u = c8b->idx8wc2c[wc >> (Cwc2c_TI_SHIFT + Cwc2c_TT_SHIFT)];
				u = __UCLIBC_CURLOCALE->tbl8wc2c[(u << Cwc2c_TI_SHIFT)
						 + ((wc >> Cwc2c_TT_SHIFT)
							& ((1 << Cwc2c_TI_SHIFT)-1))];
				wc = __UCLIBC_CURLOCALE->tbl8wc2c[Cwc2c_TI_LEN
						 + (u << Cwc2c_TT_SHIFT)
						 + (wc & ((1 << Cwc2c_TT_SHIFT)-1))];
				if (wc) {
					goto CHAR_GOOD;
				}
			}
			**outbuf = '?';
			++nrcount;
		}

		*outbuf += inco;
		*outbytesleft -= inco;
	BOM_SKIP_OUTPUT:
		*inbuf += inci;
		*inbytesleft -= inci;
	}
	return nrcount;
}
#endif
