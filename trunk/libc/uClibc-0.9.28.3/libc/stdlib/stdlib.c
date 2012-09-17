/*  Copyright (C) 2002     Manuel Novoa III
 *  From my (incomplete) stdlib library for linux and (soon) elks.
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
 *  This code is currently under development.  Also, I plan to port
 *  it to elks which is a 16-bit environment with a fairly limited
 *  compiler.  Therefore, please refrain from modifying this code
 *  and, instead, pass any bug-fixes, etc. to me.  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */

/* Oct 29, 2002
 * Fix a couple of 'restrict' bugs in mbstowcs and wcstombs.
 *
 * Nov 21, 2002
 * Add wscto{inttype} functions.
 */

#define _ISOC99_SOURCE			/* for ULLONG primarily... */
#define _GNU_SOURCE
#include <limits.h>
#include <stdint.h>
#include <inttypes.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>
#include <unistd.h>

/* Work around gcc's refusal to create aliases. 
 * TODO: Add in a define to disable the aliases? */

#if UINT_MAX == ULONG_MAX
#define atoi __ignore_atoi
#define abs __ignore_abs
#endif
#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
#define llabs __ignore_llabs
#define atoll __ignore_atoll
#define strtoll __ignore_strtoll
#define strtoull __ignore_strtoull
#define wcstoll __ignore_wcstoll
#define wcstoull __ignore_wcstoull
#define strtoll_l __ignore_strtoll_l
#define strtoull_l __ignore_strtoull_l
#define wcstoll_l __ignore_wcstoll_l
#define wcstoull_l __ignore_wcstoull_l
#endif

#include <stdlib.h>
#include <locale.h>

#ifdef __UCLIBC_HAS_WCHAR__

#include <wchar.h>
#include <wctype.h>
#include <bits/uClibc_uwchar.h>

#ifdef __UCLIBC_HAS_XLOCALE__
#include <xlocale.h>
#endif /* __UCLIBC_HAS_XLOCALE__ */

/* TODO: clean up the following... */

#if WCHAR_MAX > 0xffffUL
#define UTF_8_MAX_LEN 6
#else
#define UTF_8_MAX_LEN 3
#endif

#ifdef __UCLIBC_HAS_LOCALE__

#define ENCODING		((__UCLIBC_CURLOCALE_DATA).encoding)
#ifndef __CTYPE_HAS_UTF_8_LOCALES
#ifdef L_mblen
/* emit only once */
#warning __CTYPE_HAS_UTF_8_LOCALES not set!
#endif
#endif

#else  /* __UCLIBC_HAS_LOCALE__ */

#ifdef __UCLIBC_MJN3_ONLY__
#ifdef L_mblen
/* emit only once */
#warning devel checks
#endif
#endif
#ifdef __CTYPE_HAS_8_BIT_LOCALES
#error __CTYPE_HAS_8_BIT_LOCALES is defined!
#endif
#ifdef __CTYPE_HAS_UTF_8_LOCALES
#error __CTYPE_HAS_UTF_8_LOCALES is defined!
#endif
#endif

#endif /* __UCLIBC_HAS_LOCALE__ */

#if UINT_MAX == ULONG_MAX
#undef atoi
#undef abs
#endif
#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
#undef llabs
#undef atoll
#undef strtoll
#undef strtoull
#undef wcstoll
#undef wcstoull
#undef strtoll_l
#undef strtoull_l
#undef wcstoll_l
#undef wcstoull_l
#endif /* __UCLIBC_HAS_WCHAR__ */

/**********************************************************************/
#ifdef __UCLIBC_HAS_XLOCALE__

extern unsigned long
_stdlib_strto_l_l(register const char * __restrict str,
				  char ** __restrict endptr, int base, int sflag,
				  __locale_t locale_arg);

#if defined(ULLONG_MAX)
extern unsigned long long
_stdlib_strto_ll_l(register const char * __restrict str,
				   char ** __restrict endptr, int base, int sflag,
				  __locale_t locale_arg);
#endif

#ifdef __UCLIBC_HAS_WCHAR__
extern unsigned long
_stdlib_wcsto_l_l(register const wchar_t * __restrict str,
				  wchar_t ** __restrict endptr, int base, int sflag,
				  __locale_t locale_arg);

#if defined(ULLONG_MAX)
extern unsigned long long
_stdlib_wcsto_ll_l(register const wchar_t * __restrict str,
				   wchar_t ** __restrict endptr, int base, int sflag,
				  __locale_t locale_arg);
#endif
#endif /* __UCLIBC_HAS_WCHAR__ */

#endif /* __UCLIBC_HAS_XLOCALE__ */



extern unsigned long
_stdlib_strto_l(register const char * __restrict str,
				char ** __restrict endptr, int base, int sflag);

#if defined(ULLONG_MAX)
extern unsigned long long
_stdlib_strto_ll(register const char * __restrict str,
				 char ** __restrict endptr, int base, int sflag);
#endif

#ifdef __UCLIBC_HAS_WCHAR__
extern unsigned long
_stdlib_wcsto_l(register const wchar_t * __restrict str,
				wchar_t ** __restrict endptr, int base, int sflag);

#if defined(ULLONG_MAX)
extern unsigned long long
_stdlib_wcsto_ll(register const wchar_t * __restrict str,
				 wchar_t ** __restrict endptr, int base, int sflag);
#endif
#endif /* __UCLIBC_HAS_WCHAR__ */
/**********************************************************************/
#ifdef L_atof

double atof(const char *nptr)
{
	return strtod(nptr, (char **) NULL);
}

#endif
/**********************************************************************/
#ifdef L_abs

#if INT_MAX < LONG_MAX

int abs(int j)
{
	return (j >= 0) ? j : -j;
}

#endif /* INT_MAX < LONG_MAX */

#endif
/**********************************************************************/
#ifdef L_labs

#if UINT_MAX == ULONG_MAX
strong_alias(labs,abs)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(labs,llabs)
#endif

#if ULONG_MAX == UINTMAX_MAX
strong_alias(labs,imaxabs)
#endif

long int labs(long int j)
{
	return (j >= 0) ? j : -j;
}

#endif
/**********************************************************************/
#ifdef L_llabs

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if (ULLONG_MAX == UINTMAX_MAX)
strong_alias(llabs,imaxabs)
#endif

long long int llabs(long long int j)
{
	return (j >= 0) ? j : -j;
}

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
#ifdef L_atoi

#if INT_MAX < LONG_MAX 

int atoi(const char *nptr)
{
	return (int) strtol(nptr, (char **) NULL, 10);
}

#endif /* INT_MAX < LONG_MAX  */

#endif
/**********************************************************************/
#ifdef L_atol

#if UINT_MAX == ULONG_MAX
strong_alias(atol,atoi)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(atol,atoll)
#endif

long atol(const char *nptr)
{
	return strtol(nptr, (char **) NULL, 10);
}

#endif
/**********************************************************************/
#ifdef L_atoll

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

long long atoll(const char *nptr)
{
	return strtoll(nptr, (char **) NULL, 10);
}

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
#if defined(L_strtol) || defined(L_strtol_l)

#if (ULONG_MAX == UINTMAX_MAX) && !defined(L_strtol_l)
strong_alias(strtol,strtoimax)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(__XL(strtol),__XL(strtoll))
#endif

long __XL(strtol)(const char * __restrict str, char ** __restrict endptr,
				  int base   __LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_strto_l)(str, endptr, base, 1   __LOCALE_ARG );
}

__XL_ALIAS(strtol)

#endif
/**********************************************************************/
#if defined(L_strtoll) || defined(L_strtoll_l)

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if !defined(L_strtoll_l)
#if (ULLONG_MAX == UINTMAX_MAX)
strong_alias(strtoll,strtoimax)
#endif
strong_alias(strtoll,strtoq)
#endif

long long __XL(strtoll)(const char * __restrict str,
						char ** __restrict endptr, int base
						__LOCALE_PARAM )
{
    return (long long) __XL_NPP(_stdlib_strto_ll)(str, endptr, base, 1
												  __LOCALE_ARG );
}

__XL_ALIAS(strtoll)

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
#if defined(L_strtoul) || defined(L_strtoul_l)

#if (ULONG_MAX == UINTMAX_MAX) && !defined(L_strtoul_l)
strong_alias(strtoul,strtoumax)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(__XL(strtoul),__XL(strtoull))
#endif

unsigned long __XL(strtoul)(const char * __restrict str,
							char ** __restrict endptr, int base
							__LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_strto_l)(str, endptr, base, 0   __LOCALE_ARG );
}

__XL_ALIAS(strtoul)

#endif
/**********************************************************************/
#if defined(L_strtoull) || defined(L_strtoull_l)

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if !defined(L_strtoull_l)
#if (ULLONG_MAX == UINTMAX_MAX)
strong_alias(strtoull,strtoumax)
#endif
strong_alias(strtoull,strtouq)
#endif

unsigned long long __XL(strtoull)(const char * __restrict str,
								  char ** __restrict endptr, int base
								  __LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_strto_ll)(str, endptr, base, 0   __LOCALE_ARG );
}

__XL_ALIAS(strtoull)

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
/* Support routines follow */
/**********************************************************************/
/* Set if we want errno set appropriately. */
/* NOTE: Implies _STRTO_ENDPTR below */
#define _STRTO_ERRNO            1

/* Set if we want support for the endptr arg. */
/* Implied by _STRTO_ERRNO. */
#define _STRTO_ENDPTR           1

#if _STRTO_ERRNO
#undef _STRTO_ENDPTR
#define _STRTO_ENDPTR           1
#define SET_ERRNO(X)            __set_errno(X)
#else
#define SET_ERRNO(X)            ((void)(X))	/* keep side effects */
#endif

/**********************************************************************/
#if defined(L__stdlib_wcsto_l) || defined(L__stdlib_wcsto_l_l)
#define L__stdlib_strto_l
#endif

#if defined(L__stdlib_strto_l) || defined(L__stdlib_strto_l_l)

#if defined(L__stdlib_wcsto_l) || defined(L__stdlib_wcsto_l_l)

#define _stdlib_strto_l _stdlib_wcsto_l
#define _stdlib_strto_l_l _stdlib_wcsto_l_l
#define Wchar wchar_t
#define Wuchar __uwchar_t
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) iswspace_l((C), locale_arg)
#else
#define ISSPACE(C) iswspace((C))
#endif

#else  /* defined(L__stdlib_wcsto_l) || defined(L__stdlib_wcsto_l_l) */

#define Wchar char
#define Wuchar unsigned char
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) isspace_l((C), locale_arg)
#else
#define ISSPACE(C) isspace((C))
#endif

#endif /* defined(L__stdlib_wcsto_l) || defined(L__stdlib_wcsto_l_l) */

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

unsigned long _stdlib_strto_l(register const Wchar * __restrict str,
							  Wchar ** __restrict endptr, int base,
							  int sflag)
{
	return _stdlib_strto_l_l(str, endptr, base, sflag, __UCLIBC_CURLOCALE);
}


#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

/* This is the main work fuction which handles both strtol (sflag = 1) and
 * strtoul (sflag = 0). */

unsigned long __XL_NPP(_stdlib_strto_l)(register const Wchar * __restrict str,
										Wchar ** __restrict endptr, int base,
										int sflag   __LOCALE_PARAM )
{
    unsigned long number, cutoff;
#if _STRTO_ENDPTR
    const Wchar *fail_char;
#define SET_FAIL(X)       fail_char = (X)
#else
#define SET_FAIL(X)       ((void)(X)) /* Keep side effects. */
#endif
    unsigned char negative, digit, cutoff_digit;

	assert(((unsigned int)sflag) <= 1);

	SET_FAIL(str);

    while (ISSPACE(*str)) { /* Skip leading whitespace. */
		++str;
    }

    /* Handle optional sign. */
    negative = 0;
    switch(*str) {
		case '-': negative = 1;	/* Fall through to increment str. */
		case '+': ++str;
    }

    if (!(base & ~0x10)) {		/* Either dynamic (base = 0) or base 16. */
		base += 10;				/* Default is 10 (26). */
		if (*str == '0') {
			SET_FAIL(++str);
			base -= 2;			/* Now base is 8 or 16 (24). */
			if ((0x20|(*str)) == 'x') { /* WARNING: assumes ascii. */
				++str;
				base += base;	/* Base is 16 (16 or 48). */
			}
		}

		if (base > 16) {		/* Adjust in case base wasn't dynamic. */
			base = 16;
		}
    }

	number = 0;

    if (((unsigned)(base - 2)) < 35) { /* Legal base. */
		cutoff_digit = ULONG_MAX % base;
		cutoff = ULONG_MAX / base;
		do {
			digit = (((Wuchar)(*str - '0')) <= 9)
				? (*str - '0')
				: ((*str >= 'A')
				   ? (((0x20|(*str)) - 'a' + 10)) /* WARNING: assumes ascii. */
					  : 40);

			if (digit >= base) {
				break;
			}

			SET_FAIL(++str);

			if ((number > cutoff)
				|| ((number == cutoff) && (digit > cutoff_digit))) {
				number = ULONG_MAX;
				negative &= sflag;
				SET_ERRNO(ERANGE);
			} else {
				number = number * base + digit;
			}
		} while (1);
	}

#if _STRTO_ENDPTR
    if (endptr) {
		*endptr = (Wchar *) fail_char;
    }
#endif

	{
		unsigned long tmp = ((negative)
							 ? ((unsigned long)(-(1+LONG_MIN)))+1
							 : LONG_MAX);
		if (sflag && (number > tmp)) {
			number = tmp;
			SET_ERRNO(ERANGE);
		}
	}

	return negative ? (unsigned long)(-((long)number)) : number;
}

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */


#endif
/**********************************************************************/
#if defined(L__stdlib_wcsto_ll) || defined(L__stdlib_wcsto_ll_l)
#define L__stdlib_strto_ll
#endif

#if defined(L__stdlib_strto_ll) || defined(L__stdlib_strto_ll_l)

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if defined(L__stdlib_wcsto_ll) || defined(L__stdlib_wcsto_ll_l)
#define _stdlib_strto_ll _stdlib_wcsto_ll
#define _stdlib_strto_ll_l _stdlib_wcsto_ll_l
#define Wchar wchar_t
#define Wuchar __uwchar_t
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) iswspace_l((C), locale_arg)
#else
#define ISSPACE(C) iswspace((C))
#endif

#else  /* defined(L__stdlib_wcsto_ll) || defined(L__stdlib_wcsto_ll_l) */

#define Wchar char
#define Wuchar unsigned char
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) isspace_l((C), locale_arg)
#else
#define ISSPACE(C) isspace((C))
#endif

#endif /* defined(L__stdlib_wcsto_ll) || defined(L__stdlib_wcsto_ll_l) */

#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

unsigned long long _stdlib_strto_ll(register const Wchar * __restrict str,
									Wchar ** __restrict endptr, int base,
									int sflag)
{
	return _stdlib_strto_ll_l(str, endptr, base, sflag, __UCLIBC_CURLOCALE);
}


#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

/* This is the main work fuction which handles both strtoll (sflag = 1) and
 * strtoull (sflag = 0). */

unsigned long long __XL_NPP(_stdlib_strto_ll)(register const Wchar * __restrict str,
											  Wchar ** __restrict endptr, int base,
											  int sflag   __LOCALE_PARAM )
{
    unsigned long long number;
#if _STRTO_ENDPTR
    const Wchar *fail_char;
#define SET_FAIL(X)       fail_char = (X)
#else
#define SET_FAIL(X)       ((void)(X)) /* Keep side effects. */
#endif
	unsigned int n1;
    unsigned char negative, digit;

	assert(((unsigned int)sflag) <= 1);

	SET_FAIL(str);

    while (ISSPACE(*str)) {		/* Skip leading whitespace. */
		++str;
    }

    /* Handle optional sign. */
    negative = 0;
    switch(*str) {
		case '-': negative = 1;	/* Fall through to increment str. */
		case '+': ++str;
    }

    if (!(base & ~0x10)) {		/* Either dynamic (base = 0) or base 16. */
		base += 10;				/* Default is 10 (26). */
		if (*str == '0') {
			SET_FAIL(++str);
			base -= 2;			/* Now base is 8 or 16 (24). */
			if ((0x20|(*str)) == 'x') { /* WARNING: assumes ascii. */
				++str;
				base += base;	/* Base is 16 (16 or 48). */
			}
		}

		if (base > 16) {		/* Adjust in case base wasn't dynamic. */
			base = 16;
		}
    }

	number = 0;

    if (((unsigned)(base - 2)) < 35) { /* Legal base. */
		do {
			digit = (((Wuchar)(*str - '0')) <= 9)
				? (*str - '0')
				: ((*str >= 'A')
				   ? (((0x20|(*str)) - 'a' + 10)) /* WARNING: assumes ascii. */
					  : 40);

			if (digit >= base) {
				break;
			}

			SET_FAIL(++str);

#if 1
			/* Optional, but speeds things up in the usual case. */
			if (number <= (ULLONG_MAX >> 6)) {
				number = number * base + digit;
			} else
#endif
			{
				n1 = ((unsigned char) number) * base + digit;
				number = (number >> CHAR_BIT) * base;

				if (number + (n1 >> CHAR_BIT) <= (ULLONG_MAX >> CHAR_BIT)) {
					number = (number << CHAR_BIT) + n1;
				} else {		/* Overflow. */
					number = ULLONG_MAX;
					negative &= sflag;
					SET_ERRNO(ERANGE);
				}
			}

		} while (1);
	}

#if _STRTO_ENDPTR
    if (endptr) {
		*endptr = (Wchar *) fail_char;
    }
#endif

	{
		unsigned long long tmp = ((negative)
								  ? ((unsigned long long)(-(1+LLONG_MIN)))+1
								  : LLONG_MAX);
		if (sflag && (number > tmp)) {
			number = tmp;
			SET_ERRNO(ERANGE);
		}
	}

	return negative ? (unsigned long long)(-((long long)number)) : number;
}

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
/* Made _Exit() an alias for _exit(), as per C99. */
/*  #ifdef L__Exit */

/*  void _Exit(int status) */
/*  { */
/*  	_exit(status); */
/*  } */

/*  #endif */
/**********************************************************************/
#ifdef L_bsearch

void *bsearch(const void *key, const void *base, size_t /* nmemb */ high,
			  size_t size, int (*compar)(const void *, const void *))
{
	register char *p;
	size_t low;
	size_t mid;
	int r;

	if (size > 0) {				/* TODO: change this to an assert?? */
		low = 0;
		while (low < high) {
			mid = low + ((high - low) >> 1); /* Avoid possible overflow here. */
			p = ((char *)base) + mid * size; /* Could overflow here... */
			r = (*compar)(key, p); /* but that's an application problem! */
			if (r > 0) {
				low = mid + 1;
			} else if (r < 0) {
				high = mid;
			} else {
				return p;
			}
		}
	}
	return NULL;
}

#endif
/**********************************************************************/
#ifdef L_qsort

/* This code is derived from a public domain shell sort routine by
 * Ray Gardner and found in Bob Stout's snippets collection.  The
 * original code is included below in an #if 0/#endif block.
 *
 * I modified it to avoid the possibility of overflow in the wgap
 * calculation, as well as to reduce the generated code size with
 * bcc and gcc. */

void qsort (void  *base,
            size_t nel,
            size_t width,
            int (*comp)(const void *, const void *))
{
	size_t wgap, i, j, k;
	char tmp;

	if ((nel > 1) && (width > 0)) {
		assert( nel <= ((size_t)(-1)) / width ); /* check for overflow */
		wgap = 0;
		do {
			wgap = 3 * wgap + 1;
		} while (wgap < (nel-1)/3);
		/* From the above, we know that either wgap == 1 < nel or */
		/* ((wgap-1)/3 < (int) ((nel-1)/3) <= (nel-1)/3 ==> wgap <  nel. */
		wgap *= width;			/* So this can not overflow if wnel doesn't. */
		nel *= width;			/* Convert nel to 'wnel' */
		do {
			i = wgap;
			do {
				j = i;
				do {
					register char *a;
					register char *b;

					j -= wgap;
					a = j + ((char *)base);
					b = a + wgap;
					if ( (*comp)(a, b) <= 0 ) {
						break;
					}
					k = width;
					do {
						tmp = *a;
						*a++ = *b;
						*b++ = tmp;
					} while ( --k );
				} while (j >= wgap);
				i += width;
			} while (i < nel);
			wgap = (wgap - width)/3;
		} while (wgap);
	}
}

/* ---------- original snippets version below ---------- */

#if 0
/*
**  ssort()  --  Fast, small, qsort()-compatible Shell sort
**
**  by Ray Gardner,  public domain   5/90
*/

#include <stddef.h>

void ssort (void  *base,
            size_t nel,
            size_t width,
            int (*comp)(const void *, const void *))
{
      size_t wnel, gap, wgap, i, j, k;
      char *a, *b, tmp;

      wnel = width * nel;
      for (gap = 0; ++gap < nel;)
            gap *= 3;
      while ( gap /= 3 )
      {
            wgap = width * gap;
            for (i = wgap; i < wnel; i += width)
            {
                  for (j = i - wgap; ;j -= wgap)
                  {
                        a = j + (char *)base;
                        b = a + wgap;
                        if ( (*comp)(a, b) <= 0 )
                              break;
                        k = width;
                        do
                        {
                              tmp = *a;
                              *a++ = *b;
                              *b++ = tmp;
                        } while ( --k );
                        if (j < wgap)
                              break;
                  }
            }
      }
}
#endif

#endif
/**********************************************************************/
#ifdef L__stdlib_mb_cur_max

size_t _stdlib_mb_cur_max(void)
{
#ifdef __CTYPE_HAS_UTF_8_LOCALES
	return __UCLIBC_CURLOCALE_DATA.mb_cur_max;
#else
#ifdef __CTYPE_HAS_8_BIT_LOCALES
#ifdef __UCLIBC_MJN3_ONLY__
#warning need to change this when/if transliteration is implemented
#endif
#endif
	return 1;
#endif
}

#endif
/**********************************************************************/
#ifdef L_mblen

int mblen(register const char *s, size_t n)
{
	static mbstate_t state;
	size_t r;

	if (!s) {
		state.__mask = 0;
#ifdef __CTYPE_HAS_UTF_8_LOCALES
		return ENCODING == __ctype_encoding_utf8;
#else
		return 0;
#endif
	}

	if ((r = mbrlen(s, n, &state)) == (size_t) -2) {
		/* TODO: Should we set an error state? */
		state.__wc = 0xffffU;	/* Make sure we're in an error state. */
		return (size_t) -1;		/* TODO: Change error code above? */
	}
	return r;
}

#endif
/**********************************************************************/
#ifdef L_mbtowc

int mbtowc(wchar_t *__restrict pwc, register const char *__restrict s, size_t n)
{
	static mbstate_t state;
	size_t r;

	if (!s) {
		state.__mask = 0;
#ifdef __CTYPE_HAS_UTF_8_LOCALES
		return ENCODING == __ctype_encoding_utf8;
#else
		return 0;
#endif
	}

	if ((r = mbrtowc(pwc, s, n, &state)) == (size_t) -2) {
		/* TODO: Should we set an error state? */
		state.__wc = 0xffffU;	/* Make sure we're in an error state. */
		return (size_t) -1;		/* TODO: Change error code above? */
	}
	return r;
}

#endif
/**********************************************************************/
#ifdef L_wctomb

/* Note: We completely ignore state in all currently supported conversions. */

int wctomb(register char *__restrict s, wchar_t swc)
{
	return (!s)
		?
#ifdef __CTYPE_HAS_UTF_8_LOCALES
		(ENCODING == __ctype_encoding_utf8)
#else
		0						/* Encoding is stateless. */
#endif
		: ((ssize_t) wcrtomb(s, swc, NULL));
}

#endif
/**********************************************************************/
#ifdef L_mbstowcs

size_t mbstowcs(wchar_t * __restrict pwcs, const char * __restrict s, size_t n)
{
	mbstate_t state;
	const char *e = s;			/* Needed because of restrict. */

	state.__mask = 0;			/* Always start in initial shift state. */
	return mbsrtowcs(pwcs, &e, n, &state);
}

#endif
/**********************************************************************/
#ifdef L_wcstombs

/* Note: We completely ignore state in all currently supported conversions. */

size_t wcstombs(char * __restrict s, const wchar_t * __restrict pwcs, size_t n)
{
	const wchar_t *e = pwcs;	/* Needed because of restrict. */

	return wcsrtombs(s, &e, n, NULL);
}

#endif
/**********************************************************************/
#if defined(L_wcstol) || defined(L_wcstol_l)

#if (ULONG_MAX == UINTMAX_MAX) && !defined(L_wcstol_l)
strong_alias(wcstol,wcstoimax)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(__XL(wcstol),__XL(wcstoll))
#endif

long __XL(wcstol)(const wchar_t * __restrict str,
				  wchar_t ** __restrict endptr, int base   __LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_wcsto_l)(str, endptr, base, 1   __LOCALE_ARG );
}

__XL_ALIAS(wcstol)

#endif
/**********************************************************************/
#if defined(L_wcstoll) || defined(L_wcstoll_l)

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if !defined(L_wcstoll_l)
#if (ULLONG_MAX == UINTMAX_MAX)
strong_alias(wcstoll,wcstoimax)
#endif
strong_alias(wcstoll,wcstoq)
#endif

long long __XL(wcstoll)(const wchar_t * __restrict str,
						wchar_t ** __restrict endptr, int base
						__LOCALE_PARAM )
{
    return (long long) __XL_NPP(_stdlib_wcsto_ll)(str, endptr, base, 1
												  __LOCALE_ARG );
}

__XL_ALIAS(wcstoll)

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/
#if defined(L_wcstoul) || defined(L_wcstoul_l)

#if (ULONG_MAX == UINTMAX_MAX) && !defined(L_wcstoul_l)
strong_alias(wcstoul,wcstoumax)
#endif

#if defined(ULLONG_MAX) && (ULLONG_MAX == ULONG_MAX)
strong_alias(__XL(wcstoul),__XL(wcstoull))
#endif

unsigned long __XL(wcstoul)(const wchar_t * __restrict str,
							wchar_t ** __restrict endptr, int base
							__LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_wcsto_l)(str, endptr, base, 0   __LOCALE_ARG );
}

__XL_ALIAS(wcstoul)

#endif
/**********************************************************************/
#if defined(L_wcstoull) || defined(L_wcstoull_l)

#if defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX)

#if !defined(L_wcstoull_l)
#if (ULLONG_MAX == UINTMAX_MAX)
strong_alias(wcstoull,wcstoumax)
#endif
strong_alias(wcstoull,wcstouq)
#endif

unsigned long long __XL(wcstoull)(const wchar_t * __restrict str,
								  wchar_t ** __restrict endptr, int base
								  __LOCALE_PARAM )
{
    return __XL_NPP(_stdlib_wcsto_ll)(str, endptr, base, 0   __LOCALE_ARG );
}

__XL_ALIAS(wcstoull)

#endif /* defined(ULLONG_MAX) && (LLONG_MAX > LONG_MAX) */

#endif
/**********************************************************************/

