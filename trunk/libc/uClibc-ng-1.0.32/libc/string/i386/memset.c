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
 *
 * 2009-04: modified by Denys Vlasenko <vda.linux@googlemail.com>
 * Fill byte-by-byte is a bit too slow. I prefer 46 byte function
 * which fills x4 faster than 21 bytes one.
 */

#include <string.h>

#undef memset
void *memset(void *s, int c, size_t count)
{
	int reg, edi;
	__asm__ __volatile__(

		/* Most of the time, count is divisible by 4 and nonzero */
		/* It's better to make this case faster */
	/*	"	jecxz	9f\n" - (optional) count == 0: goto ret */
		"	mov	%%ecx, %1\n"
		"	shr	$2, %%ecx\n"
		"	jz	1f\n" /* zero words: goto fill_bytes */
		/* extend 8-bit fill to 32 bits */
		"	movzx	%%al, %%eax\n" /* 3 bytes */
	/* or:	"	and	$0xff, %%eax\n" - 5 bytes */
		"	imul	$0x01010101, %%eax\n" /* 6 bytes */
		/* fill full words */
		"	rep; stosl\n"
		/* fill 0-3 bytes */
		"1:	and	$3, %1\n"
		"	jz	9f\n" /* (count & 3) == 0: goto end */
		"2:	stosb\n"
		"	dec	%1\n"
		"	jnz	2b\n"
		/* end */
		"9:\n"

		: "=&D" (edi), "=&r" (reg)
		: "0" (s), "a" (c), "c" (count)
		: "memory"
	);
	return s;
}
libc_hidden_def(memset)

/*
gcc 4.3.1
=========
57                     push   %edi
8b 7c 24 08            mov    0x8(%esp),%edi
8b 4c 24 10            mov    0x10(%esp),%ecx
8b 44 24 0c            mov    0xc(%esp),%eax
89 ca                  mov    %ecx,%edx
c1 e9 02               shr    $0x2,%ecx
74 0b                  je     1f <__GI_memset+0x1f>
0f b6 c0               movzbl %al,%eax
69 c0 01 01 01 01      imul   $0x1010101,%eax,%eax
f3 ab                  rep stos %eax,%es:(%edi)
83 e2 03               and    $0x3,%edx
74 04                  je     28 <__GI_memset+0x28>
aa                     stos   %al,%es:(%edi)
4a                     dec    %edx
75 fc                  jne    24 <__GI_memset+0x24>
8b 44 24 08            mov    0x8(%esp),%eax
5f                     pop    %edi
c3                     ret
*/
