/*
 * Copyright (C) 2004 Joakim Tjernlund
 * Copyright (C) 2000-2005 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* These are carefully optimized mem*() functions for PPC written in C.
 * Don't muck around with these function without checking the generated
 * assembler code.
 * It is possible to optimize these significantly more by using specific
 * data cache instructions(mainly dcbz). However that requires knownledge
 * about the CPU's cache line size.
 *
 * BUG ALERT!
 * The cache instructions on MPC8xx CPU's are buggy(they don't update
 * the DAR register when causing a DTLB Miss/Error) and cannot be
 * used on 8xx CPU's without a kernel patch to work around this
 * problem.
 */

#include <string.h>


static __inline__ int expand_byte_word(int c){
	/* this does:
	   c = c << 8 | c;
	   c = c << 16 | c ;
	*/
	__asm__("rlwimi	%0,%0,8,16,23\n"
	    "\trlwimi	%0,%0,16,0,15\n"
	    : "=r" (c) : "0" (c));
	return c;
}

void *memset(void *to, int c, size_t n)
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
libc_hidden_def(memset)
