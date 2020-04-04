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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <stdint.h>
#include <assert.h>
#include <locale.h>

/**********************************************************************/
#ifdef __UCLIBC_HAS_CTYPE_TABLES__

#ifdef __UCLIBC_HAS_CTYPE_SIGNED__

#if EOF >= CHAR_MIN
#define CTYPE_DOMAIN_CHECK(C) \
	(((unsigned int)((C) - CHAR_MIN)) <= (UCHAR_MAX - CHAR_MIN))
#else
#define CTYPE_DOMAIN_CHECK(C) \
	((((unsigned int)((C) - CHAR_MIN)) <= (UCHAR_MAX - CHAR_MIN)) || ((C) == EOF))
#endif

#else  /* __UCLIBC_HAS_CTYPE_SIGNED__ */

#if EOF == -1
#define CTYPE_DOMAIN_CHECK(C) \
	(((unsigned int)((C) - EOF)) <= (UCHAR_MAX - EOF))
#else
#define CTYPE_DOMAIN_CHECK(C) \
	((((unsigned int)(C)) <= UCHAR_MAX) || ((C) == EOF))
#endif

#endif /* __UCLIBC_HAS_CTYPE_SIGNED__ */

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */

#undef PASTE2
#define PASTE2(X,Y)    X ## Y

#ifdef __UCLIBC_HAS_CTYPE_TABLES__

#undef CTYPE_NAME
#undef ISCTYPE
#undef CTYPE_ALIAS
#undef CTYPE_DEF
#ifdef __UCLIBC_DO_XLOCALE
#define CTYPE_NAME(X)  __is ## X ## _l
#define ISCTYPE(C,F)   __isctype_l( C, F, locale_arg)
#define CTYPE_ALIAS(NAME)   strong_alias( __is ## NAME ## _l , is ## NAME ## _l)
#define CTYPE_DEF(NAME) libc_hidden_def(is ## NAME ## _l)
#else
#define CTYPE_NAME(X)  is ## X
#define ISCTYPE(C,F)   __isctype( C, F )
#define CTYPE_ALIAS(NAME)
#define CTYPE_DEF(NAME) libc_hidden_def(is ## NAME)
#endif


#undef CTYPE_BODY

#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
/* Make sure assert is active for to*() funcs below. */
#undef NDEBUG
#include <assert.h>

extern void __isctype_assert(int c, int mask) __attribute__ ((__noreturn__)) attribute_hidden;

#define CTYPE_BODY(NAME,C,MASK) \
	if (CTYPE_DOMAIN_CHECK(C)) { \
		return ISCTYPE(C, MASK); \
	} \
	__isctype_assert(C, MASK);

#elif defined(__UCLIBC_HAS_CTYPE_CHECKED__)

#define CTYPE_BODY(NAME,C,MASK) \
	return CTYPE_DOMAIN_CHECK(C) \
		? ISCTYPE(C, MASK) \
		: 0;

#elif defined(__UCLIBC_HAS_CTYPE_UNSAFE__)

#define CTYPE_BODY(NAME,C,MASK) \
	return ISCTYPE(C, MASK);


#else  /* No checking done. */

#error Unknown type of ctype checking!

#endif



#define IS_FUNC_BODY(NAME) \
int CTYPE_NAME(NAME) (int c  __LOCALE_PARAM ); \
int CTYPE_NAME(NAME) (int c  __LOCALE_PARAM ) \
{ \
	CTYPE_BODY(NAME,c,PASTE2(_IS,NAME)) \
} \
CTYPE_DEF(NAME) \
CTYPE_ALIAS(NAME)

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

#define C_MACRO(X)		PASTE2(__C_is,X)(c)
#define CTYPE_NAME(X)  is ## X
#define CTYPE_DEF(NAME) libc_hidden_def(is ## NAME)

#define IS_FUNC_BODY(NAME) \
int CTYPE_NAME(NAME) (int c) \
{ \
	return C_MACRO(NAME); \
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */
/**********************************************************************/
#ifdef L___ctype_assert
#ifdef __UCLIBC_HAS_CTYPE_ENFORCED__


attribute_hidden void __isctype_assert(int c, int mask)
{
	fprintf(stderr,	"%s: __is*{_l}(%d,%#x {locale})\n", __uclibc_progname, c, mask);
	abort();
}

#endif
#endif
/**********************************************************************/
#if defined(L_isalnum) || defined(L_isalnum_l)

IS_FUNC_BODY(alnum);

#endif
/**********************************************************************/
#if defined(L_isalpha) || defined(L_isalpha_l)

IS_FUNC_BODY(alpha);

#endif
/**********************************************************************/
#if defined(L_isblank) || defined(L_isblank_l)

IS_FUNC_BODY(blank);

#endif
/**********************************************************************/
#if defined(L_iscntrl) || defined(L_iscntrl_l)

IS_FUNC_BODY(cntrl);

#endif
/**********************************************************************/
#if defined(L_isdigit) || defined(L_isdigit_l)

#ifdef __UCLIBC_HAS_CTYPE_TABLES__

/* The standards require EOF < 0. */
#if EOF >= CHAR_MIN
#define __isdigit_char_or_EOF(C)   __isdigit_char((C))
#else
#define __isdigit_char_or_EOF(C)   __isdigit_int((C))
#endif

int CTYPE_NAME(digit) (int C   __LOCALE_PARAM);
int CTYPE_NAME(digit) (int C   __LOCALE_PARAM)
{
#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
	if (CTYPE_DOMAIN_CHECK(C)) {
		return __isdigit_char_or_EOF(C); /* C is (unsigned) char or EOF. */
	}
	__isctype_assert(C, _ISdigit);
#else
	return __isdigit_int(C);	/* C could be invalid. */
#endif
}
CTYPE_DEF(digit)
CTYPE_ALIAS(digit)

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

IS_FUNC_BODY(digit);

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */

#endif
/**********************************************************************/
#if defined(L_isgraph) || defined(L_isgraph_l)

IS_FUNC_BODY(graph);

#endif
/**********************************************************************/
#if defined(L_islower) || defined(L_islower_l)

IS_FUNC_BODY(lower);

#endif
/**********************************************************************/
#if defined(L_isprint) || defined(L_isprint_l)

IS_FUNC_BODY(print);

#endif
/**********************************************************************/
#if defined(L_ispunct) || defined(L_ispunct_l)

IS_FUNC_BODY(punct);

#endif
/**********************************************************************/
#if defined(L_isspace) || defined(L_isspace_l)

IS_FUNC_BODY(space);

#endif
/**********************************************************************/
#if defined(L_isupper) || defined(L_isupper_l)

IS_FUNC_BODY(upper);

#endif
/**********************************************************************/
#if defined(L_isxdigit) || defined(L_isxdigit_l)

IS_FUNC_BODY(xdigit);

#endif
/**********************************************************************/
#ifdef L_tolower

#undef tolower
#ifdef __UCLIBC_HAS_CTYPE_TABLES__

int tolower(int c)
{
#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
	assert(CTYPE_DOMAIN_CHECK(c));
#endif
	return __UCLIBC_CTYPE_IN_TO_DOMAIN(c) ? (__UCLIBC_CTYPE_TOLOWER)[c] : c;
}

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

int tolower(int c)
{
	return __C_tolower(c);
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */
libc_hidden_def(tolower)

#endif
/**********************************************************************/
#ifdef L_tolower_l

#undef tolower_l
int tolower_l(int c, __locale_t l)
{
#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
	assert(CTYPE_DOMAIN_CHECK(c));
#endif
	return __UCLIBC_CTYPE_IN_TO_DOMAIN(c) ? l->__ctype_tolower[c] : c;
}
libc_hidden_def(tolower_l)

#endif
/**********************************************************************/
#ifdef L_toupper

#undef toupper
#ifdef __UCLIBC_HAS_CTYPE_TABLES__

int toupper(int c)
{
#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
	assert(CTYPE_DOMAIN_CHECK(c));
#endif
	return __UCLIBC_CTYPE_IN_TO_DOMAIN(c) ? (__UCLIBC_CTYPE_TOUPPER)[c] : c;
}

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

int toupper(int c)
{
	return __C_toupper(c);
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */
libc_hidden_def(toupper)

#endif
/**********************************************************************/
#ifdef L_toupper_l

#undef toupper_l
int toupper_l(int c, __locale_t l)
{
#if defined(__UCLIBC_HAS_CTYPE_ENFORCED__)
	assert(CTYPE_DOMAIN_CHECK(c));
#endif
	return __UCLIBC_CTYPE_IN_TO_DOMAIN(c) ? l->__ctype_toupper[c] : c;
}

#endif
/**********************************************************************/
#if defined(L_isascii) || defined(L_isascii_l)

#ifdef __UCLIBC_HAS_CTYPE_TABLES__

int __XL_NPP(isascii)(int c);
int __XL_NPP(isascii)(int c)
{
	return __isascii(c);		/* locale-independent */
}

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

int isascii(int c)
{
	return __isascii(c);		/* locale-independent */
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */
CTYPE_DEF(ascii)


#endif
/**********************************************************************/
#if defined(L_toascii) || defined(L_toascii_l)

#ifdef __UCLIBC_HAS_CTYPE_TABLES__

int __XL_NPP(toascii)(int c);
int __XL_NPP(toascii)(int c)
{
	return __toascii(c);		/* locale-independent */
}

#else  /* __UCLIBC_HAS_CTYPE_TABLES__ */

int toascii(int c)
{
	return __toascii(c);		/* locale-independent */
}

#endif /* __UCLIBC_HAS_CTYPE_TABLES__ */

#endif
/**********************************************************************/
/* glibc extensions */
/**********************************************************************/
#ifdef L_isctype

int isctype(int c, int mask)
{
	CTYPE_BODY(NAME,c,mask)
}

#endif
/**********************************************************************/
#ifdef L___ctype_b_loc

#ifdef __UCLIBC_HAS_XLOCALE__

const __ctype_mask_t **__ctype_b_loc(void)
{
	return &(__UCLIBC_CURLOCALE->__ctype_b);
}

libc_hidden_def(__ctype_b_loc)
#endif

#endif
/**********************************************************************/
#ifdef L___ctype_tolower_loc

#ifdef __UCLIBC_HAS_XLOCALE__

const __ctype_touplow_t **__ctype_tolower_loc(void)
{
	return &(__UCLIBC_CURLOCALE->__ctype_tolower);
}
libc_hidden_def(__ctype_tolower_loc)

#endif

#endif
/**********************************************************************/
#ifdef L___ctype_toupper_loc

#ifdef __UCLIBC_HAS_XLOCALE__

const __ctype_touplow_t **__ctype_toupper_loc(void)
{
	return &(__UCLIBC_CURLOCALE->__ctype_toupper);
}
libc_hidden_def(__ctype_toupper_loc)

#endif

#endif
/**********************************************************************/
#ifdef L___C_ctype_b

static const __ctype_mask_t __C_ctype_b_data[] = {
#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
	/* -128  M-^@ */ 0,
	/* -127  M-^A */ 0,
	/* -126  M-^B */ 0,
	/* -125  M-^C */ 0,
	/* -124  M-^D */ 0,
	/* -123  M-^E */ 0,
	/* -122  M-^F */ 0,
	/* -121  M-^G */ 0,
	/* -120  M-^H */ 0,
	/* -119  M-^I */ 0,
	/* -118  M-^J */ 0,
	/* -117  M-^K */ 0,
	/* -116  M-^L */ 0,
	/* -115  M-^M */ 0,
	/* -114  M-^N */ 0,
	/* -113  M-^O */ 0,
	/* -112  M-^P */ 0,
	/* -111  M-^Q */ 0,
	/* -110  M-^R */ 0,
	/* -109  M-^S */ 0,
	/* -108  M-^T */ 0,
	/* -107  M-^U */ 0,
	/* -106  M-^V */ 0,
	/* -105  M-^W */ 0,
	/* -104  M-^X */ 0,
	/* -103  M-^Y */ 0,
	/* -102  M-^Z */ 0,
	/* -101  M-^[ */ 0,
	/* -100  M-^\ */ 0,
	/*  -99  M-^] */ 0,
	/*  -98  M-^^ */ 0,
	/*  -97  M-^_ */ 0,
	/*  -96  M-   */ 0,
	/*  -95  M-!  */ 0,
	/*  -94  M-"  */ 0,
	/*  -93  M-#  */ 0,
	/*  -92  M-$  */ 0,
	/*  -91  M-%  */ 0,
	/*  -90  M-&  */ 0,
	/*  -89  M-'  */ 0,
	/*  -88  M-(  */ 0,
	/*  -87  M-)  */ 0,
	/*  -86  M-*  */ 0,
	/*  -85  M-+  */ 0,
	/*  -84  M-,  */ 0,
	/*  -83  M--  */ 0,
	/*  -82  M-.  */ 0,
	/*  -81  M-/  */ 0,
	/*  -80  M-0  */ 0,
	/*  -79  M-1  */ 0,
	/*  -78  M-2  */ 0,
	/*  -77  M-3  */ 0,
	/*  -76  M-4  */ 0,
	/*  -75  M-5  */ 0,
	/*  -74  M-6  */ 0,
	/*  -73  M-7  */ 0,
	/*  -72  M-8  */ 0,
	/*  -71  M-9  */ 0,
	/*  -70  M-:  */ 0,
	/*  -69  M-;  */ 0,
	/*  -68  M-<  */ 0,
	/*  -67  M-=  */ 0,
	/*  -66  M->  */ 0,
	/*  -65  M-?  */ 0,
	/*  -64  M-@  */ 0,
	/*  -63  M-A  */ 0,
	/*  -62  M-B  */ 0,
	/*  -61  M-C  */ 0,
	/*  -60  M-D  */ 0,
	/*  -59  M-E  */ 0,
	/*  -58  M-F  */ 0,
	/*  -57  M-G  */ 0,
	/*  -56  M-H  */ 0,
	/*  -55  M-I  */ 0,
	/*  -54  M-J  */ 0,
	/*  -53  M-K  */ 0,
	/*  -52  M-L  */ 0,
	/*  -51  M-M  */ 0,
	/*  -50  M-N  */ 0,
	/*  -49  M-O  */ 0,
	/*  -48  M-P  */ 0,
	/*  -47  M-Q  */ 0,
	/*  -46  M-R  */ 0,
	/*  -45  M-S  */ 0,
	/*  -44  M-T  */ 0,
	/*  -43  M-U  */ 0,
	/*  -42  M-V  */ 0,
	/*  -41  M-W  */ 0,
	/*  -40  M-X  */ 0,
	/*  -39  M-Y  */ 0,
	/*  -38  M-Z  */ 0,
	/*  -37  M-[  */ 0,
	/*  -36  M-\  */ 0,
	/*  -35  M-]  */ 0,
	/*  -34  M-^  */ 0,
	/*  -33  M-_  */ 0,
	/*  -32  M-`  */ 0,
	/*  -31  M-a  */ 0,
	/*  -30  M-b  */ 0,
	/*  -29  M-c  */ 0,
	/*  -28  M-d  */ 0,
	/*  -27  M-e  */ 0,
	/*  -26  M-f  */ 0,
	/*  -25  M-g  */ 0,
	/*  -24  M-h  */ 0,
	/*  -23  M-i  */ 0,
	/*  -22  M-j  */ 0,
	/*  -21  M-k  */ 0,
	/*  -20  M-l  */ 0,
	/*  -19  M-m  */ 0,
	/*  -18  M-n  */ 0,
	/*  -17  M-o  */ 0,
	/*  -16  M-p  */ 0,
	/*  -15  M-q  */ 0,
	/*  -14  M-r  */ 0,
	/*  -13  M-s  */ 0,
	/*  -12  M-t  */ 0,
	/*  -11  M-u  */ 0,
	/*  -10  M-v  */ 0,
	/*   -9  M-w  */ 0,
	/*   -8  M-x  */ 0,
	/*   -7  M-y  */ 0,
	/*   -6  M-z  */ 0,
	/*   -5  M-{  */ 0,
	/*   -4  M-|  */ 0,
	/*   -3  M-}  */ 0,
	/*   -2  M-~  */ 0,
#endif /* __UCLIBC_HAS_CTYPE_SIGNED__*/
	/*   -1  M-^? */ 0,
	/*    0  ^@   */ _IScntrl,
	/*    1  ^A   */ _IScntrl,
	/*    2  ^B   */ _IScntrl,
	/*    3  ^C   */ _IScntrl,
	/*    4  ^D   */ _IScntrl,
	/*    5  ^E   */ _IScntrl,
	/*    6  ^F   */ _IScntrl,
	/*    7  ^G   */ _IScntrl,
	/*    8  ^H   */ _IScntrl,
	/*    9  ^I   */ _ISspace|_ISblank|_IScntrl,
	/*   10  ^J   */ _ISspace|_IScntrl,
	/*   11  ^K   */ _ISspace|_IScntrl,
	/*   12  ^L   */ _ISspace|_IScntrl,
	/*   13  ^M   */ _ISspace|_IScntrl,
	/*   14  ^N   */ _IScntrl,
	/*   15  ^O   */ _IScntrl,
	/*   16  ^P   */ _IScntrl,
	/*   17  ^Q   */ _IScntrl,
	/*   18  ^R   */ _IScntrl,
	/*   19  ^S   */ _IScntrl,
	/*   20  ^T   */ _IScntrl,
	/*   21  ^U   */ _IScntrl,
	/*   22  ^V   */ _IScntrl,
	/*   23  ^W   */ _IScntrl,
	/*   24  ^X   */ _IScntrl,
	/*   25  ^Y   */ _IScntrl,
	/*   26  ^Z   */ _IScntrl,
	/*   27  ^[   */ _IScntrl,
	/*   28  ^\   */ _IScntrl,
	/*   29  ^]   */ _IScntrl,
	/*   30  ^^   */ _IScntrl,
	/*   31  ^_   */ _IScntrl,
	/*   32       */ _ISspace|_ISprint|_ISblank,
	/*   33  !    */ _ISprint|_ISgraph|_ISpunct,
	/*   34  "    */ _ISprint|_ISgraph|_ISpunct,
	/*   35  #    */ _ISprint|_ISgraph|_ISpunct,
	/*   36  $    */ _ISprint|_ISgraph|_ISpunct,
	/*   37  %    */ _ISprint|_ISgraph|_ISpunct,
	/*   38  &    */ _ISprint|_ISgraph|_ISpunct,
	/*   39  '    */ _ISprint|_ISgraph|_ISpunct,
	/*   40  (    */ _ISprint|_ISgraph|_ISpunct,
	/*   41  )    */ _ISprint|_ISgraph|_ISpunct,
	/*   42  *    */ _ISprint|_ISgraph|_ISpunct,
	/*   43  +    */ _ISprint|_ISgraph|_ISpunct,
	/*   44  ,    */ _ISprint|_ISgraph|_ISpunct,
	/*   45  -    */ _ISprint|_ISgraph|_ISpunct,
	/*   46  .    */ _ISprint|_ISgraph|_ISpunct,
	/*   47  /    */ _ISprint|_ISgraph|_ISpunct,
	/*   48  0    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   49  1    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   50  2    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   51  3    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   52  4    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   53  5    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   54  6    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   55  7    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   56  8    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   57  9    */ _ISdigit|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   58  :    */ _ISprint|_ISgraph|_ISpunct,
	/*   59  ;    */ _ISprint|_ISgraph|_ISpunct,
	/*   60  <    */ _ISprint|_ISgraph|_ISpunct,
	/*   61  =    */ _ISprint|_ISgraph|_ISpunct,
	/*   62  >    */ _ISprint|_ISgraph|_ISpunct,
	/*   63  ?    */ _ISprint|_ISgraph|_ISpunct,
	/*   64  @    */ _ISprint|_ISgraph|_ISpunct,
	/*   65  A    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   66  B    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   67  C    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   68  D    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   69  E    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   70  F    */ _ISupper|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   71  G    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   72  H    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   73  I    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   74  J    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   75  K    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   76  L    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   77  M    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   78  N    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   79  O    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   80  P    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   81  Q    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   82  R    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   83  S    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   84  T    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   85  U    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   86  V    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   87  W    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   88  X    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   89  Y    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   90  Z    */ _ISupper|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*   91  [    */ _ISprint|_ISgraph|_ISpunct,
	/*   92  \    */ _ISprint|_ISgraph|_ISpunct,
	/*   93  ]    */ _ISprint|_ISgraph|_ISpunct,
	/*   94  ^    */ _ISprint|_ISgraph|_ISpunct,
	/*   95  _    */ _ISprint|_ISgraph|_ISpunct,
	/*   96  `    */ _ISprint|_ISgraph|_ISpunct,
	/*   97  a    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   98  b    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*   99  c    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*  100  d    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*  101  e    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*  102  f    */ _ISlower|_ISalpha|_ISxdigit|_ISprint|_ISgraph|_ISalnum,
	/*  103  g    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  104  h    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  105  i    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  106  j    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  107  k    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  108  l    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  109  m    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  110  n    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  111  o    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  112  p    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  113  q    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  114  r    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  115  s    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  116  t    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  117  u    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  118  v    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  119  w    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  120  x    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  121  y    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  122  z    */ _ISlower|_ISalpha|_ISprint|_ISgraph|_ISalnum,
	/*  123  {    */ _ISprint|_ISgraph|_ISpunct,
	/*  124  |    */ _ISprint|_ISgraph|_ISpunct,
	/*  125  }    */ _ISprint|_ISgraph|_ISpunct,
	/*  126  ~    */ _ISprint|_ISgraph|_ISpunct,
	/*  127  ^?   */ _IScntrl,
	/*  128  M-^@ */ 0,
	/*  129  M-^A */ 0,
	/*  130  M-^B */ 0,
	/*  131  M-^C */ 0,
	/*  132  M-^D */ 0,
	/*  133  M-^E */ 0,
	/*  134  M-^F */ 0,
	/*  135  M-^G */ 0,
	/*  136  M-^H */ 0,
	/*  137  M-^I */ 0,
	/*  138  M-^J */ 0,
	/*  139  M-^K */ 0,
	/*  140  M-^L */ 0,
	/*  141  M-^M */ 0,
	/*  142  M-^N */ 0,
	/*  143  M-^O */ 0,
	/*  144  M-^P */ 0,
	/*  145  M-^Q */ 0,
	/*  146  M-^R */ 0,
	/*  147  M-^S */ 0,
	/*  148  M-^T */ 0,
	/*  149  M-^U */ 0,
	/*  150  M-^V */ 0,
	/*  151  M-^W */ 0,
	/*  152  M-^X */ 0,
	/*  153  M-^Y */ 0,
	/*  154  M-^Z */ 0,
	/*  155  M-^[ */ 0,
	/*  156  M-^\ */ 0,
	/*  157  M-^] */ 0,
	/*  158  M-^^ */ 0,
	/*  159  M-^_ */ 0,
	/*  160  M-   */ 0,
	/*  161  M-!  */ 0,
	/*  162  M-"  */ 0,
	/*  163  M-#  */ 0,
	/*  164  M-$  */ 0,
	/*  165  M-%  */ 0,
	/*  166  M-&  */ 0,
	/*  167  M-'  */ 0,
	/*  168  M-(  */ 0,
	/*  169  M-)  */ 0,
	/*  170  M-*  */ 0,
	/*  171  M-+  */ 0,
	/*  172  M-,  */ 0,
	/*  173  M--  */ 0,
	/*  174  M-.  */ 0,
	/*  175  M-/  */ 0,
	/*  176  M-0  */ 0,
	/*  177  M-1  */ 0,
	/*  178  M-2  */ 0,
	/*  179  M-3  */ 0,
	/*  180  M-4  */ 0,
	/*  181  M-5  */ 0,
	/*  182  M-6  */ 0,
	/*  183  M-7  */ 0,
	/*  184  M-8  */ 0,
	/*  185  M-9  */ 0,
	/*  186  M-:  */ 0,
	/*  187  M-;  */ 0,
	/*  188  M-<  */ 0,
	/*  189  M-=  */ 0,
	/*  190  M->  */ 0,
	/*  191  M-?  */ 0,
	/*  192  M-@  */ 0,
	/*  193  M-A  */ 0,
	/*  194  M-B  */ 0,
	/*  195  M-C  */ 0,
	/*  196  M-D  */ 0,
	/*  197  M-E  */ 0,
	/*  198  M-F  */ 0,
	/*  199  M-G  */ 0,
	/*  200  M-H  */ 0,
	/*  201  M-I  */ 0,
	/*  202  M-J  */ 0,
	/*  203  M-K  */ 0,
	/*  204  M-L  */ 0,
	/*  205  M-M  */ 0,
	/*  206  M-N  */ 0,
	/*  207  M-O  */ 0,
	/*  208  M-P  */ 0,
	/*  209  M-Q  */ 0,
	/*  210  M-R  */ 0,
	/*  211  M-S  */ 0,
	/*  212  M-T  */ 0,
	/*  213  M-U  */ 0,
	/*  214  M-V  */ 0,
	/*  215  M-W  */ 0,
	/*  216  M-X  */ 0,
	/*  217  M-Y  */ 0,
	/*  218  M-Z  */ 0,
	/*  219  M-[  */ 0,
	/*  220  M-\  */ 0,
	/*  221  M-]  */ 0,
	/*  222  M-^  */ 0,
	/*  223  M-_  */ 0,
	/*  224  M-`  */ 0,
	/*  225  M-a  */ 0,
	/*  226  M-b  */ 0,
	/*  227  M-c  */ 0,
	/*  228  M-d  */ 0,
	/*  229  M-e  */ 0,
	/*  230  M-f  */ 0,
	/*  231  M-g  */ 0,
	/*  232  M-h  */ 0,
	/*  233  M-i  */ 0,
	/*  234  M-j  */ 0,
	/*  235  M-k  */ 0,
	/*  236  M-l  */ 0,
	/*  237  M-m  */ 0,
	/*  238  M-n  */ 0,
	/*  239  M-o  */ 0,
	/*  240  M-p  */ 0,
	/*  241  M-q  */ 0,
	/*  242  M-r  */ 0,
	/*  243  M-s  */ 0,
	/*  244  M-t  */ 0,
	/*  245  M-u  */ 0,
	/*  246  M-v  */ 0,
	/*  247  M-w  */ 0,
	/*  248  M-x  */ 0,
	/*  249  M-y  */ 0,
	/*  250  M-z  */ 0,
	/*  251  M-{  */ 0,
	/*  252  M-|  */ 0,
	/*  253  M-}  */ 0,
	/*  254  M-~  */ 0,
	/*  255  M-^? */ 0
};

const __ctype_mask_t *__C_ctype_b = __C_ctype_b_data + __UCLIBC_CTYPE_B_TBL_OFFSET;
libc_hidden_data_def(__C_ctype_b)

#ifndef __UCLIBC_HAS_XLOCALE__

const __ctype_mask_t *__ctype_b = __C_ctype_b_data + __UCLIBC_CTYPE_B_TBL_OFFSET;
libc_hidden_data_def(__ctype_b)

#endif

#endif
/**********************************************************************/
#ifdef L___C_ctype_tolower

static const __ctype_touplow_t __C_ctype_tolower_data[] = {
#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
	-128,         -127,         -126,         -125,
	-124,         -123,         -122,         -121,
	-120,         -119,         -118,         -117,
	-116,         -115,         -114,         -113,
	-112,         -111,         -110,         -109,
	-108,         -107,         -106,         -105,
	-104,         -103,         -102,         -101,
	-100,          -99,          -98,          -97,
	 -96,          -95,          -94,          -93,
	 -92,          -91,          -90,          -89,
	 -88,          -87,          -86,          -85,
	 -84,          -83,          -82,          -81,
	 -80,          -79,          -78,          -77,
	 -76,          -75,          -74,          -73,
	 -72,          -71,          -70,          -69,
	 -68,          -67,          -66,          -65,
	 -64,          -63,          -62,          -61,
	 -60,          -59,          -58,          -57,
	 -56,          -55,          -54,          -53,
	 -52,          -51,          -50,          -49,
	 -48,          -47,          -46,          -45,
	 -44,          -43,          -42,          -41,
	 -40,          -39,          -38,          -37,
	 -36,          -35,          -34,          -33,
	 -32,          -31,          -30,          -29,
	 -28,          -27,          -26,          -25,
	 -24,          -23,          -22,          -21,
	 -20,          -19,          -18,          -17,
	 -16,          -15,          -14,          -13,
	 -12,          -11,          -10,           -9,
	  -8,           -7,           -6,           -5,
	  -4,           -3,           -2,           -1,
#endif /* __UCLIBC_HAS_CTYPE_SIGNED__*/
	   0,            1,            2,            3,
	   4,            5,            6,            7,
	   8,            9,           10,           11,
	  12,           13,           14,           15,
	  16,           17,           18,           19,
	  20,           21,           22,           23,
	  24,           25,           26,           27,
	  28,           29,           30,           31,
	  32,           33,           34,           35,
	  36,           37,           38,           39,
	  40,           41,           42,           43,
	  44,           45,           46,           47,
	  48,           49,           50,           51,
	  52,           53,           54,           55,
	  56,           57,           58,           59,
	  60,           61,           62,           63,
	  64,           97 /* a */,   98 /* b */,   99 /* c */,
	 100 /* d */,  101 /* e */,  102 /* f */,  103 /* g */,
	 104 /* h */,  105 /* i */,  106 /* j */,  107 /* k */,
	 108 /* l */,  109 /* m */,  110 /* n */,  111 /* o */,
	 112 /* p */,  113 /* q */,  114 /* r */,  115 /* s */,
	 116 /* t */,  117 /* u */,  118 /* v */,  119 /* w */,
	 120 /* x */,  121 /* y */,  122 /* z */,   91,
	  92,           93,           94,           95,
	  96,           97,           98,           99,
	 100,          101,          102,          103,
	 104,          105,          106,          107,
	 108,          109,          110,          111,
	 112,          113,          114,          115,
	 116,          117,          118,          119,
	 120,          121,          122,          123,
	 124,          125,          126,          127,
	 128,          129,          130,          131,
	 132,          133,          134,          135,
	 136,          137,          138,          139,
	 140,          141,          142,          143,
	 144,          145,          146,          147,
	 148,          149,          150,          151,
	 152,          153,          154,          155,
	 156,          157,          158,          159,
	 160,          161,          162,          163,
	 164,          165,          166,          167,
	 168,          169,          170,          171,
	 172,          173,          174,          175,
	 176,          177,          178,          179,
	 180,          181,          182,          183,
	 184,          185,          186,          187,
	 188,          189,          190,          191,
	 192,          193,          194,          195,
	 196,          197,          198,          199,
	 200,          201,          202,          203,
	 204,          205,          206,          207,
	 208,          209,          210,          211,
	 212,          213,          214,          215,
	 216,          217,          218,          219,
	 220,          221,          222,          223,
	 224,          225,          226,          227,
	 228,          229,          230,          231,
	 232,          233,          234,          235,
	 236,          237,          238,          239,
	 240,          241,          242,          243,
	 244,          245,          246,          247,
	 248,          249,          250,          251,
	 252,          253,          254,          255
};

const __ctype_touplow_t *__C_ctype_tolower =
		__C_ctype_tolower_data + __UCLIBC_CTYPE_TO_TBL_OFFSET;
libc_hidden_data_def(__C_ctype_tolower)

#ifndef __UCLIBC_HAS_XLOCALE__

const __ctype_touplow_t *__ctype_tolower =
		__C_ctype_tolower_data + __UCLIBC_CTYPE_TO_TBL_OFFSET;
libc_hidden_data_def(__ctype_tolower)

#endif

#endif
/**********************************************************************/
#ifdef L___C_ctype_toupper

static const __ctype_touplow_t __C_ctype_toupper_data[] = {
#ifdef __UCLIBC_HAS_CTYPE_SIGNED__
	-128,         -127,         -126,         -125,
	-124,         -123,         -122,         -121,
	-120,         -119,         -118,         -117,
	-116,         -115,         -114,         -113,
	-112,         -111,         -110,         -109,
	-108,         -107,         -106,         -105,
	-104,         -103,         -102,         -101,
	-100,          -99,          -98,          -97,
	 -96,          -95,          -94,          -93,
	 -92,          -91,          -90,          -89,
	 -88,          -87,          -86,          -85,
	 -84,          -83,          -82,          -81,
	 -80,          -79,          -78,          -77,
	 -76,          -75,          -74,          -73,
	 -72,          -71,          -70,          -69,
	 -68,          -67,          -66,          -65,
	 -64,          -63,          -62,          -61,
	 -60,          -59,          -58,          -57,
	 -56,          -55,          -54,          -53,
	 -52,          -51,          -50,          -49,
	 -48,          -47,          -46,          -45,
	 -44,          -43,          -42,          -41,
	 -40,          -39,          -38,          -37,
	 -36,          -35,          -34,          -33,
	 -32,          -31,          -30,          -29,
	 -28,          -27,          -26,          -25,
	 -24,          -23,          -22,          -21,
	 -20,          -19,          -18,          -17,
	 -16,          -15,          -14,          -13,
	 -12,          -11,          -10,           -9,
	  -8,           -7,           -6,           -5,
	  -4,           -3,           -2,           -1,
#endif /* __UCLIBC_HAS_CTYPE_SIGNED__*/
	   0,            1,            2,            3,
	   4,            5,            6,            7,
	   8,            9,           10,           11,
	  12,           13,           14,           15,
	  16,           17,           18,           19,
	  20,           21,           22,           23,
	  24,           25,           26,           27,
	  28,           29,           30,           31,
	  32,           33,           34,           35,
	  36,           37,           38,           39,
	  40,           41,           42,           43,
	  44,           45,           46,           47,
	  48,           49,           50,           51,
	  52,           53,           54,           55,
	  56,           57,           58,           59,
	  60,           61,           62,           63,
	  64,           65,           66,           67,
	  68,           69,           70,           71,
	  72,           73,           74,           75,
	  76,           77,           78,           79,
	  80,           81,           82,           83,
	  84,           85,           86,           87,
	  88,           89,           90,           91,
	  92,           93,           94,           95,
	  96,           65 /* A */,   66 /* B */,   67 /* C */,
	  68 /* D */,   69 /* E */,   70 /* F */,   71 /* G */,
	  72 /* H */,   73 /* I */,   74 /* J */,   75 /* K */,
	  76 /* L */,   77 /* M */,   78 /* N */,   79 /* O */,
	  80 /* P */,   81 /* Q */,   82 /* R */,   83 /* S */,
	  84 /* T */,   85 /* U */,   86 /* V */,   87 /* W */,
	  88 /* X */,   89 /* Y */,   90 /* Z */,  123,
	 124,          125,          126,          127,
	 128,          129,          130,          131,
	 132,          133,          134,          135,
	 136,          137,          138,          139,
	 140,          141,          142,          143,
	 144,          145,          146,          147,
	 148,          149,          150,          151,
	 152,          153,          154,          155,
	 156,          157,          158,          159,
	 160,          161,          162,          163,
	 164,          165,          166,          167,
	 168,          169,          170,          171,
	 172,          173,          174,          175,
	 176,          177,          178,          179,
	 180,          181,          182,          183,
	 184,          185,          186,          187,
	 188,          189,          190,          191,
	 192,          193,          194,          195,
	 196,          197,          198,          199,
	 200,          201,          202,          203,
	 204,          205,          206,          207,
	 208,          209,          210,          211,
	 212,          213,          214,          215,
	 216,          217,          218,          219,
	 220,          221,          222,          223,
	 224,          225,          226,          227,
	 228,          229,          230,          231,
	 232,          233,          234,          235,
	 236,          237,          238,          239,
	 240,          241,          242,          243,
	 244,          245,          246,          247,
	 248,          249,          250,          251,
	 252,          253,          254,          255
};

const __ctype_touplow_t *__C_ctype_toupper =
		__C_ctype_toupper_data + __UCLIBC_CTYPE_TO_TBL_OFFSET;
libc_hidden_data_def(__C_ctype_toupper)

#ifndef __UCLIBC_HAS_XLOCALE__

const __ctype_touplow_t *__ctype_toupper =
		__C_ctype_toupper_data + __UCLIBC_CTYPE_TO_TBL_OFFSET;
libc_hidden_data_def(__ctype_toupper)

#endif

#endif
/**********************************************************************/
