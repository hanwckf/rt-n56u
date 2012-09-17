/*
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Library General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Library General Public License for more
 * details.
 */

/* These are carefully optimized mem*() functions for PPC written in C.
 * Don't muck around with these function without checking the generated
 * assmbler code.
 * It is possible to optimize these significantly more by using specific
 * data cache instructions(mainly dcbz). However that requires knownledge
 * about the CPU's cache line size.
 *
 * BUG ALERT!
 * The cache instructions on MPC8xx CPU's are buggy(they don't update
 * the DAR register when causing a DTLB Miss/Error) and cannot be
 * used on 8xx CPU's without a kernel patch to work around this
 * problem.
 *   
 * Copyright (C) 2004 Joakim Tjernlund
 */

#define _STDIO_UTILITY
#define _GNU_SOURCE
#include <string.h>
#include <locale.h> /* for __LOCALE_C_ONLY */

#ifdef L_memcpy
void attribute_hidden *__memcpy(void *to, const void *from, size_t n)
/* PPC can do pre increment and load/store, but not post increment and load/store.
   Therefore use *++ptr instead of *ptr++. */
{
	unsigned long rem, chunks, tmp1, tmp2;
	unsigned char *tmp_to;
	unsigned char *tmp_from = (unsigned char *)from;

	chunks = n / 8;
	tmp_from -= 4;
	tmp_to = to - 4;
	if (!chunks)
		goto lessthan8;
	rem = (unsigned long )tmp_to % 4;
	if (rem)
		goto align;
 copy_chunks:
	do {
		/* make gcc to load all data, then store it */
		tmp1 = *(unsigned long *)(tmp_from+4);
		tmp_from += 8;
		tmp2 = *(unsigned long *)tmp_from;
		*(unsigned long *)(tmp_to+4) = tmp1;
		tmp_to += 8;
		*(unsigned long *)tmp_to = tmp2;
	} while (--chunks);
 lessthan8:
	n = n % 8;
	if (n >= 4) {
		*(unsigned long *)(tmp_to+4) = *(unsigned long *)(tmp_from+4);
		tmp_from += 4;
		tmp_to += 4;
		n = n-4;
	}
	if (!n ) return to;
	tmp_from += 3;
	tmp_to += 3;
	do {
		*++tmp_to = *++tmp_from;
	} while (--n);
	
	return to;
 align:
	rem = 4 - rem;
	n = n - rem;
	do {
		*(tmp_to+4) = *(tmp_from+4);
		++tmp_from;
		++tmp_to;
	} while (--rem);
	chunks = n / 8;
	if (chunks)
		goto copy_chunks;
	goto lessthan8;
}
strong_alias(__memcpy, memcpy);
#endif

#ifdef L_memmove
void attribute_hidden *__memmove(void *to, const void *from, size_t n)
{
	unsigned long rem, chunks, tmp1, tmp2;
	unsigned char *tmp_to;
	unsigned char *tmp_from = (unsigned char *)from;

	if (tmp_from >= (unsigned char *)to)
		return memcpy(to, from, n);
	chunks = n / 8;
	tmp_from += n;
	tmp_to = to + n;
	if (!chunks)
		goto lessthan8;
	rem = (unsigned long )tmp_to % 4;
	if (rem)
		goto align;
 copy_chunks:
	do {
		/* make gcc to load all data, then store it */
		tmp1 = *(unsigned long *)(tmp_from-4);
		tmp_from -= 8;
		tmp2 = *(unsigned long *)tmp_from;
		*(unsigned long *)(tmp_to-4) = tmp1;
		tmp_to -= 8;
		*(unsigned long *)tmp_to = tmp2;
	} while (--chunks);
 lessthan8:
	n = n % 8;
	if (n >= 4) {
		*(unsigned long *)(tmp_to-4) = *(unsigned long *)(tmp_from-4);
		tmp_from -= 4;
		tmp_to -= 4;
		n = n-4;
	}
	if (!n ) return to;
	do {
		*--tmp_to = *--tmp_from;
	} while (--n);
	
	return to;
 align:
	rem = 4 - rem;
	n = n - rem;
	do {
		*--tmp_to = *--tmp_from;
	} while (--rem);
	chunks = n / 8;
	if (chunks)
		goto copy_chunks;
	goto lessthan8;
}
strong_alias(__memmove, memmove);
#endif

#ifdef L_memset
static inline int expand_byte_word(int c){
	/* this does: 
	   c = c << 8 | c;
	   c = c << 16 | c ;
	*/
	asm("rlwimi	%0,%0,8,16,23\n"
	    "\trlwimi	%0,%0,16,0,15\n"
	    : "=r" (c) : "0" (c));
	return c;
}
void attribute_hidden *__memset(void *to, int c, size_t n)
{
	unsigned long rem, chunks;
	unsigned char *tmp_to;

	chunks = n / 8;
	tmp_to = to - 4;
	c = expand_byte_word(c);
	if (!chunks)
		goto lessthan8;
	rem = (unsigned long )tmp_to % 4;
	if (rem)
		goto align;
 copy_chunks:
	do {
		*(unsigned long *)(tmp_to+4) = c;
		tmp_to += 4;
		*(unsigned long *)(tmp_to+4) = c;
		tmp_to += 4;
	} while (--chunks);
 lessthan8:
	n = n % 8;
	if (n >= 4) {
		*(unsigned long *)(tmp_to+4) = c;
		tmp_to += 4;
		n = n-4;
	}
	if (!n ) return to;
	tmp_to += 3;
	do {
		*++tmp_to = c;
	} while (--n);
	
	return to;
 align:
	rem = 4 - rem;
	n = n-rem;
	do {
		*(tmp_to+4) = c;
		++tmp_to;
	} while (--rem);
	chunks = n / 8;
	if (chunks)
		goto copy_chunks;
	goto lessthan8;
}
strong_alias(__memset, memset);
#endif

#ifdef L_bzero
weak_alias(__bzero,bzero);
void __bzero(void *s, size_t n)
{
	(void)memset(s, 0, n);
}
#endif
