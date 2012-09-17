/* Determine the length of a string.
   For SPARC v7.
   Copyright (C) 1996, 1999, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jj@ultra.linux.cz>.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

	/* Normally, this uses ((xword - 0x01010101) & 0x80808080) test
	   to find out if any byte in xword could be zero. This is fast, but
	   also gives false alarm for any byte in range 0x81-0xff. It does
	   not matter for correctness, as if this test tells us there could
	   be some zero byte, we check it byte by byte, but if bytes with
	   high bits set are common in the strings, then this will give poor
	   performance. You can #define EIGHTBIT_NOT_RARE and the algorithm
	   will use one tick slower, but more precise test
	   ((xword - 0x01010101) & (~xword) & 0x80808080),
	   which does not give any false alarms (but if some bits are set,
	   one cannot assume from it which bytes are zero and which are not).
	   It is yet to be measured, what is the correct default for glibc
	   in these days for an average user.
	 */

	.text
	.align		4
10:	ldub		[%o0], %o5
	cmp		%o5, 0
	be		1f
	 add		%o0, 1, %o0
	andcc		%o0, 3, %g0
	be		4f
	 or		%o4, %lo(0x80808080), %o3
	ldub		[%o0], %o5
	cmp		%o5, 0
	be		2f
	 add		%o0, 1, %o0
	andcc		%o0, 3, %g0
	be		5f
	 sethi		%hi(0x01010101), %o4
	ldub		[%o0], %o5
	cmp		%o5, 0
	be		3f
	 add		%o0, 1, %o0
	b		11f
	 or		%o4, %lo(0x01010101), %o2
1:	retl
	 mov		0, %o0
2:	retl
	 mov		1, %o0
3:	retl
	 mov		2, %o0

ENTRY(strlen)
	mov		%o0, %o1
	andcc		%o0, 3, %g0
	bne		10b
	 sethi		%hi(0x80808080), %o4
	or		%o4, %lo(0x80808080), %o3
4:	sethi		%hi(0x01010101), %o4
5:	or		%o4, %lo(0x01010101), %o2
11:	ld		[%o0], %o5
12:	sub		%o5, %o2, %o4
#ifdef EIGHTBIT_NOT_RARE
	andn		%o4, %o5, %o4
#endif
	andcc		%o4, %o3, %g0
	be		11b
	 add		%o0, 4, %o0

	srl		%o5, 24, %g5
	andcc		%g5, 0xff, %g0
	be		13f
	 add		%o0, -4, %o4
	srl		%o5, 16, %g5
	andcc		%g5, 0xff, %g0
	be		13f
	 add		%o4, 1, %o4
	srl		%o5, 8, %g5
	andcc		%g5, 0xff, %g0
	be		13f
	 add		%o4, 1, %o4
	andcc		%o5, 0xff, %g0
	bne,a		12b
	 ld		[%o0], %o5
	add		%o4, 1, %o4
13:	retl
	 sub		%o4, %o1, %o0
END(strlen)
libc_hidden_def(strlen)
