/*
 * This string-include defines all string functions as inline
 * functions. Use gcc. It also assumes ds=es=data space, this should be
 * normal. Most of the string-functions are rather heavily hand-optimized,
 * see especially strtok,strstr,str[c]spn. They should work, but are not
 * very easy to understand. Everything is done entirely within the register
 * set, making the functions fast and clean. String instructions have been
 * used through-out, making for "slightly" unclear code :-)
 *
 *		NO Copyright (C) 1991, 1992 Linus Torvalds,
 *		consider these trivial functions to be PD.
 */

/*
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*
 * Modified for uClibc by Erik Andersen <andersen@codepoet.org>
 * These make no attempt to use nifty things like mmx/3dnow/etc.
 * These are not inline, and will therefore not be as fast as
 * modifying the headers to use inlines (and cannot therefore
 * do tricky things when dealing with const memory).  But they
 * should (I hope!) be faster than their generic equivalents....
 *
 * More importantly, these should provide a good example for
 * others to follow when adding arch specific optimizations.
 *  -Erik
 */

#include <string.h>

#undef memchr
/*#define memchr TESTING*/
void *memchr(const void *s, int c, size_t count)
{
	void *edi;
	int ecx;
	__asm__ __volatile__(
		"	jecxz	1f\n"
		"	repne; scasb\n"
		"	leal	-1(%%edi), %%edi\n"
		"	je	2f\n"
		"1:\n"
		"	xorl	%%edi, %%edi\n" /* NULL */
		"2:\n"
		: "=&D" (edi), "=&c" (ecx)
		: "a" (c), "0" (s), "1" (count)
		/* : no clobbers */
	);
	return edi;
}
#ifndef memchr
libc_hidden_def(memchr)
#else
/* Uncomment TESTING, gcc -D_GNU_SOURCE -m32 -Os memchr.c -o memchr
 * and run ./memchr
 */
int main()
{
	static const char str[] = "abc.def";
	printf((char*)memchr(str, '.',-2) - str == 3 ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.',-1) - str == 3 ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 0) == NULL    ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 1) == NULL    ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 2) == NULL    ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 3) == NULL    ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 4) - str == 3 ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str, '.', 5) - str == 3 ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str+3, '.', 0) == NULL    ? "ok\n" : "BAD!\n");
	printf((char*)memchr(str+3, '.', 5) - str == 3 ? "ok\n" : "BAD!\n");
}
#endif
