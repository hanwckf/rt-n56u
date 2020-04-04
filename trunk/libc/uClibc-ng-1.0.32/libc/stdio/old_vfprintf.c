/*
 * This file based on printf.c from 'Dlibs' on the atari ST  (RdeBath)
 *
 *
 *    Dale Schumacher                         399 Beacon Ave.
 *    (alias: Dalnefre')                      St. Paul, MN  55104
 *    dal@syntel.UUCP                         United States of America
 *  "It's not reality that's important, but how you perceive things."
 */

/* Altered to use stdarg, made the core function vfnprintf.
 * Hooked into the stdio package using 'inside information'
 * Altered sizeof() assumptions, now assumes all integers except chars
 * will be either
 *  sizeof(xxx) == sizeof(long) or sizeof(xxx) == sizeof(short)
 *
 * -RDB
 */

/*
 *                    Manuel Novoa III   Dec 2000
 *
 * The previous vfnprintf routine was almost completely rewritten with the
 * goal of fixing some shortcomings and reducing object size.
 *
 * The summary of changes:
 *
 * Converted print conversion specification parsing from one big switch
 *   to a method using string tables.  This new method verifies that the
 *   conversion flags, field width, precision, qualifier, and specifier
 *   appear in the correct order.  Many questionable specifications were
 *   accepted by the previous code.  This new method also resulted in a
 *   substantial reduction in object size of about 330 bytes (20%) from
 *   the old version (1627 bytes) on i386, even with the following
 *   improvements.
 *
 *     Implemented %n specifier as required by the standards.
 *     Implemented proper handling of precision for int types.
 *     Implemented # for hex and pointer, fixed error for octal rep of 0.
 *     Implemented return of -1 on stream error.
 *
 * Added optional support for the GNU extension %m which prints the string
 *   corresponding the errno.
 *
 * Added optional support for long long ints and unsigned long long ints
 *   using the conversion qualifiers "ll", "L", or "q" (like glibc).
 *
 * Added optional support for doubles in a very limited form.  None of
 *   the formating options are obeyed.  The string returned by __dtostr
 *   is printed directly.
 *
 * Converted to use my (un)signed long (long) to string routines, which are
 * smaller than the previous functions and don't require static buffers.
 *
 * Other Modifications:
 *   Modified sprintf, snprintf, vsprintf, vsnprintf to share on fake-file.
 */

/*
 *                    Manuel Novoa III   Jan 2001
 *
 * Removed fake file from *s*printf functions because of possible problems
 *    if called recursively.  Instead, have sprintf, snprintf, and vsprintf
 *    call vsnprintf which allocates a fake file on the stack.
 * Removed WANT_FPUTC option.  Always use standard putc macro to avoid
 *    problems with the fake file used by the *s*printf functions.
 * Fixed bug parsing flags -- did not restart scan.
 * Added function asprintf.
 * Fixed 0-pad prefixing bug.
 * Converted sizeof(int) == sizeof(long) tests to compile time vs run time.
 *    This saves 112 bytes of code on i386.
 * Fixed precision bug -- when negative set to default.
 * Added function fnprintf to support __dtostr.
 * Added floating point support for doubles.  Yeah!
 *
 *
 * May 2001     Fixes from Johan Adolfsson (johan.adolfsson@axis.com)
 *    1) printf("%c",0) returned 0 instead of 1.
 *    2) unrolled loop in asprintf to reduce size and remove compile warning.
 *
 *
 * June 2001
 *    1) fix %p so that "0x" is prepended to outputed hex val
 *    2) fix %p so that "(nil)" is output for (void *)0 to match glibc
 *
 * Sep 5, 2003
 *    Convert to new floating point conversion routine.
 *    Fix qualifier handling on integer and %n conversions.
 *    Add support for vsnprintf when in non-buffered/no-wchar configuration.
 *
 */

/*****************************************************************************/
/*                            OPTIONS                                        */
/*****************************************************************************/
/* The optional support for long longs and doubles comes in two forms.
 *
 *   1) Normal (or partial for doubles) output support.  Set to 1 to turn on.
 *      Adds about 130 bytes for doubles, about 220 bytes for long longs,
 *      and about 275 for both to the base code size of 1163 on i386.
 */

/* These are now set in uClibc_config.h based on Config. */
/*
#define __UCLIBC_HAS_FLOATS__            1
*/

/*   2) An error message is inserted into the stream, an arg of the
 *      appropriate size is removed from the arglist, and processing
 *      continues.  This is adds less code and may be useful in some
 *      cases.  Set to 1 to turn on.  Adds about 50 bytes for doubles,
 *      about 140 bytes for long longs, and about 175 bytes for both
 *      to the base code size of 1163 on i386.
 */

#define WANT_FLOAT_ERROR      0

/*
 * Set to support GNU extension of %m to print string corresponding to errno.
 *
 * Warning: This adds about 50 bytes (i386) to the code but it also pulls in
 * strerror and the corresponding string table which together are about 3.8k.
 */

/* Now controlled by uClibc_stdio.h and set below. */
/* #define WANT_GNU_ERRNO         0 */

/**************************************************************************/

#include "_stdio.h"
#include <stdarg.h>
#include <limits.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <bits/uClibc_uintmaxtostr.h>

#include "_fpmaxtostr.h"

/*  #undef WANT_FLOAT_ERROR */
/*  #define WANT_FLOAT_ERROR      1 */

/*  #define __isdigit(c) (((unsigned int)(c - '0')) < 10) */

#ifdef __UCLIBC_HAS_PRINTF_M_SPEC__
#define WANT_GNU_ERRNO         1
#else
#define WANT_GNU_ERRNO         0
#endif

#undef PUTC
#undef OUTNSTR
#undef _outnstr

#ifdef __STDIO_BUFFERS

#define PUTC(C,F)      putc_unlocked((C),(F))
#define OUTNSTR        _outnstr
#define _outnstr(stream, string, len)	__stdio_fwrite(string, len, stream)

#else  /* __STDIO_BUFFERS */

typedef struct {
	FILE f;
	unsigned char *bufend;		/* pointer to 1 past end of buffer */
	unsigned char *bufpos;
} __FILE_vsnprintf;

#ifdef __UCLIBC_HAS_FLOATS__
static void _outnstr(FILE *stream, const unsigned char *s, size_t n)
{
	__FILE_vsnprintf *f = (__FILE_vsnprintf *) stream;

	if (!__STDIO_STREAM_IS_FAKE_VSNPRINTF_NB(&f->f)) {
		__stdio_fwrite(s, n, &f->f);
	} else if (f->bufend > f->bufpos) {
		size_t r = f->bufend - f->bufpos;
		if (r > n) {
			r = n;
		}
		memcpy(f->bufpos, s, r);
		f->bufpos += r;
	}
}
#endif

static void putc_unlocked_sprintf(int c, __FILE_vsnprintf *f)
{
	if (!__STDIO_STREAM_IS_FAKE_VSNPRINTF_NB(&f->f)) {
		putc_unlocked(c, &f->f);
	} else if (f->bufpos < f->bufend) {
		*f->bufpos++ = c;
	}
}


#define PUTC(C,F)      putc_unlocked_sprintf((C),(__FILE_vsnprintf *)(F))
#define OUTNSTR        _outnstr

#endif /* __STDIO_BUFFERS */

#ifdef __UCLIBC_HAS_FLOATS__

static void _charpad(FILE * __restrict stream, int padchar, size_t numpad)
{
	/* TODO -- Use a buffer to cut down on function calls... */
	char pad[1];

	*pad = padchar;
	while (numpad) {
		OUTNSTR(stream, pad, 1);
		--numpad;
	}
}

static void _fp_out_narrow(FILE *fp, intptr_t type, intptr_t len, intptr_t buf)
{
	if (type & 0x80) {			/* Some type of padding needed. */
		int buflen = strlen((const char *) buf);
		if ((len -= buflen) > 0) {
			_charpad(fp, (type & 0x7f), len);
		}
		len = buflen;
	}
	if (len) {
		OUTNSTR(fp, (const char *) buf, len);
	}
}

#endif


enum {
	FLAG_PLUS = 0,
	FLAG_MINUS_LJUSTIFY,
	FLAG_HASH,
	FLAG_0_PAD,
	FLAG_SPACE,
};

/* layout                   01234  */
static const char spec[] = "+-#0 ";

/**********************************************************************/

/*
 * In order to ease translation to what arginfo and _print_info._flags expect,
 * we map:  0:int  1:char  2:longlong 4:long  8:short
 * and then _flags |= (((q << 7) + q) & 0x701) and argtype |= (_flags & 0x701)
 */

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
	/* j:(u)intmax_t z:(s)size_t  t:ptrdiff_t  \0:int */ \
	/* q:long_long  Z:(s)size_t */ \
	'h',   'l',  'L',  'j',  'z',  't',  'q', 'Z',  0, \
	 2,     4,    8,  IMS,   SS,  PDS,    8,  SS,   0, /* TODO -- fix!!! */\
     1,     8 \
}

static const char qual_chars[] = QUAL_CHARS;

/* static const char qual[] = "hlLq"; */
/**********************************************************************/

#if !defined(__UCLIBC_HAS_FLOATS__) && WANT_FLOAT_ERROR
static const char dbl_err[] = "<DOUBLE>";
#endif

#if defined(__UCLIBC_HAS_FLOATS__) || WANT_FLOAT_ERROR
/* layout                     012345678901234567   */
static const char u_spec[] = "%nbopxXudicsfgGeEaA";
#else
/* layout                     0123456789012   */
static const char u_spec[] = "%nbopxXudics";
#endif

/* WARNING: u_spec and u_radix need to stay in agreement!!! */
/* u_radix[i] <-> u_spec[i+2] for unsigned entries only */
static const char u_radix[] = "\x02\x08\x10\x10\x10\x0a";

int vfprintf(FILE * __restrict op, register const char * __restrict fmt,
			 va_list ap)
{
	union {
#ifdef LLONG_MAX
		long long ll;
#endif
#if LONG_MAX != INT_MAX
		long l;
#endif
		int i;
	} intarg;
	int i, cnt, dataargtype, len;
	const void *argptr = argptr; /* ok to be initialized. */
	register char *p;
	const char *fmt0;
	int preci, width;
#define upcase i
	int radix, dpoint /*, upcase*/;
	char tmp[65];		/* TODO - determine needed size from headers */
	char flag[sizeof(spec)];
	__STDIO_AUTO_THREADLOCK_VAR;

	__STDIO_AUTO_THREADLOCK(op);

	__STDIO_STREAM_VALIDATE(op);

	cnt = 0;

	if (__STDIO_STREAM_IS_NARROW_WRITING(op)
		|| !__STDIO_STREAM_TRANS_TO_WRITE(op, __FLAG_NARROW)
		) {

	while (*fmt) {
		if (*fmt == '%') {
			fmt0 = fmt;			/* save our position in case of bad format */
			++fmt;
			width = -1;			/* min field width */
			preci = -5;			/* max string width or mininum digits */
			radix = 10;			/* number base */
			dpoint = 0;			/* found decimal point */

			/* init flags */
			for (p =(char *) spec ; *p ; p++) {
				flag[p-spec] = '\0';
			}
			flag[FLAG_0_PAD] = ' ';

			/* process optional flags */
			for (p = (char *)spec ; *p ; ) {
				if (*fmt == *p) {
					flag[p-spec] = *fmt++;
					p = (char *)spec; /* restart scan */
				} else {
					p++;
				}
			}

			if (!flag[FLAG_PLUS]) {
				flag[FLAG_PLUS] = flag[FLAG_SPACE];
			}

			/* process optional width and precision */
			do {
				if (*fmt == '.') {
					++fmt;
					dpoint = 1;
				}
				if (*fmt == '*') {	/* parameter width value */
					++fmt;
					i = va_arg(ap, int);
				} else {
					for ( i = 0 ; (*fmt >= '0') && (*fmt <= '9') ; ++fmt ) {
						i = (i * 10) + (*fmt - '0');
					}
				}

				if (dpoint) {
					preci = i;
					if (i<0) {
						preci = -5;
					}
				} else {
					width = i;
					if (i<0) {
						width = -i;
						flag[FLAG_MINUS_LJUSTIFY] = 1;
					}
				}
			} while ((*fmt == '.') && !dpoint );

			/* process optional qualifier */
			p = (char *) qual_chars;
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

#if WANT_GNU_ERRNO
			if (*fmt == 'm') {
				flag[FLAG_PLUS] = '\0';
				flag[FLAG_0_PAD] = ' ';
				p = __glibc_strerror_r(errno, tmp, sizeof(tmp));
				goto print;
			}
#endif

			/* process format specifier */
			for (p = (char *) u_spec ; *p ; p++) {
				if (*fmt != *p) continue;
				if (p-u_spec < 1) {	/* print a % */
					goto charout;
				}
				if (p-u_spec < 2) {	/* store output count in int ptr */
					_store_inttype(va_arg(ap, void *),
								   dataargtype,
								   (intmax_t) (cnt));
					goto nextfmt;
				}

				if (p-u_spec < 10) {
					if (*p == 'p') {
#if INTPTR_MAX == INT_MAX
						dataargtype = 0;
#else
#error Fix dataargtype for pointers!
#endif
					}

					switch(dataargtype) {
						case (PA_INT|PA_FLAG_LONG_LONG):
#ifdef LLONG_MAX
							intarg.ll = va_arg(ap, long long);
							argptr = &intarg.ll;
							break;
#endif
						case (PA_INT|PA_FLAG_LONG):
#if LONG_MAX != INT_MAX
							intarg.l = va_arg(ap, long);
							argptr = &intarg.l;
							break;
#endif
						default:
							intarg.i = va_arg(ap, int);
							argptr = &intarg.i;
							break;
					}
				}

				if (p-u_spec < 8) { /* unsigned conversion */
					radix = u_radix[p-u_spec-2];
					upcase = ((*p == 'x') ? __UIM_LOWER : __UIM_UPPER);
					if (*p == 'p') {
						upcase = __UIM_LOWER;
						flag[FLAG_HASH] = 'p';
					}
					p = _uintmaxtostr(tmp + sizeof(tmp) - 1,
									  (uintmax_t)
									  _load_inttype(dataargtype, argptr, radix),
									  radix, upcase);

					flag[FLAG_PLUS] = '\0';	/* meaningless for unsigned */
					if (*p != '0') { /* non-zero */
						if (flag[FLAG_HASH]) {
							if (radix == 8) {
								*--p = '0';	/* add leadding zero */
							} else if (radix != 10) { /* either 2 or 16 */
								flag[FLAG_PLUS] = '0';
								*--p = 'b';
								if (radix == 16) {
									*p = 'x';
									if (*fmt == 'X') {
										*p = 'X';
									}
								}
							}
						}
					} else if (flag[FLAG_HASH] == 'p') { /* null pointer */
						p = "(nil)";
					}
				} else if (p-u_spec < 10) { /* signed conversion */
					p = _uintmaxtostr(tmp + sizeof(tmp) - 1,
									  (uintmax_t)
									  _load_inttype(dataargtype, argptr, -radix),
									  -radix, upcase);

				} else if (p-u_spec < 12) {	/* character or string */
					flag[FLAG_PLUS] = '\0';
					flag[FLAG_0_PAD] = ' ';
					if (*p == 'c') {	/* character */
						p = tmp;
						*p = va_arg(ap, int);
						/* This takes care of the "%c",0 case */
						len = 1;
						goto print_len_set;
					} else {	/* string */
						p = va_arg(ap, char *);
						if (!p) {
							p = "(null)";
							preci = 6;
						} else {
							if (preci < 0) {
								preci = INT_MAX;
							}
						}
						len = strnlen(p, preci);
						goto print_len_set;
					}
#if defined(__UCLIBC_HAS_FLOATS__) || WANT_FLOAT_ERROR
				} else if (p-u_spec < 27) {		/* floating point */
#endif /* defined(__UCLIBC_HAS_FLOATS__) || WANT_FLOAT_ERROR */
#if defined(__UCLIBC_HAS_FLOATS__)
					struct printf_info info;
					if (preci < 0) {
						preci = 6;
					}
					info.width = width;
					info.prec = preci;
					info.spec = *fmt;
					info.pad = flag[FLAG_0_PAD];
					info._flags = 0;
					if (flag[FLAG_PLUS] == '+') {
						PRINT_INFO_SET_FLAG(&info,showsign);
					} else if (flag[FLAG_PLUS] == ' ') {
						PRINT_INFO_SET_FLAG(&info,space);
					}
					if (flag[FLAG_HASH]) {
						PRINT_INFO_SET_FLAG(&info,alt);
					}
					if (flag[FLAG_MINUS_LJUSTIFY]) {
						PRINT_INFO_SET_FLAG(&info,left);
					}
#if 1
					cnt += _fpmaxtostr(op,
									   (__fpmax_t)
									   ((dataargtype == (8 << 8))
										? va_arg(ap, long double)
										: (long double) va_arg(ap, double)),
									   &info, _fp_out_narrow);
#else
					cnt += _fpmaxtostr(op,
									   (__fpmax_t)
									   ((lval > 1)
										? va_arg(ap, long double)
										: (long double) va_arg(ap, double)),
									   &info, _fp_out_narrow);
#endif
					goto nextfmt;
#elif WANT_FLOAT_ERROR
					(void) ((lval > 1) ? va_arg(ap, long double)
							: va_arg(ap, double)); /* carry on */
					p = (char *) dbl_err;
#endif /* defined(__UCLIBC_HAS_FLOATS__) */
				}

#if WANT_GNU_ERRNO
			print:
#endif
				{				/* this used to be printfield */
					/* cheaper than strlen call */
/*  					for ( len = 0 ; p[len] ; len++ ) { } */
					len = strnlen(p, SIZE_MAX);
				print_len_set:
					if ((*p == '-')
#if WANT_GNU_ERRNO
						&& (*fmt != 'm')
#endif
						&& (*fmt != 's')) {
						flag[FLAG_PLUS] = *p++;
						--len;
					}
				    if (flag[FLAG_PLUS]) {
						++len;
						++preci;
						if (flag[FLAG_PLUS] == '0') { /* base 16 */
							++preci; /* account for x or X */
						}
					}

					if (preci >= 0) {
						if ((*fmt == 's')
#if WANT_GNU_ERRNO
						|| (*fmt == 'm')
#endif
						) {
							if (len > preci) {
								len = preci;
							} else {
								preci = len;
							}
						}
						preci -= len;
						if (preci < 0) {
							preci = 0;
						}
						width -= preci;
					}

					width -= len;
					if (width < 0) {
						width = 0;
					}

					if (preci < 0) {
						preci = 0;
						if (!flag[FLAG_MINUS_LJUSTIFY]
							/* && flag[FLAG_PLUS] */
							&& (flag[FLAG_0_PAD] == '0')) {
							preci = width;
							width = 0;
						}
					}

					while (width + len + preci) {
						unsigned char ch;
						/* right padding || left padding */
						if ((!len && !preci)
							|| (width && !flag[FLAG_MINUS_LJUSTIFY])) {
							ch = ' ';
							--width;
						} else if (flag[FLAG_PLUS]) {
							ch = flag[FLAG_PLUS]; /* sign */
							if (flag[FLAG_PLUS]=='0') {	/* base 16 case */
								flag[FLAG_PLUS] = *p++;	/* get the x|X */
							} else {
								flag[FLAG_PLUS] = '\0';
							}
							--len;
						} else if (preci) {
							ch = '0';
							--preci;
						} else {
							ch = *p++; /* main field */
							--len;
						}
						++cnt;
						PUTC(ch, op);
					}
				}
				goto nextfmt;
			}

			fmt = fmt0;	/* this was an illegal format */
		}

	charout:
		++cnt;
		PUTC(*fmt, op); /* normal char out */

	nextfmt:
		++fmt;
	}

	}

	i = (__FERROR_UNLOCKED(op)) ? -1 : cnt;

	__STDIO_STREAM_VALIDATE(op);

	__STDIO_AUTO_THREADUNLOCK(op);

	return i;
}
libc_hidden_def(vfprintf)
