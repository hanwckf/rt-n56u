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

#ifdef __USE_GNU

#undef strnlen
/*#define strnlen TESTING*/
size_t strnlen(const char *s, size_t count)
{
	int edx;
	int eax;
	__asm__ __volatile__(
		"	leal	-1(%%ecx), %%eax\n"
		"1:	incl	%%eax\n"
		"	decl	%%edx\n"
		"	jz	3f\n"
		"	cmpb	$0, (%%eax)\n"
		"	jnz	1b\n"
		"3:	subl	%%ecx, %%eax"
		: "=a" (eax), "=&d" (edx)
		: "c" (s), "1" (count + 1)
	);
	return eax;
}
#ifndef strnlen
libc_hidden_def(strnlen)
#else
/* Uncomment TESTING, gcc -D_GNU_SOURCE -m32 -Os strnlen.c -o strnlen
 * and run ./strnlen
 */
int main()
{
	printf(strnlen("abc\0def", -2) == 3 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", -1) == 3 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 0) == 0 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 1) == 1 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 2) == 2 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 3) == 3 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 4) == 3 ? "ok\n" : "BAD!\n");
	printf(strnlen("abc\0def", 5) == 3 ? "ok\n" : "BAD!\n");
}
#endif

#endif
