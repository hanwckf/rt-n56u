/* vi: set sw=4 ts=4: */
/*
 * Copyright (C) 2000-2005 by Erik Andersen <andersen@codepoet.org>
 *
 * GNU Lesser General Public License version 2.1 or later.
 */

#ifndef _LINUX_STRING_H_
#define _LINUX_STRING_H_

#include <dl-sysdep.h> /* for do_rem */
#include <features.h>

/* provide some sane defaults */
#ifndef do_rem
# define do_rem(result, n, base) ((result) = (n) % (base))
#endif
#ifndef do_div_10
# define do_div_10(result, remain) ((result) /= 10)
#endif

static size_t _dl_strlen(const char *str);
static char *_dl_strcat(char *dst, const char *src);
static char *_dl_strcpy(char *dst, const char *src);
static int _dl_strcmp(const char *s1, const char *s2);
static int _dl_strncmp(const char *s1, const char *s2, size_t len);
static char *_dl_strchr(const char *str, int c);
static char *_dl_strrchr(const char *str, int c);
static char *_dl_strstr(const char *s1, const char *s2);
static void *_dl_memcpy(void *dst, const void *src, size_t len);
static int _dl_memcmp(const void *s1, const void *s2, size_t len);
static void *_dl_memset(void *str, int c, size_t len);
static char *_dl_get_last_path_component(char *path);
static char *_dl_simple_ltoa(char *local, unsigned long i);
static char *_dl_simple_ltoahex(char *local, unsigned long i);

#ifndef NULL
#define NULL ((void *) 0)
#endif

static __always_inline size_t _dl_strlen(const char *str)
{
	register const char *ptr = (char *) str-1;
	while (*++ptr)
		;/* empty */
	return (ptr - str);
}

static __always_inline char * _dl_strcat(char *dst, const char *src)
{
	register char *ptr = dst-1;

	src--;
	while (*++ptr)
		;/* empty */
	ptr--;
	while ((*++ptr = *++src) != 0)
		;/* empty */
	return dst;
}

static __always_inline char * _dl_strcpy(char *dst, const char *src)
{
	register char *ptr = dst;

	dst--;src--;
	while ((*++dst = *++src) != 0)
		;/* empty */

	return ptr;
}

static __always_inline int _dl_strcmp(const char *s1, const char *s2)
{
	register unsigned char c1, c2;
	s1--;s2--;
	do {
		c1 = (unsigned char) *++s1;
		c2 = (unsigned char) *++s2;
		if (c1 == '\0')
			return c1 - c2;
	} while (c1 == c2);

	return c1 - c2;
}

static __always_inline int _dl_strncmp(const char *s1, const char *s2, size_t len)
{
	register unsigned char c1 = '\0';
	register unsigned char c2 = '\0';

	s1--;s2--;
	while (len > 0) {
		c1 = (unsigned char) *++s1;
		c2 = (unsigned char) *++s2;
		if (c1 == '\0' || c1 != c2)
			return c1 - c2;
		len--;
	}
	return c1 - c2;
}

static __always_inline char * _dl_strchr(const char *str, int c)
{
	register char ch;
	str--;
	do {
		if ((ch = *++str) == c)
			return (char *) str;
	}
	while (ch);

	return 0;
}

static __always_inline char * _dl_strrchr(const char *str, int c)
{
	register char *prev = 0;
	register char *ptr = (char *) str-1;

	while (*++ptr != '\0') {
		if (*ptr == c)
			prev = ptr;
	}
	if (c == '\0')
		return(ptr);
	return(prev);
}

static __always_inline char * _dl_strstr(const char *s1, const char *s2)
{
	register const char *s = s1;
	register const char *p = s2;

	do {
		if (!*p)
			return (char *) s1;;
		if (*p == *s) {
			++p;
			++s;
		} else {
			p = s2;
			if (!*s)
				return NULL;
			s = ++s1;
		}
	} while (1);
}

static __always_inline void * _dl_memcpy(void *dst, const void *src, size_t len)
{
	register char *a = dst-1;
	register const char *b = src-1;

	while (len) {
		*++a = *++b;
		--len;
	}
	return dst;
}

static __always_inline int _dl_memcmp(const void *s1, const void *s2, size_t len)
{
	unsigned char *c1 = (unsigned char *)s1-1;
	unsigned char *c2 = (unsigned char *)s2-1;

	while (len) {
		if (*++c1 != *++c2)
			return *c1 - *c2;
		len--;
	}
	return 0;
}

#if defined(powerpc)
/* Will generate smaller and faster code due to loop unrolling.*/
static __always_inline void * _dl_memset(void *to, int c, size_t n)
{
	unsigned long chunks;
	unsigned long *tmp_to;
	unsigned char *tmp_char;

	chunks = n / 4;
	tmp_to = to + n;
	c = c << 8 | c;
	c = c << 16 | c;
	if (!chunks)
		goto lessthan4;
	do {
		*--tmp_to = c;
	} while (--chunks);
lessthan4:
	n = n % 4;
	if (!n)
		return to;
	tmp_char = (unsigned char *)tmp_to;
	do {
		*--tmp_char = c;
	} while (--n);
	return to;
}
#else
static __always_inline void * _dl_memset(void *str, int c, size_t len)
{
	register char *a = str;

	while (len--)
		*a++ = c;

	return str;
}
#endif

static __always_inline char * _dl_get_last_path_component(char *path)
{
	register char *ptr = path-1;

	while (*++ptr)
		;/* empty */

	/* strip trailing slashes */
	while (ptr != path && *--ptr == '/') {
		*ptr = '\0';
	}

	/* find last component */
	while (ptr != path && *--ptr != '/')
		;/* empty */
	return ptr == path ? ptr : ptr+1;
}

/* Early on, we can't call printf, so use this to print out
 * numbers using the SEND_STDERR() macro.  Avoid using mod
 * or using long division */
static __always_inline char * _dl_simple_ltoa(char *local, unsigned long i)
{
	/* 20 digits plus a null terminator should be good for
	 * 64-bit or smaller ints (2^64 - 1)*/
	char *p = &local[22];
	*--p = '\0';
	do {
		char temp;
		do_rem(temp, i, 10);
		*--p = '0' + temp;
		do_div_10(i, temp);
	} while (i > 0);
	return p;
}

static __always_inline char * _dl_simple_ltoahex(char *local, unsigned long i)
{
	/* 16 digits plus a leading "0x" plus a null terminator,
	 * should be good for 64-bit or smaller ints */
	char *p = &local[22];
	*--p = '\0';
	do {
		char temp = i & 0xf;
		if (temp <= 0x09)
			*--p = '0' + temp;
		else
			*--p = 'a' - 0x0a + temp;
		i >>= 4;
	} while (i > 0);
	*--p = 'x';
	*--p = '0';
	return p;
}




/* The following macros may be used in dl-startup.c to debug
 * ldso before ldso has fixed itself up to make function calls */

/* On some (wierd) arches, none of this stuff works at all, so
 * disable the whole lot... */
#if defined(__mips__)

# define SEND_STDERR(X)
# define SEND_ADDRESS_STDERR(X, add_a_newline)
# define SEND_NUMBER_STDERR(X, add_a_newline)

#else

/* On some arches constant strings are referenced through the GOT.
 * This requires that load_addr must already be defined... */
#if defined(mc68000)  || defined(__arm__) || defined(__thumb__) || \
    defined(__mips__) || defined(__sh__)  || defined(__powerpc__) || \
    defined(__avr32__) || defined(__xtensa__) || defined(__sparc__) || defined(__microblaze__)
# define CONSTANT_STRING_GOT_FIXUP(X) \
	if ((X) < (const char *) load_addr) (X) += load_addr
# define NO_EARLY_SEND_STDERR
#else
# define CONSTANT_STRING_GOT_FIXUP(X)
#endif

#define SEND_STDERR(X) \
{ \
	const char *tmp1 = (X); \
	CONSTANT_STRING_GOT_FIXUP(tmp1); \
	_dl_write(2, tmp1, _dl_strlen(tmp1)); \
}

#define SEND_ADDRESS_STDERR(ADR, add_a_newline) \
{ \
	char tmp[26], v, *tmp2, *tmp1 = tmp; \
	unsigned long X = (unsigned long)(ADR); \
	CONSTANT_STRING_GOT_FIXUP(tmp1); \
	tmp2 = tmp1 + sizeof(tmp); \
	*--tmp2 = '\0'; \
	if (add_a_newline) *--tmp2 = '\n'; \
	do { \
		v = (X) & 0xf; \
		if (v <= 0x09) \
			*--tmp2 = '0' + v; \
		else \
			*--tmp2 = 'a' - 0x0a + v; \
		(X) >>= 4; \
	} while ((X) > 0); \
	*--tmp2 = 'x'; \
	*--tmp2 = '0'; \
	_dl_write(2, tmp2, tmp1 - tmp2 + sizeof(tmp) - 1); \
}

#define SEND_NUMBER_STDERR(NUM, add_a_newline) \
{ \
	char tmp[26], v, *tmp2, *tmp1 = tmp; \
	unsigned long X = (unsigned long)(NUM); \
	CONSTANT_STRING_GOT_FIXUP(tmp1); \
	tmp2 = tmp1 + sizeof(tmp); \
	*--tmp2 = '\0'; \
	if (add_a_newline) *--tmp2 = '\n'; \
	do { \
		do_rem(v, (X), 10); \
		*--tmp2 = '0' + v; \
		do_div_10((X), v); \
	} while ((X) > 0); \
	_dl_write(2, tmp2, tmp1 - tmp2 + sizeof(tmp) - 1); \
}
#endif

/* Some targets may have to override this to something that doesn't
 * reference constant strings through the GOT.  This macro should be
 * preferred over SEND_STDERR for constant strings before we complete
 * bootstrap.
 */
#ifndef SEND_EARLY_STDERR
# define SEND_EARLY_STDERR(S) SEND_STDERR(S)
#else
# define EARLY_STDERR_SPECIAL
#endif

#ifdef __SUPPORT_LD_DEBUG_EARLY__
# define SEND_STDERR_DEBUG(X) SEND_STDERR(X)
# define SEND_EARLY_STDERR_DEBUG(X) SEND_EARLY_STDERR(X)
# define SEND_NUMBER_STDERR_DEBUG(X, add_a_newline) SEND_NUMBER_STDERR(X, add_a_newline)
# define SEND_ADDRESS_STDERR_DEBUG(X, add_a_newline) SEND_ADDRESS_STDERR(X, add_a_newline)
#else
# define SEND_STDERR_DEBUG(X)
# define SEND_EARLY_STDERR_DEBUG(X)
# define SEND_NUMBER_STDERR_DEBUG(X, add_a_newline)
# define SEND_ADDRESS_STDERR_DEBUG(X, add_a_newline)
#endif

#endif
