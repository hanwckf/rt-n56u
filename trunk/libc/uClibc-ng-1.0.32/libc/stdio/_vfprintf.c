/*  Copyright (C) 2002-2004     Manuel Novoa III
 *  My stdio library for linux and (soon) elks.
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

/* This code needs a lot of clean up.  Some of that is on hold until uClibc
 * gets a better configuration system (on Erik's todo list).
 * The other cleanup will take place during the implementation/integration of
 * the wide char (un)formatted i/o functions which I'm currently working on.
 */

/*  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!
 *
 *  This code is currently under development.  Also, I plan to port
 *  it to elks which is a 16-bit environment with a fairly limited
 *  compiler.  Therefore, please refrain from modifying this code
 *  and, instead, pass any bug-fixes, etc. to me.  Thanks.  Manuel
 *
 *  ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION!   ATTENTION! */


/* April 1, 2002
 * Initialize thread locks for fake files in vsnprintf and vdprintf.
 *    reported by Erik Andersen (andersen@codepoet.com)
 * Fix an arg promotion handling bug in _do_one_spec for %c.
 *    reported by Ilguiz Latypov <ilatypov@superbt.com>
 *
 * May 10, 2002
 * Remove __isdigit and use new ctype.h version.
 * Add conditional setting of QUAL_CHARS for size_t and ptrdiff_t.
 *
 * Aug 16, 2002
 * Fix two problems that showed up with the python 2.2.1 tests; one
 *    involving %o and one involving %f.
 *
 * Oct 28, 2002
 * Fix a problem in vasprintf (reported by vodz a while back) when built
 *    without custom stream support.  In that case, it is necessary to do
 *    a va_copy.
 * Make sure each va_copy has a matching va_end, as required by C99.
 *
 * Nov 4, 2002
 * Add locale-specific grouping support for integer decimal conversion.
 * Add locale-specific decimal point support for floating point conversion.
 *   Note: grouping will have to wait for _dtostr() rewrite.
 * Add printf wchar support for %lc (%C) and %ls (%S).
 * Require printf format strings to be valid multibyte strings beginning and
 *   ending in their initial shift state, as per the stds.
 *
 * Nov 21, 2002
 * Add *wprintf functions.  Currently they don't support floating point
 *   conversions.  That will wait until the rewrite of _dtostr.
 *
 * Aug 1, 2003
 * Optional hexadecimal float notation support for %a/%A.
 * Floating point output now works for *wprintf.
 * Support for glibc locale-specific digit grouping for floats.
 * Misc bug fixes.
 *
 * Aug 31, 2003
 * Fix precision bug for %g conversion specifier when using %f style.
 *
 * Sep 5, 2003
 * Implement *s*scanf for the non-buffered stdio case with old_vfprintf.
 *
 * Sep 23, 2003
 * vfprintf was not always checking for narrow stream orientation.
 */

/* TODO:
 *
 * Should we validate that *printf format strings are valid multibyte
 *   strings in the current locale?  ANSI/ISO C99 seems to imply this
 *   and Plauger's printf implementation in his Standard C Library book
 *   treats this as an error.
 */

#include <features.h>
#include "_stdio.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <assert.h>
#include <stdint.h>
#include <errno.h>
#include <locale.h>

#ifdef __UCLIBC_HAS_THREADS__
# include <stdio_ext.h>
# include <pthread.h>
#endif

#ifdef __UCLIBC_HAS_WCHAR__
# include <wchar.h>
#endif

#include <bits/uClibc_uintmaxtostr.h>
#include <bits/uClibc_va_copy.h>

/* Some older or broken gcc toolchains define LONG_LONG_MAX but not
 * LLONG_MAX.  Since LLONG_MAX is part of the standard, that's what
 * we use.  So complain if we do not have it but should.
 */
#if !defined(LLONG_MAX) && defined(LONG_LONG_MAX)
#error Apparently, LONG_LONG_MAX is defined but LLONG_MAX is not.  You need to fix your toolchain headers to support the standard macros for (unsigned) long long.
#endif

#include "_fpmaxtostr.h"

#undef __STDIO_HAS_VSNPRINTF
#if defined(__STDIO_BUFFERS) || defined(__USE_OLD_VFPRINTF__) || defined(__UCLIBC_HAS_GLIBC_CUSTOM_STREAMS__)
# define __STDIO_HAS_VSNPRINTF 1
#endif

/**********************************************************************/

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
# define MAX_USER_SPEC       10
# define MAX_ARGS_PER_SPEC    5
#else
# undef MAX_USER_SPEC
# define MAX_ARGS_PER_SPEC    1
#endif

#if MAX_ARGS_PER_SPEC < 1
# error MAX_ARGS_PER_SPEC < 1!
# undef MAX_ARGS_PER_SPEC
# define MAX_ARGS_PER_SPEC    1
#endif

#if defined(NL_ARGMAX) && (NL_ARGMAX < 9)
# error NL_ARGMAX < 9!
#endif

#if defined(NL_ARGMAX) && (NL_ARGMAX >= (MAX_ARGS_PER_SPEC + 2))
# define MAX_ARGS        NL_ARGMAX
#else
/* N for spec itself, plus 1 each for width and precision */
# define MAX_ARGS        (MAX_ARGS_PER_SPEC + 2)
#endif

/**********************************************************************/

#define __PA_FLAG_INTMASK \
	(__PA_FLAG_CHAR|PA_FLAG_SHORT|__PA_FLAG_INT|PA_FLAG_LONG|PA_FLAG_LONG_LONG)

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
extern printf_function _custom_printf_handler[MAX_USER_SPEC] attribute_hidden;
extern printf_arginfo_function *_custom_printf_arginfo[MAX_USER_SPEC] attribute_hidden;
extern char *_custom_printf_spec attribute_hidden;
#endif

/**********************************************************************/

#define SPEC_FLAGS		" +0-#'I"
enum {
	FLAG_SPACE      = 0x01,
	FLAG_PLUS       = 0x02,	/* must be 2 * FLAG_SPACE */
	FLAG_ZERO       = 0x04,
	FLAG_MINUS      = 0x08,	/* must be 2 * FLAG_ZERO */
	FLAG_HASH       = 0x10,
	FLAG_THOUSANDS  = 0x20,
	FLAG_I18N       = 0x40,	/* only works for d, i, u */
	FLAG_WIDESTREAM = 0x80
};

/**********************************************************************/

/* float layout          01234567890123456789   TODO: B?*/
#define SPEC_CHARS		"npxXoudifFeEgGaACScs"
enum {
	CONV_n = 0,
	CONV_p,
	CONV_x, CONV_X,	CONV_o,	CONV_u,	CONV_d,	CONV_i,
	CONV_f, CONV_F, CONV_e, CONV_E, CONV_g, CONV_G, CONV_a, CONV_A,
	CONV_C, CONV_S, CONV_c, CONV_s,
#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
	CONV_m,
#endif
	CONV_custom0				/* must be last */
};

/*                         p   x   X  o   u   d   i */
#define SPEC_BASE       { 16, 16, 16, 8, 10, 10, 10 }

#define SPEC_RANGES     { CONV_n, CONV_p, CONV_i, CONV_A, \
                          CONV_C, CONV_S, CONV_c, CONV_s, CONV_custom0 }

#define SPEC_OR_MASK		 { \
	/* n */			(PA_FLAG_PTR|PA_INT), \
	/* p */			PA_POINTER, \
	/* oxXudi */	PA_INT, \
	/* fFeEgGaA */	PA_DOUBLE, \
	/* C */			PA_WCHAR, \
	/* S */			PA_WSTRING, \
	/* c */			PA_CHAR, \
	/* s */			PA_STRING, \
}

#define SPEC_AND_MASK		{ \
	/* n */			(PA_FLAG_PTR|__PA_INTMASK), \
	/* p */			PA_POINTER, \
	/* oxXudi */	(__PA_INTMASK), \
	/* fFeEgGaA */	(PA_FLAG_LONG_DOUBLE|PA_DOUBLE), \
	/* C */			(PA_WCHAR), \
	/* S */			(PA_WSTRING), \
	/* c */			(PA_CHAR), \
	/* s */			(PA_STRING), \
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
# error PDS already defined!
#endif
#ifdef SS
# error SS already defined!
#endif
#ifdef IMS
# error IMS already defined!
#endif

#if PTRDIFF_MAX == INT_MAX
# define PDS		0
#elif PTRDIFF_MAX == LONG_MAX
# define PDS		4
#elif defined(LLONG_MAX) && (PTRDIFF_MAX == LLONG_MAX)
# define PDS		8
#else
# error fix QUAL_CHARS ptrdiff_t entry 't'!
#endif

#if SIZE_MAX == UINT_MAX
# define SS		0
#elif SIZE_MAX == ULONG_MAX
# define SS		4
#elif defined(LLONG_MAX) && (SIZE_MAX == ULLONG_MAX)
# define SS		8
#else
# error fix QUAL_CHARS size_t entries 'z', 'Z'!
#endif

#if INTMAX_MAX == INT_MAX
# define IMS		0
#elif INTMAX_MAX == LONG_MAX
# define IMS		4
#elif defined(LLONG_MAX) && (INTMAX_MAX == LLONG_MAX)
# define IMS		8
#else
# error fix QUAL_CHARS intmax_t entry 'j'!
#endif

#define QUAL_CHARS		{ \
	/* j:(u)intmax_t z:(s)size_t  t:ptrdiff_t  \0:int */ \
	/* q:long_long  Z:(s)size_t */ \
	'h',   'l',  'L',  'j',  'z',  't',  'q', 'Z',  0, \
	 2,     4,    8,  IMS,   SS,  PDS,    8,  SS,   0, /* TODO -- fix!!! */\
	 1,     8 \
}

/**********************************************************************/

#ifdef __STDIO_VA_ARG_PTR
# ifdef __BCC__
#  define __va_arg_ptr(ap,type)		(((type *)(ap += sizeof(type))) - 1)
# endif

# if 1
#  ifdef __GNUC__
/* TODO -- need other than for 386 as well! */

#   ifndef __va_rounded_size
#    define __va_rounded_size(TYPE) \
	(((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))
#   endif
#   define __va_arg_ptr(AP, TYPE)  \
	(AP = (va_list) ((char *) (AP) + __va_rounded_size (TYPE)),  \
	 ((void *) ((char *) (AP) - __va_rounded_size (TYPE)))  \
	)
#  endif
# endif
#endif /* __STDIO_VA_ARG_PTR */

#ifdef __va_arg_ptr
# define GET_VA_ARG(AP,F,TYPE,ARGS)	(*(AP) = __va_arg_ptr(ARGS,TYPE))
# define GET_ARG_VALUE(AP,F,TYPE)	(*((TYPE *)(*(AP))))
#else
typedef union {
	wchar_t wc;
	unsigned int u;
	unsigned long ul;
# ifdef ULLONG_MAX
	unsigned long long ull;
# endif
# ifdef __UCLIBC_HAS_FLOATS__
	double d;
	long double ld;
# endif
	void *p;
} argvalue_t;

# define GET_VA_ARG(AU,F,TYPE,ARGS)	(AU->F = va_arg(ARGS,TYPE))
# define GET_ARG_VALUE(AU,F,TYPE)	((TYPE)((AU)->F))
#endif

typedef struct {
	const char *fmtpos;			/* TODO: move below struct?? */
	struct printf_info info;
#ifdef NL_ARGMAX
	int maxposarg;				/* > 0 if args are positional, 0 if not, -1 if unknown */
#endif
	int num_data_args;			/* TODO: use sentinal??? */
	unsigned int conv_num;
	unsigned char argnumber[4]; /* width | prec | 1st data | unused */
	int argtype[MAX_ARGS];
	va_list arg;
#ifdef __va_arg_ptr
	void *argptr[MAX_ARGS];
#else
/* if defined(NL_ARGMAX) || defined(__UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__) */
	/* While this is wasteful of space in the case where pos args aren't
	 * enabled, it is also needed to support custom printf handlers. */
	argvalue_t argvalue[MAX_ARGS];
#endif
} ppfs_t;						/* parse printf format state */

/**********************************************************************/

/* TODO: fix printf to return 0 and set errno if format error.  Standard says
   only returns -1 if sets error indicator for the stream. */

extern int _ppfs_init(ppfs_t *ppfs, const char *fmt0) attribute_hidden; /* validates */
extern void _ppfs_prepargs(ppfs_t *ppfs, va_list arg) attribute_hidden; /* sets posargptrs */
extern void _ppfs_setargs(ppfs_t *ppfs) attribute_hidden; /* sets argptrs for current spec */
extern int _ppfs_parsespec(ppfs_t *ppfs) attribute_hidden; /* parses specifier */

/**********************************************************************/
#ifdef L_parse_printf_format

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__

/* NOTE: This function differs from the glibc version in that parsing stops
 * upon encountering an invalid conversion specifier.  Since this is the way
 * my printf functions work, I think it makes sense to do it that way here.
 * Unfortunately, since glibc sets the return type as size_t, we have no way
 * of returning that the template is illegal, other than returning 0.
 */

size_t parse_printf_format(register const char *template,
						   size_t n, register int *argtypes)
{
	ppfs_t ppfs;
	size_t i;
	size_t count = 0;

	if (_ppfs_init(&ppfs, template) >= 0) {
#ifdef NL_ARGMAX
		if (ppfs.maxposarg > 0)  {
			/* Using positional args. */
			count = ppfs.maxposarg;
			if (n > count) {
				n = count;
			}
			for (i = 0 ; i < n ; i++) {
				*argtypes++ = ppfs.argtype[i];
			}
		} else
#endif
		{
			/* Not using positional args. */
			while (*template) {
				if ((*template == '%') && (*++template != '%')) {
					ppfs.fmtpos = template;
					_ppfs_parsespec(&ppfs); /* Can't fail. */
					template = ppfs.fmtpos; /* Update to one past spec end. */
					if (ppfs.info.width == INT_MIN) {
						++count;
						if (n > 0) {
							*argtypes++ = PA_INT;
							--n;
						}
					}
					if (ppfs.info.prec == INT_MIN) {
						++count;
						if (n > 0) {
							*argtypes++ = PA_INT;
							--n;
						}
					}
					for (i = 0 ; i < ppfs.num_data_args ; i++) {
						if ((ppfs.argtype[i]) != __PA_NOARG) {
							++count;
							if (n > 0) {
								*argtypes++ = ppfs.argtype[i];
								--n;
							}
						}
					}
				} else {
					++template;
				}
			}
		}
	}

	return count;
}

#endif

#endif
/**********************************************************************/
#ifdef L__ppfs_init

int attribute_hidden _ppfs_init(register ppfs_t *ppfs, const char *fmt0)
{
	int r;

	/* First, zero out everything... argnumber[], argtype[], argptr[] */
	memset(ppfs, 0, sizeof(ppfs_t)); /* TODO: nonportable???? */
#ifdef NL_ARGMAX
	--ppfs->maxposarg;			/* set to -1 */
#endif
	ppfs->fmtpos = fmt0;
#ifdef __UCLIBC_HAS_LOCALE__
	/* To support old programs, don't check mb validity if in C locale. */
	if (__UCLIBC_CURLOCALE->encoding != __ctype_encoding_7_bit) {
		/* ANSI/ISO C99 requires format string to be a valid multibyte string
		 * beginning and ending in its initial shift state. */
		static const char invalid_mbs[] = "Invalid multibyte format string.";
		mbstate_t mbstate;
		const char *p;
		mbstate.__mask = 0;	/* Initialize the mbstate. */
		p = fmt0;
		if (mbsrtowcs(NULL, &p, SIZE_MAX, &mbstate) == ((size_t)(-1))) {
			ppfs->fmtpos = invalid_mbs;
			return -1;
		}
	}
#endif /* __UCLIBC_HAS_LOCALE__ */
	/* now set all argtypes to no-arg */
	{
#if 1
		/* TODO - use memset here since already "paid for"? */
		register int *p = ppfs->argtype;

		r = MAX_ARGS;
		do {
			*p++ = __PA_NOARG;
		} while (--r);
#else
		/* TODO -- get rid of this?? */
		register char *p = (char *) ((MAX_ARGS-1) * sizeof(int));

		do {
			*((int *)(((char *)ppfs) + ((int)p) + offsetof(ppfs_t,argtype))) = __PA_NOARG;
			p -= sizeof(int);
		} while (p);
#endif
	}

	/*
	 * Run through the entire format string to validate it and initialize
	 * the positional arg numbers (if any).
	 */
	{
		register const char *fmt = fmt0;

		while (*fmt) {
			if ((*fmt == '%') && (*++fmt != '%')) {
				ppfs->fmtpos = fmt; /* back up to the '%' */
				r = _ppfs_parsespec(ppfs);
				if (r < 0) {
					return -1;
				}
				fmt = ppfs->fmtpos;	/* update to one past end of spec */
			} else {
				++fmt;
			}
		}
		ppfs->fmtpos = fmt0;		/* rewind */
	}

#ifdef NL_ARGMAX
	/* If we have positional args, make sure we know all the types. */
	{
		register int *p = ppfs->argtype;
		r = ppfs->maxposarg;
		while (--r >= 0) {
			if ( *p == __PA_NOARG ) { /* missing arg type!!! */
				return -1;
			}
			++p;
		}
	}
#endif /* NL_ARGMAX */

	return 0;
}
#endif
/**********************************************************************/
#ifdef L__ppfs_prepargs
void attribute_hidden _ppfs_prepargs(register ppfs_t *ppfs, va_list arg)
{
	int i;

	va_copy(ppfs->arg, arg);

#ifdef NL_ARGMAX
	i = ppfs->maxposarg; /* init for positional args */
	if (i > 0) {
		ppfs->num_data_args = i;
		ppfs->info.width = ppfs->info.prec = ppfs->maxposarg = 0;
		_ppfs_setargs(ppfs);
		ppfs->maxposarg = i;
	}
#endif
}
#endif
/**********************************************************************/
#ifdef L__ppfs_setargs

void attribute_hidden _ppfs_setargs(register ppfs_t *ppfs)
{
#ifdef __va_arg_ptr
	register void **p = ppfs->argptr;
#else
	register argvalue_t *p = ppfs->argvalue;
#endif
	int i;

#ifdef NL_ARGMAX
	if (ppfs->maxposarg == 0) {	/* initing for or no pos args */
#endif
		if (ppfs->info.width == INT_MIN) {
			ppfs->info.width =
#ifdef __va_arg_ptr
				*(int *)
#endif
				GET_VA_ARG(p,u,unsigned int,ppfs->arg);
		}
		if (ppfs->info.prec == INT_MIN) {
			ppfs->info.prec =
#ifdef __va_arg_ptr
				*(int *)
#endif
				GET_VA_ARG(p,u,unsigned int,ppfs->arg);
		}
		i = 0;
		while (i < ppfs->num_data_args) {
			switch(ppfs->argtype[i++]) {
				case (PA_INT|PA_FLAG_LONG_LONG):
#ifdef ULLONG_MAX
					GET_VA_ARG(p,ull,unsigned long long,ppfs->arg);
					break;
#endif
				case (PA_INT|PA_FLAG_LONG):
#if ULONG_MAX != UINT_MAX
					GET_VA_ARG(p,ul,unsigned long,ppfs->arg);
					break;
#endif
				case PA_CHAR:	/* TODO - be careful */
 					/* ... users could use above and really want below!! */
				case (PA_INT|__PA_FLAG_CHAR):/* TODO -- translate this!!! */
				case (PA_INT|PA_FLAG_SHORT):
				case PA_INT:
					GET_VA_ARG(p,u,unsigned int,ppfs->arg);
					break;
				case PA_WCHAR:	/* TODO -- assume int? */
					/* we're assuming wchar_t is at least an int */
					GET_VA_ARG(p,wc,wchar_t,ppfs->arg);
					break;
#ifdef __UCLIBC_HAS_FLOATS__
					/* PA_FLOAT */
				case PA_DOUBLE:
					GET_VA_ARG(p,d,double,ppfs->arg);
					break;
				case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
					GET_VA_ARG(p,ld,long double,ppfs->arg);
					break;
#else  /* __UCLIBC_HAS_FLOATS__ */
				case PA_DOUBLE:
				case (PA_DOUBLE|PA_FLAG_LONG_DOUBLE):
					assert(0);
					continue;
#endif /* __UCLIBC_HAS_FLOATS__ */
				default:
					/* TODO -- really need to ensure this can't happen */
					assert(ppfs->argtype[i-1] & PA_FLAG_PTR);
				case PA_POINTER:
				case PA_STRING:
				case PA_WSTRING:
					GET_VA_ARG(p,p,void *,ppfs->arg);
					break;
				case __PA_NOARG:
					continue;
			}
			++p;
		}
#ifdef NL_ARGMAX
	} else {
		if (ppfs->info.width == INT_MIN) {
			ppfs->info.width
				= (int) GET_ARG_VALUE(p + ppfs->argnumber[0] - 1,u,unsigned int);
		}
		if (ppfs->info.prec == INT_MIN) {
			ppfs->info.prec
				= (int) GET_ARG_VALUE(p + ppfs->argnumber[1] - 1,u,unsigned int);
		}
	}
#endif /* NL_ARGMAX */

	/* Now we know the width and precision. */
	if (ppfs->info.width < 0) {
		ppfs->info.width = -ppfs->info.width;
		PRINT_INFO_SET_FLAG(&(ppfs->info),left);
		PRINT_INFO_CLR_FLAG(&(ppfs->info),space);
		ppfs->info.pad = ' ';
	}
#if 0
	/* NOTE -- keep neg for now so float knows! */
	if (ppfs->info.prec < 0) {	/* spec says treat as omitted. */
		/* so use default prec... 1 for everything but floats and strings. */
		ppfs->info.prec = 1;
	}
#endif
}
#endif
/**********************************************************************/
#ifdef L__ppfs_parsespec

/* Notes: argtype differs from glibc for the following:
 *         mine              glibc
 *  lc     PA_WCHAR          PA_CHAR       the standard says %lc means %C
 *  ls     PA_WSTRING        PA_STRING     the standard says %ls means %S
 *  {*}n   {*}|PA_FLAG_PTR   PA_FLAG_PTR   size of n can be qualified
 */

/* TODO: store positions of positional args */

/* TODO -- WARNING -- assumes aligned on integer boundaries!!! */

/* TODO -- disable if not using positional args!!! */
#define _OVERLAPPING_DIFFERENT_ARGS

/* TODO -- rethink this -- perhaps we should set to largest type??? */

#ifdef _OVERLAPPING_DIFFERENT_ARGS

#define PROMOTED_SIZE_OF(X)		((sizeof(X) + sizeof(int) - 1) / sizeof(X))

static const short int type_codes[] = {
	__PA_NOARG,					/* must be first entry */
	PA_POINTER,
	PA_STRING,
	PA_WSTRING,
	PA_CHAR,
	PA_INT|PA_FLAG_SHORT,
	PA_INT,
	PA_INT|PA_FLAG_LONG,
	PA_INT|PA_FLAG_LONG_LONG,
	PA_WCHAR,
#ifdef __UCLIBC_HAS_FLOATS__
	/* PA_FLOAT, */
	PA_DOUBLE,
	PA_DOUBLE|PA_FLAG_LONG_DOUBLE,
#endif
};

static const unsigned char type_sizes[] = {
	/* unknown type consumes no arg */
	0,							/* must be first entry */
	PROMOTED_SIZE_OF(void *),
	PROMOTED_SIZE_OF(char *),
	PROMOTED_SIZE_OF(wchar_t *),
	PROMOTED_SIZE_OF(char),
	PROMOTED_SIZE_OF(short),
	PROMOTED_SIZE_OF(int),
	PROMOTED_SIZE_OF(long),
#ifdef ULLONG_MAX
	PROMOTED_SIZE_OF(long long),
#else
	PROMOTED_SIZE_OF(long),		/* TODO -- is this correct? (above too) */
#endif
	PROMOTED_SIZE_OF(wchar_t),
#ifdef __UCLIBC_HAS_FLOATS__
	/* PROMOTED_SIZE_OF(float), */
	PROMOTED_SIZE_OF(double),
	PROMOTED_SIZE_OF(long double),
#endif
};

static int _promoted_size(int argtype)
{
	register const short int *p;

	/* note -- since any unrecognized type is treated as a pointer */
	p = type_codes + sizeof(type_codes)/sizeof(type_codes[0]);
	do {
		if (*--p == argtype) {
			break;
		}
	} while (p > type_codes);

	return type_sizes[(int)(p - type_codes)];
}

static int _is_equal_or_bigger_arg(int curtype, int newtype)
{
	/* Quick test */
	if (newtype == __PA_NOARG) {
		return 0;
	}
	if ((curtype == __PA_NOARG) || (curtype == newtype)) {
		return 1;
	}
	/* Ok... slot is already filled and types are different in name. */
	/* So, compare promoted sizes of curtype and newtype args. */
	return _promoted_size(curtype) <= _promoted_size(newtype);
}

#else

#define _is_equal_or_bigger_arg(C,N)	(((C) == __PA_NOARG) || ((C) == (N)))

#endif

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
/* TODO - do this differently? */
static char _bss_custom_printf_spec[MAX_USER_SPEC]; /* 0-init'd for us.  */

attribute_hidden char *_custom_printf_spec = _bss_custom_printf_spec;
attribute_hidden printf_arginfo_function *_custom_printf_arginfo[MAX_USER_SPEC];
attribute_hidden printf_function _custom_printf_handler[MAX_USER_SPEC];
#endif /* __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__ */

int attribute_hidden _ppfs_parsespec(ppfs_t *ppfs)
{
	register const char *fmt;
	register const char *p;
	int preci;
	int width;
	int flags;
	int dataargtype;
	int i;
	int dpoint;
#ifdef NL_ARGMAX
	int maxposarg;
#endif
	int p_m_spec_chars;
	int n;
	int argtype[MAX_ARGS_PER_SPEC+2];
	int argnumber[3];			/* width, precision, 1st data arg */
	static const char spec_flags[] = SPEC_FLAGS;
	static const char spec_chars[] = SPEC_CHARS;/* TODO: b? */
	static const char spec_ranges[] = SPEC_RANGES;
	static const short spec_or_mask[] = SPEC_OR_MASK;
	static const short spec_and_mask[] = SPEC_AND_MASK;
	static const char qual_chars[] = QUAL_CHARS;
#ifdef __UCLIBC_HAS_WCHAR__
	char buf[32];
#endif

	/* WIDE note: we can test against '%' here since we don't allow */
	/* WIDE note: other mappings of '%' in the wide char set. */
	preci = -1;
	argnumber[0] = 0;
	argnumber[1] = 0;
	argtype[0] = __PA_NOARG;
	argtype[1] = __PA_NOARG;
#ifdef NL_ARGMAX
	maxposarg = ppfs->maxposarg;
#endif

#ifdef __UCLIBC_HAS_WCHAR__
	/* This is somewhat lame, but saves a lot of code.  If we're dealing with
	 * a wide stream, that means the format is a wchar string.  So, copy it
	 * char-by-char into a normal char buffer for processing.  Make the buffer
	 * (buf) big enough so that any reasonable format specifier will fit.
	 * While there a legal specifiers that won't, the all involve duplicate
	 * flags or outrageous field widths/precisions. */
	width = dpoint = 0;
	flags = ppfs->info._flags & FLAG_WIDESTREAM;
	if (flags == 0) {
		fmt = ppfs->fmtpos;
	} else {
		fmt = buf + 1;
		i = 0;
		do {
			buf[i] = (char) (((wchar_t *) ppfs->fmtpos)[i-1]);
			if (buf[i] != (((wchar_t *) ppfs->fmtpos)[i-1])) {
				return -1;
			}
		} while (buf[i++] && (i < sizeof(buf)));
		buf[sizeof(buf)-1] = 0;
	}
#else  /* __UCLIBC_HAS_WCHAR__ */
	width = flags = dpoint = 0;
	fmt = ppfs->fmtpos;
#endif

	assert(fmt[-1] == '%');
	assert(fmt[0] != '%');

	/* Process arg pos and/or flags and/or width and/or precision. */
 width_precision:
	p = fmt;
	if (*fmt == '*') {
		argtype[-dpoint] = PA_INT;
		++fmt;
	}
	i = 0;
	while (isdigit(*fmt)) {
		if (i < INT_MAX / 10
		    || (i == INT_MAX / 10 && (*fmt - '0') <= INT_MAX % 10)) {
			i = (i * 10) + (*fmt - '0');
		} else {
			i = INT_MAX; /* best we can do... */
		}
		++fmt;
	}
	if (p[-1] == '%') { /* Check for a position. */

		/* TODO: if val not in range, then error */

#ifdef NL_ARGMAX
		if ((*fmt == '$') && (i > 0)) {/* Positional spec. */
			++fmt;
			if (maxposarg == 0) {
				return -1;
			}
			argnumber[2] = i;
			if (argnumber[2] > maxposarg) {
				maxposarg = i;
			}
			/* Now fall through to check flags. */
		} else {
			if (maxposarg > 0) {
# ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
				if (*fmt == 'm') {
					goto PREC_WIDTH;
				}
# endif /* __UCLIBC_HAS_PRINTF_M_SPEC__ */
				return -1;
			}
			maxposarg = 0;		/* Possible redundant store, but cuts size. */

			if ((fmt > p) && (*p != '0')) {
				goto PREC_WIDTH;
			}

			fmt = p;			/* Back up for possible '0's flag. */
			/* Now fall through to check flags. */
		}
#else  /* NL_ARGMAX */
		if (*fmt == '$') {		/* Positional spec. */
			return -1;
		}

		if ((fmt > p) && (*p != '0')) {
			goto PREC_WIDTH;
		}

		fmt = p;			/* Back up for possible '0's flag. */
		/* Now fall through to check flags. */
#endif /* NL_ARGMAX */

	restart_flags:		/* Process flags. */
		i = 1;
		p = spec_flags;

		do {
			if (*fmt == *p++) {
				++fmt;
				flags |= i;
				goto restart_flags;
			}
			i += i;				/* Better than i <<= 1 for bcc */
		} while (*p);
		i = 0;

		/* If '+' then ignore ' ', and if '-' then ignore '0'. */
		/* Note: Need to ignore '0' when prec is an arg with val < 0, */
		/*       but that test needs to wait until the arg is retrieved. */
		flags &= ~((flags & (FLAG_PLUS|FLAG_MINUS)) >> 1);
		/* Note: Ignore '0' when prec is specified < 0 too (in printf). */

		if (fmt[-1] != '%') {	/* If we've done anything, loop for width. */
			goto width_precision;
		}
	}
 PREC_WIDTH:
	if (*p == '*') {			/* Prec or width takes an arg. */
#ifdef NL_ARGMAX
		if (maxposarg) {
			if ((*fmt++ != '$') || (i <= 0)) {
				/* Using pos args and no $ or invalid arg number. */
				return -1;
			}
			argnumber[-dpoint] = i;
		} else
#endif
		if (++p != fmt) {
			 /* Not using pos args but digits followed *. */
			return -1;
		}
		i = INT_MIN;
	}

	if (!dpoint) {
		width = i;
		if (*fmt == '.') {
			++fmt;
			dpoint = -1;		/* To use as default precison. */
			goto width_precision;
		}
	} else {
		preci = i;
	}

	/* Process qualifier. */
	p = qual_chars;
	do {
		if (*fmt == *p) {
			++fmt;
			break;
		}
	} while (*++p);
	if ((p - qual_chars < 2) && (*fmt == *p)) {
		p += ((sizeof(qual_chars)-2) / 2);
		++fmt;
	}
	dataargtype = ((int)(p[(sizeof(qual_chars)-2) / 2])) << 8;

	/* Process conversion specifier. */
	if (!*fmt) {
		return -1;
	}

	p = spec_chars;

	do {
		if (*fmt == *p) {
			p_m_spec_chars = p - spec_chars;

			if ((p_m_spec_chars >= CONV_c)
				&& (dataargtype & PA_FLAG_LONG)) {
				p_m_spec_chars -= 2; /* lc -> C and ls -> S */
			}

			ppfs->conv_num = p_m_spec_chars;
			p = spec_ranges-1;
			while (p_m_spec_chars > *++p) {}

			i = p - spec_ranges;
			argtype[2] = (dataargtype | spec_or_mask[i]) & spec_and_mask[i];
			p = spec_chars;
			break;
		}
	} while(*++p);

	ppfs->info.spec = *fmt;
	ppfs->info.prec = preci;
	ppfs->info.width = width;
	ppfs->info.pad = ((flags & FLAG_ZERO) ? '0' : ' ');
	ppfs->info._flags = (flags & ~FLAG_ZERO) | (dataargtype & __PA_INTMASK);
	ppfs->num_data_args = 1;

	if (!*p) {
#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
		if (*fmt == 'm') {
			ppfs->conv_num = CONV_m;
			ppfs->num_data_args = 0;
		} else
#endif
		{
#ifndef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
			return -1;  /* Error */
#else
			/* Handle custom arg -- WARNING -- overwrites p!!! */
			ppfs->conv_num = CONV_custom0;
			p = _custom_printf_spec;
			while (1) {
				if (*p == *fmt) {
					printf_arginfo_function *fp = _custom_printf_arginfo[(int)(p - _custom_printf_spec)];
					ppfs->num_data_args = fp(&(ppfs->info), MAX_ARGS_PER_SPEC, argtype + 2);
					if (ppfs->num_data_args > MAX_ARGS_PER_SPEC) {
						return -1;  /* Error -- too many args! */
					}
					break;
				}
				if (++p >= (_custom_printf_spec + MAX_USER_SPEC))
					return -1;  /* Error */
			}
#endif
		}
	}

#ifdef NL_ARGMAX
	if (maxposarg > 0) {
		i = 0;
		do {
			/* Update maxposarg and check that NL_ARGMAX is not exceeded. */
			n = ((i <= 2)
				 ? (ppfs->argnumber[i] = argnumber[i])
				 : argnumber[2] + (i-2));
			if (n > maxposarg) {
				maxposarg = n;
				if (maxposarg > NL_ARGMAX) {
					return -1;
				}
			}
			--n;
			/* Record argtype with largest size (current, new). */
			if (_is_equal_or_bigger_arg(ppfs->argtype[n], argtype[i])) {
				ppfs->argtype[n] = argtype[i];
			}
		} while (++i < ppfs->num_data_args + 2);
	} else
#endif /* NL_ARGMAX */
	{
		ppfs->argnumber[2] = 1;
		memcpy(ppfs->argtype, argtype + 2, ppfs->num_data_args * sizeof(int));
	}

#ifdef NL_ARGMAX
	ppfs->maxposarg = maxposarg;
#endif

#ifdef __UCLIBC_HAS_WCHAR__
	flags = ppfs->info._flags & FLAG_WIDESTREAM;
	if (flags == 0) {
		ppfs->fmtpos = ++fmt;
	} else {
		ppfs->fmtpos = (const char *) (((const wchar_t *)(ppfs->fmtpos))
									   + (fmt - buf) );
	}
#else  /* __UCLIBC_HAS_WCHAR__ */
	ppfs->fmtpos = ++fmt;
#endif

 	return ppfs->num_data_args + 2;
}

#endif
/**********************************************************************/
#ifdef L_register_printf_function

#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__

int register_printf_function(int spec, printf_function handler,
							 printf_arginfo_function arginfo)
{
	register char *r;
	register char *p;

	if (spec && (arginfo != NULL)) { /* TODO -- check if spec is valid char */
		r = NULL;
		p = _custom_printf_spec + MAX_USER_SPEC;
		do {
			--p;
			if (!*p) {
				r = p;
			}
#ifdef __BCC__
			else				/* bcc generates less code with fall-through */
#endif
			if (*p == spec) {
				r = p;
				p = _custom_printf_spec;
			}
		} while (p > _custom_printf_spec);

		if (r) {
			if (handler) {
				*r = spec;
				_custom_printf_handler[(int)(r - p)] = handler;
				_custom_printf_arginfo[(int)(r - p)] = arginfo;
			} else {
				*r = 0;
			}
			return 0;
		}
		/* TODO -- if asked to unregister a non-existent spec, return what? */
	}
	return -1;
}

#endif

#endif
/**********************************************************************/
#if defined(L__vfprintf_internal) || defined(L__vfwprintf_internal)

/* We only support ascii digits (or their USC equivalent codes) in
 * precision and width settings in *printf (wide) format strings.
 * In other words, we don't currently support glibc's 'I' flag.
 * We do accept it, but it is currently ignored. */

static size_t _charpad(FILE * __restrict stream, int padchar, size_t numpad);

#ifdef L__vfprintf_internal

#define VFPRINTF_internal _vfprintf_internal
#define FMT_TYPE char
#define OUTNSTR _outnstr
#define STRLEN  strlen
#define _PPFS_init _ppfs_init
#define OUTPUT(F,S)			fputs_unlocked(S,F)
/* #define _outnstr(stream, string, len)	__stdio_fwrite(string, len, stream) */
#define _outnstr(stream, string, len)	((len > 0) ? __stdio_fwrite((const unsigned char *)(string), len, stream) : 0)
#define FP_OUT _fp_out_narrow

#ifdef __UCLIBC_HAS_FLOATS__

static size_t _fp_out_narrow(FILE *fp, intptr_t type, intptr_t len, intptr_t buf)
{
	size_t r = 0;

	if (type & 0x80) {			/* Some type of padding needed. */
		int buflen = strlen((const char *) buf);
		len -= buflen;
		if (len > 0) {
			r = _charpad(fp, (type & 0x7f), len);
			if (r != len) {
				return r;
			}
		}
		len = buflen;
	}
	return r + OUTNSTR(fp, (const char *) buf, len);
}

#endif /* __UCLIBC_HAS_FLOATS__ */

#else  /* L__vfprintf_internal */

#define VFPRINTF_internal _vfwprintf_internal
#define FMT_TYPE wchar_t
#define OUTNSTR _outnwcs
#define STRLEN  wcslen
#define _PPFS_init _ppwfs_init
/* Pulls in fseek: */
#define OUTPUT(F,S)			fputws_unlocked(S,F)
/* TODO: #define OUTPUT(F,S)		_wstdio_fwrite((S),wcslen(S),(F)) */
#define _outnwcs(stream, wstring, len)	_wstdio_fwrite((const wchar_t *)(wstring), len, stream)
#define FP_OUT _fp_out_wide

static size_t _outnstr(FILE *stream, const char *s, size_t wclen)
{
	/* NOTE!!! len here is the number of wchars we want to generate!!! */
	wchar_t wbuf[64];
	mbstate_t mbstate;
	size_t todo, r, n;

	mbstate.__mask = 0;
	todo = wclen;

	while (todo) {
		r = mbsrtowcs(wbuf, &s,
					  ((todo <= sizeof(wbuf)/sizeof(wbuf[0]))
					   ? todo
					   : sizeof(wbuf)/sizeof(wbuf[0])),
					  &mbstate);
		assert(((ssize_t)r) > 0);
		n = _outnwcs(stream, wbuf, r);
		todo -= n;
		if (n != r) {
			break;
		}
	}

	return wclen - todo;
}

#ifdef __UCLIBC_HAS_FLOATS__

static size_t _fp_out_wide(FILE *fp, intptr_t type, intptr_t len, intptr_t buf)
{
	wchar_t wbuf[BUF_SIZE];
	const char *s = (const char *) buf;
	size_t r = 0;
	int i;

	if (type & 0x80) {			/* Some type of padding needed */
		int buflen = strlen(s);
		len -= buflen;
		if (len > 0) {
			r = _charpad(fp, (type & 0x7f), len);
			if (r != len) {
				return r;
			}
		}
		len = buflen;
	}

	if (len > 0) {
		i = 0;
		do {
#ifdef __LOCALE_C_ONLY
			wbuf[i] = s[i];
#else

# ifdef __UCLIBC_HAS_GLIBC_DIGIT_GROUPING__
			if (s[i] == ',') {
				wbuf[i] = __UCLIBC_CURLOCALE->thousands_sep_wc;
			} else
# endif
			if (s[i] == '.') {
				wbuf[i] = __UCLIBC_CURLOCALE->decimal_point_wc;
			} else {
				wbuf[i] = s[i];
			}
#endif /* __LOCALE_C_ONLY */

		} while (++i < len);

		r += OUTNSTR(fp, wbuf, len);
	}

	return r;
}

#endif /* __UCLIBC_HAS_FLOATS__ */

static int _ppwfs_init(register ppfs_t *ppfs, const wchar_t *fmt0)
{
	static const wchar_t invalid_wcs[] = L"Invalid wide format string.";
	int r;

	/* First, zero out everything... argnumber[], argtype[], argptr[] */
	memset(ppfs, 0, sizeof(ppfs_t)); /* TODO: nonportable???? */
#ifdef NL_ARGMAX
	--ppfs->maxposarg;			/* set to -1 */
#endif
	ppfs->fmtpos = (const char *) fmt0;
	ppfs->info._flags = FLAG_WIDESTREAM;

	{
		mbstate_t mbstate;
		const wchar_t *p;
		mbstate.__mask = 0;	/* Initialize the mbstate. */
		p = fmt0;
		if (wcsrtombs(NULL, &p, SIZE_MAX, &mbstate) == ((size_t)(-1))) {
			ppfs->fmtpos = (const char *) invalid_wcs;
			return -1;
		}
	}

	/* now set all argtypes to no-arg */
	{
#if 1
		/* TODO - use memset here since already "paid for"? */
		register int *p = ppfs->argtype;

		r = MAX_ARGS;
		do {
			*p++ = __PA_NOARG;
		} while (--r);
#else
		/* TODO -- get rid of this?? */
		register char *p = (char *) ((MAX_ARGS-1) * sizeof(int));

		do {
			*((int *)(((char *)ppfs) + ((int)p) + offsetof(ppfs_t,argtype))) = __PA_NOARG;
			p -= sizeof(int);
		} while (p);
#endif
	}

	/*
	 * Run through the entire format string to validate it and initialize
	 * the positional arg numbers (if any).
	 */
	{
		register const wchar_t *fmt = fmt0;

		while (*fmt) {
			if ((*fmt == '%') && (*++fmt != '%')) {
				ppfs->fmtpos = (const char *) fmt; /* back up to the '%' */
				r = _ppfs_parsespec(ppfs);
				if (r < 0) {
					return -1;
				}
				fmt = (const wchar_t *) ppfs->fmtpos; /* update to one past end of spec */
			} else {
				++fmt;
			}
		}
		ppfs->fmtpos = (const char *) fmt0; /* rewind */
	}

#ifdef NL_ARGMAX
	/* If we have positional args, make sure we know all the types. */
	{
		register int *p = ppfs->argtype;
		r = ppfs->maxposarg;
		while (--r >= 0) {
			if ( *p == __PA_NOARG ) { /* missing arg type!!! */
				return -1;
			}
			++p;
		}
	}
#endif /* NL_ARGMAX */

	return 0;
}

#endif /* L__vfprintf_internal */


static size_t _charpad(FILE * __restrict stream, int padchar, size_t numpad)
{
	size_t todo = numpad;

	/* TODO -- Use a buffer to cut down on function calls... */
	FMT_TYPE pad[1];

	*pad = padchar;
	while (todo && (OUTNSTR(stream, (const char *) pad, 1) == 1)) {
		--todo;
	}

	return numpad - todo;
}

/* TODO -- Dynamically allocate work space to accomodate stack-poor archs? */
static int _do_one_spec(FILE * __restrict stream,
						 register ppfs_t *ppfs, int *count)
{
	static const char spec_base[] = SPEC_BASE;
#ifdef L__vfprintf_internal
	static const char prefix[] = "+\0-\0 \0000x\0000X";
	/*                            0  2  4    6     9 11*/
#else
	static const wchar_t prefix[] = L"+\0-\0 \0000x\0000X";
#endif
	enum {
		PREFIX_PLUS = 0,
		PREFIX_MINUS = 2,
		PREFIX_SPACE = 4,
		PREFIX_LWR_X = 6,
		PREFIX_UPR_X = 9,
		PREFIX_NONE = 11
	};

#ifdef __va_arg_ptr
	const void * const *argptr;
#else
	const void * argptr[MAX_ARGS_PER_SPEC];
#endif
	int *argtype;
#ifdef __UCLIBC_HAS_WCHAR__
	const wchar_t *ws = NULL;
	mbstate_t mbstate;
#endif
	size_t slen;
#ifdef L__vfprintf_internal
#define SLEN slen
#else
	size_t SLEN;
	wchar_t wbuf[2];
#endif
	int base;
	int numpad;
	int alphacase;
	int numfill = 0;			/* TODO: fix */
	int prefix_num = PREFIX_NONE;
	char padchar = ' ';
	/* TODO: buf needs to be big enough for any possible error return strings
	 * and also for any locale-grouped long long integer strings generated.
	 * This should be large enough for any of the current archs/locales, but
	 * eventually this should be handled robustly. */
	char buf[128];

#ifdef NDEBUG
	_ppfs_parsespec(ppfs);
#else
	if (_ppfs_parsespec(ppfs) < 0) { /* TODO: just for debugging */
		abort();
	}
#endif
	_ppfs_setargs(ppfs);

	argtype = ppfs->argtype + ppfs->argnumber[2] - 1;
	/* Deal with the argptr vs argvalue issue. */
#ifdef __va_arg_ptr
	argptr = (const void * const *) ppfs->argptr;
# ifdef NL_ARGMAX
	if (ppfs->maxposarg > 0) {	/* Using positional args... */
		argptr += ppfs->argnumber[2] - 1;
	}
# endif
#else
	/* Need to build a local copy... */
	{
		register argvalue_t *p = ppfs->argvalue;
		int i;
# ifdef NL_ARGMAX
		if (ppfs->maxposarg > 0) {	/* Using positional args... */
			p += ppfs->argnumber[2] - 1;
		}
# endif
		for (i = 0 ; i < ppfs->num_data_args ; i++ ) {
			argptr[i] = (void *) p++;
		}
	}
#endif
	{
		register char *s = NULL; /* TODO: Should s be unsigned char * ? */

		if (ppfs->conv_num == CONV_n) {
			_store_inttype(*(void **)*argptr,
						   ppfs->info._flags & __PA_INTMASK,
						   (intmax_t) (*count));
			return 0;
		}
		if (ppfs->conv_num <= CONV_i) {	/* pointer or (un)signed int */
			alphacase = __UIM_LOWER;

			base = spec_base[(int)(ppfs->conv_num - CONV_p)];
			if (base == 10) {
				if (PRINT_INFO_FLAG_VAL(&(ppfs->info),group)) {
					alphacase = __UIM_GROUP;
				}
				if (PRINT_INFO_FLAG_VAL(&(ppfs->info),i18n)) {
					alphacase |= 0x80;
				}
			}

			if (ppfs->conv_num <= CONV_u) { /* pointer or unsigned int */
				if (ppfs->conv_num == CONV_X) {
					alphacase = __UIM_UPPER;
				}
				if (ppfs->conv_num == CONV_p) { /* pointer */
					prefix_num = PREFIX_LWR_X;
				} else {		/* unsigned int */
				}
			} else {			/* signed int */
				base = -base;
			}
			if (ppfs->info.prec < 0) { /* Ignore '0' flag if prec specified. */
				padchar = ppfs->info.pad;
			}
			s = _uintmaxtostr(buf + sizeof(buf) - 1,
							  (uintmax_t)
							  _load_inttype(ppfs->conv_num == CONV_p ? PA_FLAG_LONG : *argtype & __PA_INTMASK,
											*argptr, base), base, alphacase);
			if (ppfs->conv_num > CONV_u) { /* signed int */
				if (*s == '-') {
					PRINT_INFO_SET_FLAG(&(ppfs->info),showsign);
					++s;		/* handle '-' in the prefix string */
					prefix_num = PREFIX_MINUS;
				} else if (PRINT_INFO_FLAG_VAL(&(ppfs->info),showsign)) {
					prefix_num = PREFIX_PLUS;
				} else if (PRINT_INFO_FLAG_VAL(&(ppfs->info),space)) {
					prefix_num = PREFIX_SPACE;
				}
			}
			slen = (char *)(buf + sizeof(buf) - 1) - s;
#ifdef L__vfwprintf_internal
			{
				const char *q = s;
				mbstate.__mask = 0; /* Initialize the mbstate. */
				SLEN = mbsrtowcs(NULL, &q, 0, &mbstate);
			}
#endif
			numfill = ((ppfs->info.prec < 0) ? 1 : ppfs->info.prec);
			if (PRINT_INFO_FLAG_VAL(&(ppfs->info),alt)) {
				if (ppfs->conv_num <= CONV_x) {	/* x or p */
					prefix_num = PREFIX_LWR_X;
				}
				if (ppfs->conv_num == CONV_X) {
					prefix_num = PREFIX_UPR_X;
				}
				if ((ppfs->conv_num == CONV_o) && (numfill <= SLEN)) {
					numfill = ((*s == '0') ? 1 : SLEN + 1);
				}
			}
			if (*s == '0') {
				if (prefix_num >= PREFIX_LWR_X) {
					prefix_num = PREFIX_NONE;
				}
				if (ppfs->conv_num == CONV_p) {/* null pointer */
					s = "(nil)";
#ifdef L__vfwprintf_internal
					SLEN =
#endif
					slen = 5;
					numfill = 0;
				} else if (numfill == 0) {	/* if precision 0, no output */
#ifdef L__vfwprintf_internal
					SLEN =
#endif
					slen = 0;
				}
			}
			numfill = ((numfill > SLEN) ? numfill - SLEN : 0);
		} else if (ppfs->conv_num <= CONV_A) {	/* floating point */
#ifdef __UCLIBC_HAS_FLOATS__
			ssize_t nf;
			nf = _fpmaxtostr(stream,
							 (__fpmax_t)
							 (PRINT_INFO_FLAG_VAL(&(ppfs->info),is_long_double)
							  ? *(long double *) *argptr
							  : (long double) (* (double *) *argptr)),
							 &ppfs->info, FP_OUT );
			if (nf < 0) {
				return -1;
			}
			*count += nf;

			return 0;
#else  /* __UCLIBC_HAS_FLOATS__ */
			return -1;			/* TODO -- try to continue? */
#endif
		} else if (ppfs->conv_num <= CONV_S) {	/* wide char or string */
#ifdef L__vfprintf_internal

#ifdef __UCLIBC_HAS_WCHAR__
			mbstate.__mask = 0;	/* Initialize the mbstate. */
			if (ppfs->conv_num == CONV_S) { /* wide string */
				ws = *((const wchar_t **) *argptr);
				if (!ws) {
					goto NULL_STRING;
				}
				/* We use an awful uClibc-specific hack here, passing
				 * (char*) &ws as the conversion destination.  This signals
				 * uClibc's wcsrtombs that we want a "restricted" length
				 * such that the mbs fits in a buffer of the specified
				 * size with no partial conversions. */
				slen = wcsrtombs((char *) &ws, &ws, /* Use awful hack! */
							((ppfs->info.prec >= 0)
							 ? ppfs->info.prec
							 : SIZE_MAX),
							&mbstate);
				if (slen == ((size_t)-1)) {
					return -1;	/* EILSEQ */
				}
			} else {			/* wide char */
				s = buf;
				slen = wcrtomb(s, (*((const wchar_t *) *argptr)), &mbstate);
				if (slen == ((size_t)-1)) {
					return -1;	/* EILSEQ */
				}
				s[slen] = 0;	/* TODO - Is this necessary? */
			}
#else  /* __UCLIBC_HAS_WCHAR__ */
			return -1;
#endif
		} else if (ppfs->conv_num <= CONV_s) {	/* char or string */
			if (ppfs->conv_num == CONV_s) { /* string */
				s = *((char **) (*argptr));
				if (s) {
#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
				SET_STRING_LEN:
#endif
					slen = strnlen(s, ((ppfs->info.prec >= 0)
									   ? ppfs->info.prec : SIZE_MAX));
				} else {
#ifdef __UCLIBC_HAS_WCHAR__
				NULL_STRING:
#endif
					s = "(null)";
					slen = 6;
					/* Use an empty string rather than truncation if precision is too small. */
					if (ppfs->info.prec >= 0 && ppfs->info.prec < slen)
						slen = 0;
				}
			} else {			/* char */
				s = buf;
				*s = (unsigned char)(*((const int *) *argptr));
				s[1] = 0;
				slen = 1;
			}

#else  /* L__vfprintf_internal */

			if (ppfs->conv_num == CONV_S) { /* wide string */
				ws = *((wchar_t **) (*argptr));
				if (!ws) {
					goto NULL_STRING;
				}
				SLEN = wcsnlen(ws, ((ppfs->info.prec >= 0)
									? ppfs->info.prec : SIZE_MAX));
			} else {			/* wide char */
				*wbuf = (wchar_t)(*((const wint_t *) *argptr));
			CHAR_CASE:
				ws = wbuf;
				wbuf[1] = 0;
				SLEN = 1;
			}

		} else if (ppfs->conv_num <= CONV_s) {	/* char or string */

			if (ppfs->conv_num == CONV_s) { /* string */
				s = *((char **) (*argptr));
				if (s) {
#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
				SET_STRING_LEN:
#endif
					/* We use an awful uClibc-specific hack here, passing
					 * (wchar_t*) &mbstate as the conversion destination.
					 *  This signals uClibc's mbsrtowcs that we want a
					 * "restricted" length such that the mbs fits in a buffer
					 * of the specified size with no partial conversions. */
					{
						const char *q = s;
						mbstate.__mask = 0;	/* Initialize the mbstate. */
						SLEN = mbsrtowcs((wchar_t *) &mbstate, &q,
										 ((ppfs->info.prec >= 0)
										  ? ppfs->info.prec : SIZE_MAX),
										 &mbstate);
					}
					if (SLEN == ((size_t)(-1))) {
						return -1;	/* EILSEQ */
					}
				} else {
				NULL_STRING:
					s = "(null)";
					SLEN = slen = 6;
					/* Use an empty string rather than truncation if precision is too small. */
					if (ppfs->info.prec >= 0 && ppfs->info.prec < slen)
						SLEN = slen = 0;
				}
			} else {			/* char */
				*wbuf = btowc( (unsigned char)(*((const int *) *argptr)) );
				goto CHAR_CASE;
			}

#endif /* L__vfprintf_internal */

#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
		} else if (ppfs->conv_num == CONV_m) {
			s = __glibc_strerror_r(errno, buf, sizeof(buf));
			goto SET_STRING_LEN;
#endif
		} else {
#ifdef __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__
			assert(ppfs->conv_num == CONV_custom0);

			s = _custom_printf_spec;
			do {
				if (*s == ppfs->info.spec) {
					int rv;
					/* TODO -- check return value for sanity? */
					rv = (*_custom_printf_handler
						  [(int)(s-_custom_printf_spec)])
						(stream, &ppfs->info, argptr);
					if (rv < 0) {
						return -1;
					}
					*count += rv;
					return 0;
				}
			} while (++s < (_custom_printf_spec + MAX_USER_SPEC));
#endif /* __UCLIBC_HAS_GLIBC_CUSTOM_PRINTF__ */
			assert(0);
			return -1;
		}

		{
			size_t t;

			t = SLEN + numfill;
			if (prefix_num != PREFIX_NONE) {
				t += ((prefix_num < PREFIX_LWR_X) ? 1 : 2);
			}
			numpad = ((ppfs->info.width > t) ? (ppfs->info.width - t) : 0);
			*count += t + numpad;
		}
		if (padchar == '0') { /* TODO: check this */
			numfill += numpad;
			numpad = 0;
		}

		/* Now handle the output itself. */
		if (!PRINT_INFO_FLAG_VAL(&(ppfs->info),left)) {
			if (_charpad(stream, ' ', numpad) != numpad) {
				return -1;
			}
			numpad = 0;
		}
		OUTPUT(stream, prefix + prefix_num);

		if (_charpad(stream, '0', numfill) != numfill) {
			return -1;
		}

#ifdef L__vfprintf_internal

# ifdef __UCLIBC_HAS_WCHAR__
		if (!ws) {
			assert(s);
			if (_outnstr(stream, s, slen) != slen) {
				return -1;
			}
		} else {				/* wide string */
			size_t t;
			mbstate.__mask = 0;	/* Initialize the mbstate. */
			while (slen) {
				t = (slen <= sizeof(buf)) ? slen : sizeof(buf);
				t = wcsrtombs(buf, &ws, t, &mbstate);
				assert(t != ((size_t)(-1)));
				if (_outnstr(stream, buf, t) != t) {
					return -1;
				}
				slen -= t;
			}
		}
# else  /* __UCLIBC_HAS_WCHAR__ */
		if (_outnstr(stream, (const unsigned char *) s, slen) != slen) {
			return -1;
		}
# endif

#else  /* L__vfprintf_internal */

		if (!ws) {
			assert(s);
			if (_outnstr(stream, s, SLEN) != SLEN) {
				return -1;
			}
		} else {
			if (_outnwcs(stream, ws, SLEN) != SLEN) {
				return -1;
			}
		}

#endif /* L__vfprintf_internal */
		if (_charpad(stream, ' ', numpad) != numpad) {
			return -1;
		}
	}

	return 0;
}


int VFPRINTF_internal (FILE * __restrict stream,
			  const FMT_TYPE * __restrict format,
			  va_list arg)
{
	ppfs_t ppfs;
	int count, r;
	register const FMT_TYPE *s;

	count = 0;
	s = format;

	if (_PPFS_init(&ppfs, format) < 0) {	/* Bad format string. */
		OUTNSTR(stream, (const char *) ppfs.fmtpos,
				STRLEN((const FMT_TYPE *)(ppfs.fmtpos)));
#if defined(L__vfprintf_internal) && !defined(NDEBUG)
		fprintf(stderr,"\nIMbS: \"%s\"\n\n", format);
#endif
		count = -1;
	} else {
		_ppfs_prepargs(&ppfs, arg);	/* This did a va_copy!!! */

		do {
			while (*format && (*format != '%')) {
				++format;
			}

			if (format - s) {	/* output any literal text in format string */
				r = OUTNSTR(stream, (const char *) s, format - s);
				if (r != (format - s)) {
					count = -1;
					break;
				}
				count += r;
			}

			if (!*format) {			/* we're done */
				break;
			}

			if (format[1] != '%') {	/* if we get here, *format == '%' */
				/* TODO: _do_one_spec needs to know what the output funcs are!!! */
				ppfs.fmtpos = (const char *)(++format);
				/* TODO: check -- should only fail on stream error */
				r = _do_one_spec(stream, &ppfs, &count);
				if (r < 0) {
					count = -1;
					break;
				}
				s = format = (const FMT_TYPE *) ppfs.fmtpos;
			} else {			/* %% means literal %, so start new string */
				s = ++format;
				++format;
			}
		} while (1);

		va_end(ppfs.arg);		/* Need to clean up after va_copy! */
	}

/* #if defined(L__vfprintf_internal) && defined(__UCLIBC_HAS_WCHAR__) */
/*  DONE: */
/* #endif */

	return count;
}
#endif /* defined(L__vfprintf_internal) || defined(L__vfwprintf_internal) */


/**********************************************************************/
#if defined(L_vfprintf) || defined(L_vfwprintf)

/* This is just a wrapper around VFPRINTF_internal.
 * Factoring out vfprintf internals allows:
 * (1) vdprintf and vsnprintf don't need to setup fake locking,
 * (2) __STDIO_STREAM_TRANS_TO_WRITE is not used in vfprintf internals,
 * and thus fseek etc is not pulled in by vdprintf and vsnprintf.
 *
 * In order to not pull in fseek through fputs, OUTPUT() macro
 * is using __stdio_fwrite (TODO: do the same for wide functions).
 */
#ifdef L_vfprintf
# define VFPRINTF vfprintf
# define VFPRINTF_internal _vfprintf_internal
# define FMT_TYPE char
#else
# define VFPRINTF vfwprintf
# define VFPRINTF_internal _vfwprintf_internal
# define FMT_TYPE wchar_t
#endif

libc_hidden_proto(VFPRINTF)
int VFPRINTF (FILE * __restrict stream,
			  const FMT_TYPE * __restrict format,
			  va_list arg)
{
	int count;
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(stream);

	if
#ifdef L_vfprintf
	(!__STDIO_STREAM_IS_NARROW_WRITING(stream)
	 && __STDIO_STREAM_TRANS_TO_WRITE(stream, __FLAG_NARROW))
#else
	(!__STDIO_STREAM_IS_WIDE_WRITING(stream)
	 && __STDIO_STREAM_TRANS_TO_WRITE(stream, __FLAG_WIDE))
#endif
	{
		count = -1;
	} else {
		count = VFPRINTF_internal(stream, format, arg);
	}

	__STDIO_AUTO_THREADUNLOCK(stream);

	return count;
}
libc_hidden_def(VFPRINTF)
#endif /* defined(L_vfprintf) || defined(L_vfwprintf) */

/**********************************************************************/
