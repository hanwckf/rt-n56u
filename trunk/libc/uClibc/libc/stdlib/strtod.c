/*  Copyright (C) 2000, 2003     Manuel Novoa III
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


/* Notes:
 *
 * The primary objective of this implementation was minimal size and
 * portablility, while providing robustness and resonable accuracy.
 *
 * This implementation depends on IEEE floating point behavior and expects
 * to be able to generate +/- infinity as a result.
 *
 * There are a number of compile-time options below.
 */

/* July 27, 2003
 *
 * General cleanup and some minor size optimizations.
 * Change implementation to support __strtofpmax() rather than strtod().
 *   Now all the strto{floating pt}() funcs are implemented in terms of
 *   of the internal __strtofpmax() function.
 * Support "nan", "inf", and "infinity" strings (case-insensitive).
 * Support hexadecimal floating point notation.
 * Support wchar variants.
 * Support xlocale variants.
 *
 * TODO:
 *
 * Consider accumulating blocks of digits in longs to save floating pt mults.
 *   This would likely be much better on anything that only supported floats
 *   where DECIMAL_DIG == 9.  Actually, if floats have FLT_MAX_10_EXP == 38,
 *   we could calculate almost all the exponent multipliers (p_base) in
 *   long arithmetic as well.
 */

/**********************************************************************/
/*							OPTIONS									  */
/**********************************************************************/

/* Defined if we want to recognize "nan", "inf", and "infinity". (C99) */
#define _STRTOD_NAN_INF_STRINGS  1

/* Defined if we want support hexadecimal floating point notation. (C99) */
/* Note!  Now controlled by uClibc configuration.  See below. */
#define _STRTOD_HEXADECIMAL_FLOATS 1

/* Defined if we want to scale with a O(log2(exp)) multiplications.
 * This is generally a good thing to do unless you are really tight
 * on space and do not expect to convert values of large magnitude. */

#define _STRTOD_LOG_SCALING	  1

/* WARNING!!!   WARNING!!!   WARNING!!!   WARNING!!!   WARNING!!!
 *
 * Clearing any of the options below this point is not advised (or tested).
 *
 * WARNING!!!   WARNING!!!   WARNING!!!   WARNING!!!   WARNING!!! */

/* Defined if we want strtod to set errno appropriately. */
/* NOTE: Implies all options below. */
#define _STRTOD_ERRNO			1

/* Defined if we want support for the endptr arg. */
/* Implied by _STRTOD_ERRNO. */
#define _STRTOD_ENDPTR		   1

/* Defined if we want to prevent overflow in accumulating the exponent. */
/* Implied by _STRTOD_ERRNO. */
#define _STRTOD_RESTRICT_EXP	 1

/* Defined if we want to process mantissa digits more intelligently. */
/* Implied by _STRTOD_ERRNO. */
#define _STRTOD_RESTRICT_DIGITS  1

/* Defined if we want to skip scaling 0 for the exponent. */
/* Implied by _STRTOD_ERRNO. */
#define _STRTOD_ZERO_CHECK	   1

/**********************************************************************/
/* Don't change anything that follows.									   */
/**********************************************************************/

#ifdef _STRTOD_ERRNO
#undef _STRTOD_ENDPTR
#undef _STRTOD_RESTRICT_EXP
#undef _STRTOD_RESTRICT_DIGITS
#undef _STRTOD_ZERO_CHECK
#define _STRTOD_ENDPTR		   1
#define _STRTOD_RESTRICT_EXP	 1
#define _STRTOD_RESTRICT_DIGITS  1
#define _STRTOD_ZERO_CHECK	   1
#endif

/**********************************************************************/

#define _ISOC99_SOURCE 1
#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <float.h>
#include <bits/uClibc_fpmax.h>

#include <locale.h>

#ifdef __UCLIBC_HAS_WCHAR__

#include <wchar.h>
#include <wctype.h>
#include <bits/uClibc_uwchar.h>

#endif

#ifdef __UCLIBC_HAS_XLOCALE__
#include <xlocale.h>
#endif /* __UCLIBC_HAS_XLOCALE__ */



/* Handle _STRTOD_HEXADECIMAL_FLOATS via uClibc config now. */
#undef _STRTOD_HEXADECIMAL_FLOATS
#ifdef __UCLIBC_HAS_HEXADECIMAL_FLOATS__
#define _STRTOD_HEXADECIMAL_FLOATS 1
#endif /* __UCLIBC_HAS_HEXADECIMAL_FLOATS__ */

/**********************************************************************/

#undef _STRTOD_FPMAX

#if FPMAX_TYPE == 3

#define NEED_STRTOLD_WRAPPER
#define NEED_STRTOD_WRAPPER
#define NEED_STRTOF_WRAPPER

#elif FPMAX_TYPE == 2

#define NEED_STRTOD_WRAPPER
#define NEED_STRTOF_WRAPPER

#elif FPMAX_TYPE == 1

#define NEED_STRTOF_WRAPPER

#else

#error unknown FPMAX_TYPE!

#endif

extern void __fp_range_check(__fpmax_t y, __fpmax_t x);

/**********************************************************************/

#ifdef _STRTOD_RESTRICT_DIGITS
#define EXP_DENORM_ADJUST DECIMAL_DIG
#define MAX_ALLOWED_EXP (DECIMAL_DIG  + EXP_DENORM_ADJUST - FPMAX_MIN_10_EXP)

#if MAX_ALLOWED_EXP > INT_MAX
#error size assumption violated for MAX_ALLOWED_EXP
#endif
#else
/* We want some excess if we're not restricting mantissa digits. */
#define MAX_ALLOWED_EXP ((20 - FPMAX_MIN_10_EXP) * 2)
#endif


#if defined(_STRTOD_RESTRICT_DIGITS) || defined(_STRTOD_ENDPTR) || defined(_STRTOD_HEXADECIMAL_FLOATS)
#undef _STRTOD_NEED_NUM_DIGITS
#define _STRTOD_NEED_NUM_DIGITS 1
#endif

/**********************************************************************/
#if defined(L___strtofpmax) || defined(L___strtofpmax_l) || defined(L___wcstofpmax) || defined(L___wcstofpmax_l)

#if defined(L___wcstofpmax) || defined(L___wcstofpmax_l)

#define __strtofpmax    __wcstofpmax
#define __strtofpmax_l  __wcstofpmax_l

#define Wchar wchar_t
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) iswspace_l((C), locale_arg)
#else
#define ISSPACE(C) iswspace((C))
#endif

#else  /* defined(L___wcstofpmax) || defined(L___wcstofpmax) */

#define Wchar char
#ifdef __UCLIBC_DO_XLOCALE
#define ISSPACE(C) isspace_l((C), locale_arg)
#else
#define ISSPACE(C) isspace((C))
#endif

#endif /* defined(L___wcstofpmax) || defined(L___wcstofpmax) */


#if defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE)

__fpmax_t __strtofpmax(const Wchar *str, Wchar **endptr, int exponent_power)
{
	return __strtofpmax_l(str, endptr, exponent_power, __UCLIBC_CURLOCALE);
}

#else  /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

__fpmax_t __XL_NPP(__strtofpmax)(const Wchar *str, Wchar **endptr, int exponent_power
								 __LOCALE_PARAM )
{
	__fpmax_t number;
	__fpmax_t p_base = 10;			/* Adjusted to 16 in the hex case. */
	Wchar *pos0;
#ifdef _STRTOD_ENDPTR
	Wchar *pos1;
#endif
	Wchar *pos = (Wchar *) str;
	int exponent_temp;
	int negative; /* A flag for the number, a multiplier for the exponent. */
#ifdef _STRTOD_NEED_NUM_DIGITS
	int num_digits;
#endif
#ifdef __UCLIBC_HAS_LOCALE__
	const char *decpt = __LOCALE_PTR->decimal_point;
#if defined(L___wcstofpmax) || defined(L___wcstofpmax)
	wchar_t decpt_wc = __LOCALE_PTR->decimal_point;
#else
	int decpt_len = __LOCALE_PTR->decimal_point_len;
#endif
#endif

#ifdef _STRTOD_HEXADECIMAL_FLOATS
	Wchar expchar = 'e';
	Wchar *poshex = NULL;
	__uint16_t is_mask = _ISdigit;
#define EXPCHAR		expchar
#define IS_X_DIGIT(C) __isctype((C), is_mask)
#else  /* _STRTOD_HEXADECIMAL_FLOATS */
#define EXPCHAR		'e'
#define IS_X_DIGIT(C) isdigit((C))
#endif /* _STRTOD_HEXADECIMAL_FLOATS */

	while (ISSPACE(*pos)) {		/* Skip leading whitespace. */
		++pos;
	}

	negative = 0;
	switch(*pos) {				/* Handle optional sign. */
		case '-': negative = 1;	/* Fall through to increment position. */
		case '+': ++pos;
	}

#ifdef _STRTOD_HEXADECIMAL_FLOATS
	if ((*pos == '0') && (((pos[1])|0x20) == 'x')) {
		poshex = ++pos;			/* Save position of 'x' in case no digits */
		++pos;					/*   and advance past it.  */
		is_mask = _ISxdigit;	/* Used by IS_X_DIGIT. */
		expchar = 'p';			/* Adjust exponent char. */
		p_base = 16;			/* Adjust base multiplier. */
	}
#endif

	number = 0.;
#ifdef _STRTOD_NEED_NUM_DIGITS
	num_digits = -1;
#endif
/* 	exponent_power = 0; */
	pos0 = NULL;

 LOOP:
	while (IS_X_DIGIT(*pos)) {	/* Process string of (hex) digits. */
#ifdef _STRTOD_RESTRICT_DIGITS
		if (num_digits < 0) {	/* First time through? */
			++num_digits;		/* We've now seen a digit. */
		}
		if (num_digits || (*pos != '0')) { /* Had/have nonzero. */
			++num_digits;
			if (num_digits <= DECIMAL_DIG) { /* Is digit significant? */
#ifdef _STRTOD_HEXADECIMAL_FLOATS
				number = number * p_base
					+ (isdigit(*pos)
					   ? (*pos - '0')
					   : (((*pos)|0x20) - ('a' - 10)));
#else  /* _STRTOD_HEXADECIMAL_FLOATS */
				number = number * p_base + (*pos - '0');
#endif /* _STRTOD_HEXADECIMAL_FLOATS */
			}
		}
#else  /* _STRTOD_RESTRICT_DIGITS */
#ifdef _STRTOD_NEED_NUM_DIGITS
		++num_digits;
#endif
#ifdef _STRTOD_HEXADECIMAL_FLOATS
		number = number * p_base
			+ (isdigit(*pos)
			   ? (*pos - '0')
			   : (((*pos)|0x20) - ('a' - 10)));
#else  /* _STRTOD_HEXADECIMAL_FLOATS */
		number = number * p_base + (*pos - '0');
#endif /* _STRTOD_HEXADECIMAL_FLOATS */
#endif /* _STRTOD_RESTRICT_DIGITS */
		++pos;
	}

#ifdef __UCLIBC_HAS_LOCALE__
#if defined(L___wcstofpmax) || defined(L___wcstofpmax)
	if (!pos0 && (*pos == decpt_wc)) { /* First decimal point? */
		pos0 = ++pos;
		goto LOOP;
	}
#else
	if (!pos0 && !memcmp(pos, decpt, decpt_len)) { /* First decimal point? */
		pos0 = (pos += decpt_len);
		goto LOOP;
	}
#endif
#else  /* __UCLIBC_HAS_LOCALE__ */
	if ((*pos == '.') && !pos0) { /* First decimal point? */
		pos0 = ++pos;			/* Save position of decimal point */
		goto LOOP;				/*   and process rest of digits. */
	}
#endif /* __UCLIBC_HAS_LOCALE__ */

#ifdef _STRTOD_NEED_NUM_DIGITS
	if (num_digits<0) {			/* Must have at least one digit. */
#ifdef _STRTOD_HEXADECIMAL_FLOATS
		if (poshex) {			/* Back up to '0' in '0x' prefix. */
			pos = poshex;
			goto DONE;
		}
#endif /* _STRTOD_HEXADECIMAL_FLOATS */

#ifdef _STRTOD_NAN_INF_STRINGS
		if (!pos0) {			/* No decimal point, so check for inf/nan. */
			/* Note: nan is the first string so 'number = i/0.;' works. */
			static const char nan_inf_str[] = "\05nan\0\012infinity\0\05inf\0";
			int i = 0;

#ifdef __UCLIBC_HAS_LOCALE__
			/* Avoid tolower problems for INFINITY in the tr_TR locale. (yuk)*/
#undef _tolower
#define _tolower(C)     ((C)|0x20)
#endif /* __UCLIBC_HAS_LOCALE__ */

			do {
				/* Unfortunately, we have no memcasecmp(). */
				int j = 0;
				while (_tolower(pos[j]) == nan_inf_str[i+1+j]) {
					++j;
					if (!nan_inf_str[i+1+j]) {
						number = i / 0.;
						if (negative) {	/* Correct for sign. */
							number = -number;
						}
						pos += nan_inf_str[i] - 2;
						goto DONE;
					}
				}
				i += nan_inf_str[i];
			} while (nan_inf_str[i]);
		}

#endif /* STRTOD_NAN_INF_STRINGS */
#ifdef _STRTOD_ENDPTR
		pos = (Wchar *) str;
#endif
		goto DONE;
	}
#endif /* _STRTOD_NEED_NUM_DIGITS */

#ifdef _STRTOD_RESTRICT_DIGITS
	if (num_digits > DECIMAL_DIG) { /* Adjust exponent for skipped digits. */
		exponent_power += num_digits - DECIMAL_DIG;
	}
#endif

	if (pos0) {
		exponent_power += pos0 - pos; /* Adjust exponent for decimal point. */
	}

#ifdef _STRTOD_HEXADECIMAL_FLOATS
	if (poshex) {
		exponent_power *= 4;	/* Above is 2**4, but below is 2. */
		p_base = 2;
	}
#endif /* _STRTOD_HEXADECIMAL_FLOATS */

	if (negative) {				/* Correct for sign. */
		number = -number;
	}

	/* process an exponent string */
	if (((*pos)|0x20) == EXPCHAR) {
#ifdef _STRTOD_ENDPTR
		pos1 = pos;
#endif
		negative = 1;
		switch(*++pos) {		/* Handle optional sign. */
			case '-': negative = -1; /* Fall through to increment pos. */
			case '+': ++pos;
		}

		pos0 = pos;
		exponent_temp = 0;
		while (isdigit(*pos)) {	/* Process string of digits. */
#ifdef _STRTOD_RESTRICT_EXP
			if (exponent_temp < MAX_ALLOWED_EXP) { /* Avoid overflow. */
				exponent_temp = exponent_temp * 10 + (*pos - '0');
			}
#else
			exponent_temp = exponent_temp * 10 + (*pos - '0');
#endif
			++pos;
		}

#ifdef _STRTOD_ENDPTR
		if (pos == pos0) {	/* No digits? */
			pos = pos1;		/* Back up to {e|E}/{p|P}. */
		} /* else */
#endif

		exponent_power += negative * exponent_temp;
	}

#ifdef _STRTOD_ZERO_CHECK
	if (number == 0.) {
		goto DONE;
	}
#endif

	/* scale the result */
#ifdef _STRTOD_LOG_SCALING
	exponent_temp = exponent_power;

	if (exponent_temp < 0) {
		exponent_temp = -exponent_temp;
	}

	while (exponent_temp) {
		if (exponent_temp & 1) {
			if (exponent_power < 0) {
				/* Warning... caluclating a factor for the exponent and
				 * then dividing could easily be faster.  But doing so
				 * might cause problems when dealing with denormals. */
				number /= p_base;
			} else {
				number *= p_base;
			}
		}
		exponent_temp >>= 1;
		p_base *= p_base;
	}

#else  /* _STRTOD_LOG_SCALING */
	while (exponent_power) {
		if (exponent_power < 0) {
			number /= p_base;
			exponent_power++;
		} else {
			number *= p_base;
			exponent_power--;
		}
	}
#endif /* _STRTOD_LOG_SCALING */

#ifdef _STRTOD_ERRNO
	if (__FPMAX_ZERO_OR_INF_CHECK(number)) {
		__set_errno(ERANGE);
	}
#endif

 DONE:
#ifdef _STRTOD_ENDPTR
	if (endptr) {
		*endptr = pos;
	}
#endif

	return number;
}

#endif /* defined(__UCLIBC_HAS_XLOCALE__) && !defined(__UCLIBC_DO_XLOCALE) */

#endif
/**********************************************************************/
#ifdef L___fp_range_check
#if defined(NEED_STRTOF_WRAPPER) || defined(NEED_STRTOD_WRAPPER)

extern void __fp_range_check(__fpmax_t y, __fpmax_t x)
{
	if (__FPMAX_ZERO_OR_INF_CHECK(y) /* y is 0 or +/- infinity */
		&& (y != 0)	/* y is not 0 (could have x>0, y==0 if underflow) */
		&& !__FPMAX_ZERO_OR_INF_CHECK(x) /* x is not 0 or +/- infinity */
		) {
		__set_errno(ERANGE);	/* Then x is not in y's range. */
	}
}

#endif
#endif
/**********************************************************************/
#if defined(L_strtof) || defined(L_strtof_l) || defined(L_wcstof) || defined(L_wcstof_l)
#if defined(NEED_STRTOF_WRAPPER)

#if defined(L_wcstof) || defined(L_wcstof_l)
#define strtof           wcstof
#define strtof_l         wcstof_l
#define __strtof         __wcstof
#define __strtof_l       __wcstof_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif


float __XL(strtof)(const Wchar *str, Wchar **endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 1
	return __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
#else
	__fpmax_t x;
	float y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
	y = (float) x;

	__fp_range_check(y, x);

	return y;
#endif
}

__XL_ALIAS(strtof)

#endif
#endif
/**********************************************************************/
#if defined(L_strtod) || defined(L_strtod_l) || defined(L_wcstod) || defined(L_wcstod_l)
#if defined(NEED_STRTOD_WRAPPER)

#if defined(L_wcstod) || defined(L_wcstod_l)
#define strtod           wcstod
#define strtod_l         wcstod_l
#define __strtod         __wcstod
#define __strtod_l       __wcstod_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif

double __XL(strtod)(const Wchar *__restrict str,
					Wchar **__restrict endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 2
	return __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
#else
	__fpmax_t x;
	double y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
	y = (double) x;

	__fp_range_check(y, x);

	return y;
#endif
}

__XL_ALIAS(strtod)

#endif
#endif
/**********************************************************************/
#if defined(L_strtold) || defined(L_strtold_l) || defined(L_wcstold) || defined(L_wcstold_l)
#if defined(NEED_STRTOLD_WRAPPER)

#if defined(L_wcstold) || defined(L_wcstold_l)
#define strtold           wcstold
#define strtold_l         wcstold_l
#define __strtold         __wcstold
#define __strtold_l       __wcstold_l
#define __strtofpmax     __wcstofpmax
#define __strtofpmax_l   __wcstofpmax_l
#define Wchar wchar_t
#else
#define Wchar char
#endif

long double __XL(strtold)(const Wchar *str, Wchar **endptr   __LOCALE_PARAM )
{
#if FPMAX_TYPE == 3
	return __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
#else
	__fpmax_t x;
	long double y;

	x = __XL_NPP(__strtofpmax)(str, endptr, 0   __LOCALE_ARG );
	y = (long double) x;

	__fp_range_check(y, x);

	return y;
#endif
}

__XL_ALIAS(strtold)

#endif
#endif
/**********************************************************************/
