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

#undef strncpy
/*#define strncpy TESTING*/
char *strncpy(char * dest, const char * src, size_t count)
{
	int esi, edi, ecx, eax;
	__asm__ __volatile__(
		"1:	subl	$1, %%ecx\n" /* not dec! it doesnt set CF */
		"	jc	2f\n"
		"	lodsb\n"
		"	stosb\n"
		"	testb	%%al, %%al\n"
		"	jnz	1b\n"
		"	rep; stosb\n"
		"2:\n"
		: "=&S" (esi), "=&D" (edi), "=&c" (ecx), "=&a" (eax)
		: "0" (src), "1" (dest), "2" (count)
		: "memory"
	);
	return dest;
}
#ifndef strncpy
libc_hidden_def(strncpy)
#else
/* Uncomment TESTING, gcc -D_GNU_SOURCE -m32 -Os strncpy.c -o strncpy
 * and run ./strncpy
 */
int main()
{
	static char str[99];

	str[3] = '*'; str[4] = 0; strncpy(str, "abc", 3);
	printf(strcmp(str, "abc*") == 0 ? "ok\n" : "BAD!\n");

	str[4] = '*'; str[5] = '+'; strncpy(str, "abc", 5);
	printf(strcmp(str, "abc") == 0 && str[4] == 0 && str[5] == '+' ?
				"ok\n" : "BAD!\n");
	strncpy(str, "def", 0); /* should do nothing */
	printf(strcmp(str, "abc") == 0 && str[4] == 0 && str[5] == '+' ?
				"ok\n" : "BAD!\n");
}
#endif
