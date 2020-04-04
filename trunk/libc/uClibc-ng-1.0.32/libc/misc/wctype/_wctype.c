/*  Copyright (C) 2002, 2003     Manuel Novoa III
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

#define __NO_CTYPE

#include <wctype.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <locale.h>
#include <ctype.h>
#include <stdint.h>
#include <bits/uClibc_uwchar.h>

#if defined(__LOCALE_C_ONLY) && defined(__UCLIBC_DO_XLOCALE)
# error xlocale functionality is not supported in stub locale mode.
#endif

/* We know wide char support is enabled.  We wouldn't be here otherwise. */

/* Define this if you want to unify the towupper and towlower code in the
 * towctrans function. */
/* #define SMALL_UPLOW */


/* Pull in __CTYPE_xxx constants */
#include <bits/uClibc_charclass.h>


/* The following is used to implement wctype(), but it is defined
 * here because the ordering must agree with that of the enumeration
 * below (ignoring unclassified). */
#define __CTYPE_TYPESTRING \
	"\6alnum\0\6alpha\0\6blank\0\6cntrl\0\6digit\0\6graph\0\6lower\0" \
	"\6print\0\6punct\0\6space\0\6upper\0\7xdigit\0\0"


/* The values for wctype_t. */
enum {
	_CTYPE_unclassified = 0,
	_CTYPE_isalnum,
	_CTYPE_isalpha,
	_CTYPE_isblank,
	_CTYPE_iscntrl,
	_CTYPE_isdigit,
	_CTYPE_isgraph,
	_CTYPE_islower,
	_CTYPE_isprint,
	_CTYPE_ispunct,
	_CTYPE_isspace,
	_CTYPE_isupper,
	_CTYPE_isxdigit				/* _MUST_ be last of the standard classes! */
};

/* The following is used to implement wctrans(). */

#define __CTYPE_TRANSTRING	"\10tolower\0\10toupper\0\10totitle\0\0"

enum {
	_CTYPE_tolower = 1,
	_CTYPE_toupper,
	_CTYPE_totitle
};

/*--------------------------------------------------------------------*/

#define _CTYPE_iswxdigit (_CTYPE_isxdigit)

/*--------------------------------------------------------------------*/

#define ENCODING		(__UCLIBC_CURLOCALE->encoding)

#define WCctype			(__UCLIBC_CURLOCALE->tblwctype)
#define WCuplow			(__UCLIBC_CURLOCALE->tblwuplow)
#define WCcmob			(__UCLIBC_CURLOCALE->tblwcomb)
#define WCuplow_diff		(__UCLIBC_CURLOCALE->tblwuplow_diff)


#define WC_TABLE_DOMAIN_MAX   __LOCALE_DATA_WC_TABLE_DOMAIN_MAX

#define WCctype_II_LEN        __LOCALE_DATA_WCctype_II_LEN
#define WCctype_TI_LEN        __LOCALE_DATA_WCctype_TI_LEN
#define WCctype_UT_LEN        __LOCALE_DATA_WCctype_UT_LEN
#define WCctype_II_SHIFT      __LOCALE_DATA_WCctype_II_SHIFT
#define WCctype_TI_SHIFT      __LOCALE_DATA_WCctype_TI_SHIFT

#define WCuplow_II_LEN        __LOCALE_DATA_WCuplow_II_LEN
#define WCuplow_TI_LEN        __LOCALE_DATA_WCuplow_TI_LEN
#define WCuplow_UT_LEN        __LOCALE_DATA_WCuplow_UT_LEN
#define WCuplow_II_SHIFT      __LOCALE_DATA_WCuplow_II_SHIFT
#define WCuplow_TI_SHIFT      __LOCALE_DATA_WCuplow_TI_SHIFT


#define WCctype_TI_MASK		((1 << (WCctype_TI_SHIFT)) - 1)
#define WCctype_II_MASK		((1 << (WCctype_II_SHIFT)) - 1)

/**********************************************************************/

#undef __PASTE2
#undef __PASTE3
#define __PASTE2(X,Y)		X ## Y
#define __PASTE3(X,Y,Z)		X ## Y ## Z

#ifdef __UCLIBC_DO_XLOCALE

#define ISW_FUNC_BODY(NAME) \
int __PASTE3(isw,NAME,_l) (wint_t wc, __locale_t l) \
{ \
	return iswctype_l(wc, __PASTE2(_CTYPE_is,NAME), l); \
}

#else  /* __UCLIBC_DO_XLOCALE */

#define ISW_FUNC_BODY(NAME) \
int __PASTE2(isw,NAME) (wint_t wc) \
{ \
	return iswctype(wc, __PASTE2(_CTYPE_is,NAME)); \
}

#endif /* __UCLIBC_DO_XLOCALE */
/**********************************************************************/
#if defined(L_iswalnum) || defined(L_iswalnum_l)

ISW_FUNC_BODY(alnum);
# ifdef L_iswalnum
libc_hidden_def(iswalnum)
# endif

#endif
/**********************************************************************/
#if defined(L_iswalpha) || defined(L_iswalpha_l)

ISW_FUNC_BODY(alpha);

#endif
/**********************************************************************/
#if defined(L_iswblank) || defined(L_iswblank_l)

ISW_FUNC_BODY(blank);

#endif
/**********************************************************************/
#if defined(L_iswcntrl) || defined(L_iswcntrl_l)

ISW_FUNC_BODY(cntrl);

#endif
/**********************************************************************/
#if defined(L_iswdigit) || defined(L_iswdigit_l)

ISW_FUNC_BODY(digit);

#endif
/**********************************************************************/
#if defined(L_iswgraph) || defined(L_iswgraph_l)

ISW_FUNC_BODY(graph);

#endif
/**********************************************************************/
#if defined(L_iswlower) || defined(L_iswlower_l)

ISW_FUNC_BODY(lower);
# ifdef L_iswlower
libc_hidden_def(iswlower)
# endif

#endif
/**********************************************************************/
#if defined(L_iswprint) || defined(L_iswprint_l)

ISW_FUNC_BODY(print);

#endif
/**********************************************************************/
#if defined(L_iswpunct) || defined(L_iswpunct_l)

ISW_FUNC_BODY(punct);

#endif
/**********************************************************************/
#if defined(L_iswspace) || defined(L_iswspace_l)

ISW_FUNC_BODY(space);
# ifdef L_iswspace
libc_hidden_def(iswspace)
# else
libc_hidden_def(iswspace_l)
# endif

#endif
/**********************************************************************/
#if defined(L_iswupper) || defined(L_iswupper_l)

ISW_FUNC_BODY(upper);
# ifdef L_iswupper
libc_hidden_def(iswupper)
# endif

#endif
/**********************************************************************/
#if defined(L_iswxdigit) || defined(L_iswxdigit_l)

ISW_FUNC_BODY(xdigit);

#endif
/**********************************************************************/
#if defined(L_towlower) || defined(L_towlower_l)

# ifdef L_towlower
#  define TOWLOWER(w) towlower(w)
# else
#  define TOWLOWER(w) towlower_l(w, __locale_t locale)
#  undef __UCLIBC_CURLOCALE
#  define __UCLIBC_CURLOCALE (locale)
# endif

# ifdef __UCLIBC_HAS_XLOCALE__
#  define TOWCTRANS(w,d) towctrans_l(w,d, __UCLIBC_CURLOCALE)
# else
#  define TOWCTRANS(w,d) towctrans(w,d)
# endif

# define __C_towlower(wc) \
	(((__uwchar_t)(wc) <= 0x7f) ? (__C_ctype_tolower)[(wc)] : (wc))

# ifdef __LOCALE_C_ONLY

wint_t towlower(wint_t wc)
{
#  ifdef __UCLIBC_HAS_CTYPE_TABLES__
	return __C_towlower(wc);
#  else
	return (wc == (unsigned)wc)
		? __C_tolower((unsigned)wc)
		: 0;
#  endif
}

# else  /* __LOCALE_C_ONLY */

#  ifdef SMALL_UPLOW

#   if defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__)
wint_t towlower(wint_t wc)
{
	return towctrans_l(wc, _CTYPE_tolower, __UCLIBC_CURLOCALE);
}
#   else  /* defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__) */
wint_t TOWLOWER(wint_t wc)
{
	return TOWCTRANS(wc, _CTYPE_tolower);
}
#   endif /* defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__) */

#  else  /* SMALL_UPLOW */

#   if defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__)

wint_t towlower(wint_t wc)
{
	return towlower_l(wc, __UCLIBC_CURLOCALE);
}

#   else  /* defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__) */

wint_t TOWLOWER(wint_t wc)
{
	unsigned sc, n, i;
	__uwchar_t u = wc;

	if (ENCODING == __ctype_encoding_7_bit) {
		/* We're in the C/POSIX locale, so ignore the tables. */
		return __C_towlower(wc);
	}

	if (u <= WC_TABLE_DOMAIN_MAX) {
		sc = u & ((1 << WCuplow_TI_SHIFT) - 1);
		u >>= WCuplow_TI_SHIFT;
		n = u & ((1 << WCuplow_II_SHIFT) - 1);
		u >>= WCuplow_II_SHIFT;

		i = ((unsigned) WCuplow[u]) << WCuplow_II_SHIFT;
		i = ((unsigned) WCuplow[WCuplow_II_LEN + i + n]) << WCuplow_TI_SHIFT;
		i = ((unsigned) WCuplow[WCuplow_II_LEN + WCuplow_TI_LEN	+ i + sc]) << 1;
		wc += WCuplow_diff[i + 1];
	}
	return wc;
}

#   endif /* defined(L_towlower) && defined(__UCLIBC_HAS_XLOCALE__) */

#  endif /* SMALL_UPLOW */

#  ifdef L_towlower_l
libc_hidden_def(towlower_l)
#  endif

# endif /* __LOCALE_C_ONLY */

# ifndef L_towlower_l
libc_hidden_def(towlower)
# endif

#endif
/**********************************************************************/
#if defined(L_towupper) || defined(L_towupper_l)

# ifdef L_towupper
#  define TOWUPPER(w) towupper(w)
# else
#  define TOWUPPER(w) towupper_l(w, __locale_t locale)
#  undef __UCLIBC_CURLOCALE
#  define __UCLIBC_CURLOCALE (locale)
# endif

# ifdef __UCLIBC_HAS_XLOCALE__
#  define TOWCTRANS(w,d) towctrans_l(w,d, __UCLIBC_CURLOCALE)
# else
#  define TOWCTRANS(w,d) towctrans(w,d)
# endif

# define __C_towupper(wc) \
	(((__uwchar_t)(wc) <= 0x7f) ? (__C_ctype_toupper)[(wc)] : (wc))

# ifdef __LOCALE_C_ONLY

wint_t towupper(wint_t wc)
{
#  ifdef __UCLIBC_HAS_CTYPE_TABLES__
	return __C_towupper(wc);
#  else
	return (wc == (unsigned)wc)
		? __C_toupper((unsigned)wc)
		: 0;
#  endif
}

# else  /* __LOCALE_C_ONLY */

#  ifdef SMALL_UPLOW

#   if defined(L_towupper) && defined(__UCLIBC_HAS_XLOCALE__)
wint_t towupper(wint_t wc)
{
	return towctrans_l(wc, _CTYPE_toupper, __UCLIBC_CURLOCALE);
}
#   else
wint_t TOWUPPER(wint_t wc)
{
	return TOWCTRANS(wc, _CTYPE_toupper);
}
#   endif

#  else  /* SMALL_UPLOW */

#   if defined(L_towupper) && defined(__UCLIBC_HAS_XLOCALE__)
wint_t towupper(wint_t wc)
{
	return towupper_l(wc, __UCLIBC_CURLOCALE);
}
#   else  /* defined(L_towupper) && defined(__UCLIBC_HAS_XLOCALE__) */
wint_t TOWUPPER(wint_t wc)
{
	unsigned sc, n, i;
	__uwchar_t u = wc;

	if (ENCODING == __ctype_encoding_7_bit) {
		/* We're in the C/POSIX locale, so ignore the tables. */
		return __C_towupper(wc);
	}

	if (u <= WC_TABLE_DOMAIN_MAX) {
		sc = u & ((1 << WCuplow_TI_SHIFT) - 1);
		u >>= WCuplow_TI_SHIFT;
		n = u & ((1 << WCuplow_II_SHIFT) - 1);
		u >>= WCuplow_II_SHIFT;

		i = ((unsigned) WCuplow[u]) << WCuplow_II_SHIFT;
		i = ((unsigned) WCuplow[WCuplow_II_LEN + i + n]) << WCuplow_TI_SHIFT;
		i = ((unsigned) WCuplow[WCuplow_II_LEN + WCuplow_TI_LEN + i + sc]) << 1;
		wc += WCuplow_diff[i];
	}
	return wc;
}
#   endif /* defined(L_towupper) && defined(__UCLIBC_HAS_XLOCALE__) */

#  endif /* SMALL_UPLOW */

#  ifdef L_towupper_l
libc_hidden_def(towupper_l)
#  endif

# endif /* __LOCALE_C_ONLY */

# ifndef L_towupper_l
libc_hidden_def(towupper)
# endif

#endif
/**********************************************************************/
#ifdef L_wctype

static const unsigned char typestring[] = __CTYPE_TYPESTRING;

wctype_t wctype(const char *property)
{
	const unsigned char *p;
	int i;

	p = typestring;
	i = 1;
	do {
		if (!strcmp(property, (const char *) ++p)) {
			return i;
		}
		++i;
		p += p[-1];
	} while (*p);

	/* TODO - Add locale-specific classifications. */
	return 0;
}
libc_hidden_def(wctype)

#endif
/**********************************************************************/
#ifdef L_wctype_l

wctype_t wctype_l (const char *property, __locale_t locale)
{
	return wctype(property);
}

#endif
/**********************************************************************/
#if defined(L_iswctype) || defined(L_iswctype_l)

#define __C_iswdigit(c) \
	((sizeof(c) == sizeof(char)) \
	 ? ((unsigned char)((c) - '0') < 10) \
	 : ((__uwchar_t)((c) - '0') < 10) \
	)
#define __C_iswxdigit(c) \
	(__C_iswdigit(c) \
	 || ((sizeof(c) == sizeof(char)) \
		 ? ((unsigned char)(((c) | 0x20) - 'a') < 6) \
		 : ((__uwchar_t)(((c) | 0x20) - 'a') < 6) \
	    ) \
	)

#ifdef __UCLIBC_HAS_CTYPE_TABLES__
# if !defined(__UCLIBC_HAS_XLOCALE__) || defined(L_iswctype_l)
static const unsigned short int desc2flag[] = {
	[_CTYPE_unclassified] = 0,
	[_CTYPE_isalnum] = (unsigned short int) _ISwalnum,
	[_CTYPE_isalpha] = (unsigned short int) _ISwalpha,
	[_CTYPE_isblank] = (unsigned short int) _ISwblank,
	[_CTYPE_iscntrl] = (unsigned short int) _ISwcntrl,
	[_CTYPE_isdigit] = (unsigned short int) _ISwdigit,
	[_CTYPE_isgraph] = (unsigned short int) _ISwgraph,
	[_CTYPE_islower] = (unsigned short int) _ISwlower,
	[_CTYPE_isprint] = (unsigned short int) _ISwprint,
	[_CTYPE_ispunct] = (unsigned short int) _ISwpunct,
	[_CTYPE_isspace] = (unsigned short int) _ISwspace,
	[_CTYPE_isupper] = (unsigned short int) _ISwupper,
	[_CTYPE_isxdigit] = (unsigned short int) _ISwxdigit,
};
# endif
#endif

#ifdef __LOCALE_C_ONLY

#ifdef __UCLIBC_HAS_CTYPE_TABLES__

int iswctype(wint_t wc, wctype_t desc)
{
	/* Note... wctype_t is unsigned. */

	if ((__uwchar_t) wc <= 0x7f
	 && desc < (sizeof(desc2flag) / sizeof(desc2flag[0]))
	) {
		return __isctype(wc, desc2flag[desc]);
	}
	return 0;
}

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

int iswctype(wint_t wc, wctype_t desc)
{
	/* This is lame, but it is here just to get it working for now. */

	if (wc == (unsigned)wc) {
		switch (desc) {
			case _CTYPE_isupper:
				return __C_isupper((unsigned)wc);
			case _CTYPE_islower:
				return __C_islower((unsigned)wc);
			case _CTYPE_isalpha:
				return __C_isalpha((unsigned)wc);
			case _CTYPE_isdigit:
				return __C_isdigit((unsigned)wc);
			case _CTYPE_isxdigit:
				return __C_isxdigit((unsigned)wc);
			case _CTYPE_isspace:
				return __C_isspace((unsigned)wc);
			case _CTYPE_isprint:
				return __C_isprint((unsigned)wc);
			case _CTYPE_isgraph:
				return __C_isgraph((unsigned)wc);
			case _CTYPE_isblank:
				return __C_isblank((unsigned)wc);
			case _CTYPE_iscntrl:
				return __C_iscntrl((unsigned)wc);
			case _CTYPE_ispunct:
				return __C_ispunct((unsigned)wc);
			case _CTYPE_isalnum:
				return __C_isalnum((unsigned)wc);
			default:
				break;
		}
	}
	return 0;
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */

#else  /* __LOCALE_C_ONLY */

#ifdef L_iswctype
# define ISWCTYPE(w,d) iswctype(w,d)
#else
# define ISWCTYPE(w,d) iswctype_l(w,d, __locale_t locale)
# undef __UCLIBC_CURLOCALE
# define __UCLIBC_CURLOCALE (locale)
#endif

#if defined(L_iswctype) && defined(__UCLIBC_HAS_XLOCALE__)

int iswctype(wint_t wc, wctype_t desc)
{
	return iswctype_l(wc, desc, __UCLIBC_CURLOCALE);
}

#else  /* defined(L_iswctype) && defined(__UCLIBC_HAS_XLOCALE__) */

int ISWCTYPE(wint_t wc, wctype_t desc)
{
	unsigned sc, n, i0, i1;
	unsigned char d = __CTYPE_unclassified;

	if ((ENCODING != __ctype_encoding_7_bit) || ((__uwchar_t)wc <= 0x7f)) {
		if (desc < _CTYPE_iswxdigit) {
			if ((__uwchar_t)wc <= WC_TABLE_DOMAIN_MAX) {
				/* From here on, we know wc > 0. */
				sc = wc & WCctype_TI_MASK;
				wc >>= WCctype_TI_SHIFT;
				n = wc & WCctype_II_MASK;
				wc >>= WCctype_II_SHIFT;

				i0 = WCctype[wc];
				i0 <<= WCctype_II_SHIFT;
				i1 = WCctype[WCctype_II_LEN + i0 + n];
				i1 <<= (WCctype_TI_SHIFT-1);
				d = WCctype[WCctype_II_LEN + WCctype_TI_LEN + i1 + (sc >> 1)];

				d = (sc & 1) ? (d >> 4) : (d & 0xf);
			} else if ((__uwchar_t)(wc - 0xe0020UL) <= 0x5f
				|| wc == 0xe0001UL
				|| (((__uwchar_t)(wc - 0xf0000UL) < 0x20000UL) && ((wc & 0xffffU) <= 0xfffdU))
			) {
				d = __CTYPE_punct;
			}

#if 0
			return ((unsigned char)(d - ctype_range[2*desc]) <= ctype_range[2*desc + 1])
				&& ((desc != _CTYPE_iswblank) || (d & 1));
#else
			return __UCLIBC_CURLOCALE->code2flag[d] & desc2flag[desc];
#endif
		}

		/* TODO - Add locale-specific classifications. */
		return (desc == _CTYPE_iswxdigit) ? __C_iswxdigit(wc) : 0;
	}
	return 0;
}

#endif /* defined(L_iswctype) && defined(__UCLIBC_HAS_XLOCALE__) */

#ifdef L_iswctype_l
libc_hidden_def(iswctype_l)
#endif

#endif /* __LOCALE_C_ONLY */

#ifdef L_iswctype
libc_hidden_def(iswctype)
#endif

#endif
/**********************************************************************/
#if defined(L_towctrans) || defined(L_towctrans_l)

#ifdef __LOCALE_C_ONLY

/* Minimal support for C/POSIX locale. */

wint_t towctrans(wint_t wc, wctrans_t desc)
{
	if ((unsigned)(desc - _CTYPE_tolower) <= (_CTYPE_toupper - _CTYPE_tolower)) {
		/* Transliteration is either tolower or toupper. */
#if 0
/* I think it's wrong: _toupper(c) assumes that c is a *lowercase* *letter* -
 * it is defined as ((c) ^ 0x20)! */
		if ((__uwchar_t) wc <= 0x7f) {
			return (desc == _CTYPE_tolower) ? _tolower(wc) : _toupper(wc);
		}
#endif
		__uwchar_t c = wc | 0x20; /* lowercase if it's a letter */
		if (c >= 'a' && c <= 'z') {
			if (desc == _CTYPE_toupper)
				c &= ~0x20; /* uppercase */
			return c;
		}
	} else {
		__set_errno(EINVAL);	/* Invalid transliteration. */
	}
	return wc;
}

#else  /* __LOCALE_C_ONLY */

#ifdef L_towctrans
# define TOWCTRANS(w,d) towctrans(w,d)
#else
# define TOWCTRANS(w,d) towctrans_l(w,d, __locale_t locale)
# undef __UCLIBC_CURLOCALE
# define __UCLIBC_CURLOCALE (locale)
#endif

#ifdef __UCLIBC_HAS_XLOCALE__
# define TOWLOWER(w,l) towlower_l(w,l)
# define TOWUPPER(w,l) towupper_l(w,l)
#else
# define TOWLOWER(w,l) towlower(w)
# define TOWUPPER(w,l) towupper(w)
#endif

#if defined(L_towctrans) && defined(__UCLIBC_HAS_XLOCALE__)

wint_t towctrans(wint_t wc, wctrans_t desc)
{
	return towctrans_l(wc, desc, __UCLIBC_CURLOCALE);
}

#else  /* defined(L_towctrans) && defined(__UCLIBC_HAS_XLOCALE__) */

#ifdef SMALL_UPLOW

wint_t TOWCTRANS(wint_t wc, wctrans_t desc)
{
	unsigned sc, n, i;
	__uwchar_t u = wc;

	/* TODO - clean up */
	if (ENCODING == __ctype_encoding_7_bit) {
		if ((__uwchar_t)wc > 0x7f
		 || (unsigned)(desc - _CTYPE_tolower) > (_CTYPE_toupper - _CTYPE_tolower)
		) {
			/* We're in the C/POSIX locale, so ignore non-ASCII values
			 * as well an any mappings other than toupper or tolower. */
			return wc;
		}
	}

	if ((unsigned)(desc - _CTYPE_tolower) <= (_CTYPE_totitle - _CTYPE_tolower)) {
		if (u <= WC_TABLE_DOMAIN_MAX) {
			sc = u & ((1 << WCuplow_TI_SHIFT) - 1);
			u >>= WCuplow_TI_SHIFT;
			n = u & ((1 << WCuplow_II_SHIFT) - 1);
			u >>= WCuplow_II_SHIFT;

			i = ((unsigned) WCuplow[u]) << WCuplow_II_SHIFT;
			i = ((unsigned) WCuplow[WCuplow_II_LEN + i + n]) << WCuplow_TI_SHIFT;
			i = ((unsigned) WCuplow[WCuplow_II_LEN + WCuplow_TI_LEN + i + sc]) << 1;
			if (desc == _CTYPE_tolower) {
				++i;
			}
			wc += WCuplow_diff[i];
			if (desc == _CTYPE_totitle) {
				/* WARNING! These special cases work for glibc 2.2.4.  Changes
				 * may be needed if the glibc locale tables are updated. */
				if ((__uwchar_t)(wc - 0x1c4) <= (0x1cc - 0x1c4)
				 || wc == 0x1f1
				) {
					++wc;
				}
			}
		}
	} else {
		/* TODO - Deal with other transliterations. */
		__set_errno(EINVAL);
	}

	return wc;
}

#else  /* SMALL_UPLOW */

wint_t TOWCTRANS(wint_t wc, wctrans_t desc)
{
	if (ENCODING == __ctype_encoding_7_bit) {
		if ((__uwchar_t)wc > 0x7f
		 || (unsigned)(desc - _CTYPE_tolower) > (_CTYPE_toupper - _CTYPE_tolower)
		) {
			/* We're in the C/POSIX locale, so ignore non-ASCII values
			 * as well an any mappings other than toupper or tolower. */
			return wc;
		}
	}

	if (desc == _CTYPE_tolower) {
		return TOWLOWER(wc, __UCLIBC_CURLOCALE);
	}
	if ((unsigned)(desc - _CTYPE_toupper) <= (_CTYPE_totitle - _CTYPE_toupper)) {
		wc = TOWUPPER(wc, __UCLIBC_CURLOCALE);
		if (desc == _CTYPE_totitle) {
			/* WARNING! These special cases work for glibc 2.2.4.  Changes
			 * may be needed if the glibc locale tables are updated. */
			if ((__uwchar_t)(wc - 0x1c4) <= (0x1cc - 0x1c4)
			 || wc == 0x1f1
			) {
				++wc;
			}
		}
	} else {
		/* TODO - Deal with other transliterations. */
		__set_errno(EINVAL);
	}
	return wc;
}

#endif /* SMALL_UPLOW */

#endif /* defined(L_towctrans) && defined(__UCLIBC_HAS_XLOCALE__) */

#ifdef L_towctrans_l
libc_hidden_def(towctrans_l)
#endif

#endif /* __LOCALE_C_ONLY */

#ifndef L_towctrans_l
libc_hidden_def(towctrans)
#endif

#endif
/**********************************************************************/
#ifdef L_wctrans

static const char transstring[] = __CTYPE_TRANSTRING;

wctrans_t wctrans(const char *property)
{
	const unsigned char *p;
	int i;

	p = (const unsigned char *) transstring;
	i = 1;
	do {
		if (!strcmp(property, (const char*) ++p)) {
			return i;
		}
		++i;
		p += p[-1];
	} while (*p);

	/* TODO - Add locale-specific translations. */
	return 0;
}
libc_hidden_def(wctrans)

#endif
/**********************************************************************/
#ifdef L_wctrans_l

wctrans_t wctrans_l(const char *property, __locale_t locale)
{
	return wctrans(property);
}

#endif
/**********************************************************************/
