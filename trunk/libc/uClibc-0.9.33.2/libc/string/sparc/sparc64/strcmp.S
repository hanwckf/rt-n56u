/* Compare two strings for differences.
   For SPARC v9.
   Copyright (C) 1997, 1999, 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jan Vondrak <jvon4518@ss1000.ms.mff.cuni.cz> and
                  Jakub Jelinek <jj@ultra.linux.cz>.

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

#include <asm/asi.h>
#ifndef XCC
	.register	%g2, #scratch
	.register	%g3, #scratch
	.register	%g6, #scratch
#endif

	/* Normally, this uses
	   ((xword - 0x0101010101010101) & 0x8080808080808080) test
	   to find out if any byte in xword could be zero. This is fast, but
	   also gives false alarm for any byte in range 0x81-0xff. It does
	   not matter for correctness, as if this test tells us there could
	   be some zero byte, we check it byte by byte, but if bytes with
	   high bits set are common in the strings, then this will give poor
	   performance. You can #define EIGHTBIT_NOT_RARE and the algorithm
	   will use one tick slower, but more precise test
	   ((xword - 0x0101010101010101) & (~xword) & 0x8080808080808080),
	   which does not give any false alarms (but if some bits are set,
	   one cannot assume from it which bytes are zero and which are not).
	   It is yet to be measured, what is the correct default for glibc
	   in these days for an average user.
	 */

	.text
	.align		32
ENTRY(strcmp)
	sethi		%hi(0x01010101), %g1			/* IEU0		Group		*/
	andcc		%o0, 7, %g0				/* IEU1				*/
	bne,pn		%icc, 7f				/* CTI				*/
	 or		%g1, %lo(0x01010101), %g1		/* IEU0		Group		*/

	andcc		%o1, 7, %g3				/* IEU1				*/
	bne,pn		%icc, 9f				/* CTI				*/
	 sllx		%g1, 32, %g2				/* IEU0		Group		*/
	ldx		[%o0], %o2				/* Load				*/

	or		%g1, %g2, %g1				/* IEU0		Group		*/
1:	ldx		[%o1], %o3				/* Load				*/
	sub		%o1, %o0, %o1				/* IEU1				*/
	sllx		%g1, 7, %g2				/* IEU0		Group		*/

2:	add		%o0, 8, %o0				/* IEU1				*/
	sub		%o2, %g1, %g3				/* IEU0		Group		*/
	subcc		%o2, %o3, %g0				/* IEU1				*/
	bne,pn		%xcc, 13f				/* CTI				*/

#ifdef EIGHTBIT_NOT_RARE
	 andn		%g3, %o2, %g4				/* IEU0		Group		*/
	ldxa		[%o0] ASI_PNF, %o2			/* Load				*/
	andcc		%g4, %g2, %g0				/* IEU1		Group		*/
#else
	 ldxa		[%o0] ASI_PNF, %o2			/* Load		Group		*/
	andcc		%g3, %g2, %g0				/* IEU1				*/
#endif
	be,a,pt		%xcc, 2b				/* CTI				*/
	 ldxa		[%o1 + %o0] ASI_PNF, %o3		/* Load		Group		*/

	addcc		%g3, %g1, %o4				/* IEU1				*/
	srlx		%g3, 32, %g3				/* IEU0				*/
	andcc		%g3, %g2, %g0				/* IEU1		Group		*/
	be,pt		%xcc, 3f				/* CTI				*/

	 srlx		%o4, 56, %o5				/* IEU0				*/
	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4f				/* CTI				*/
	 srlx		%o4, 48, %o5				/* IEU0				*/

	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4f				/* CTI				*/
	 srlx		%o4, 40, %o5				/* IEU0				*/
	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/

	be,pn		%icc, 4f				/* CTI				*/
	 srlx		%o4, 32, %o5				/* IEU0				*/
	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4f				/* CTI				*/

3:	 srlx		%o4, 24, %o5				/* IEU0				*/
	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4f				/* CTI				*/
	 srlx		%o4, 16, %o5				/* IEU0				*/

	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4f				/* CTI				*/
	 srlx		%o4, 8, %o5				/* IEU0				*/
	andcc		%o5, 0xff, %g0				/* IEU1		Group		*/

	be,pn		%icc, 4f				/* CTI				*/
	 andcc		%o4, 0xff, %g0				/* IEU1		Group		*/
	bne,a,pn	%icc, 2b				/* CTI				*/
	 ldxa		[%o1 + %o0] ASI_PNF, %o3		/* Load				*/

4:	retl							/* CTI+IEU1	Group		*/
	 clr		%o0					/* IEU0				*/

	.align		32
13:	mov		0xff, %g6				/* IEU0		Group		*/
#ifdef EIGHTBIT_NOT_RARE
	andcc		%g4, %g2, %g0				/* IEU1				*/
#else
	andcc		%g3, %g2, %g0				/* IEU1				*/
#endif
	be,pt		%xcc, 25f				/* CTI				*/
	 addcc		%g3, %g1, %o4				/* IEU1		Group		*/

	srlx		%g3, 32, %g3				/* IEU0				*/
	andcc		%g3, %g2, %g0				/* IEU1		Group		*/
	be,pt		%xcc, 23f				/* CTI				*/
	 sllx		%g6, 56, %o5				/* IEU0				*/

	andcc		%o4, %o5, %g0				/* IEU1		Group		*/
	be,pn		%xcc, 24f				/* CTI				*/
	 sllx		%g6, 48, %o5				/* IEU0				*/
	andcc		%o4, %o5, %g0				/* IEU1		Group		*/

	be,pn		%xcc, 24f				/* CTI				*/
	 sllx		%g6, 40, %o5				/* IEU0				*/
	andcc		%o4, %o5, %g0				/* IEU1		Group		*/
	be,pn		%xcc, 24f				/* CTI				*/

	 sllx		%g6, 32, %o5				/* IEU0				*/
	andcc		%o4, %o5, %g0				/* IEU1		Group		*/
	be,pn		%xcc, 24f				/* CTI				*/
23:	 sllx		%g6, 24, %o5				/* IEU0				*/

	andcc		%o4, %o5, %g0				/* IEU1		Group		*/
	be,pn		%icc, 24f				/* CTI				*/
	 sllx		%g6, 16, %o5				/* IEU0				*/
	andcc		%o4, %o5, %g0				/* IEU1		Group		*/

	be,pn		%icc, 24f				/* CTI				*/
	 sllx		%g6, 8, %o5				/* IEU0				*/
	andcc		%o4, %o5, %g0				/* IEU1		Group		*/
	be,pn		%icc, 24f				/* CTI				*/

	 mov		%g6, %o5				/* IEU0				*/
25:	cmp		%o4, %o3				/* IEU1		Group		*/
5:	mov		-1, %o0					/* IEU0				*/
	retl							/* CTI+IEU1	Group		*/

	 movgu		%xcc, 1, %o0				/* Single	Group		*/

	.align		16
24:	sub		%o5, 1, %g6				/* IEU0		Group		*/
	clr		%o0					/* IEU1				*/
	or		%o5, %g6, %o5				/* IEU0		Group		*/
	andn		%o4, %o5, %o4				/* IEU0		Group		*/

	andn		%o3, %o5, %o3				/* IEU1				*/
	cmp		%o4, %o3				/* IEU1		Group		*/
	movgu		%xcc, 1, %o0				/* Single	Group		*/
	retl							/* CTI+IEU1	Group		*/

	 movlu		%xcc, -1, %o0				/* Single	Group		*/
6:	retl							/* CTI+IEU1	Group		*/
	 mov		%o4, %o0				/* IEU0				*/

	.align		16
7:	ldub		[%o0], %o2				/* Load				*/
	add		%o0, 1, %o0				/* IEU1				*/
	ldub		[%o1], %o3				/* Load		Group		*/
	sllx		%g1, 32, %g2				/* IEU0				*/

8:	add		%o1, 1, %o1				/* IEU1				*/
	subcc		%o2, %o3, %o4				/* IEU1		Group		*/
	bne,pn		%xcc, 6b				/* CTI				*/
	 lduba		[%o0] ASI_PNF, %o2			/* Load				*/

	brz,pn		%o3, 4b					/* CTI+IEU1	Group		*/
	 lduba		[%o1] ASI_PNF, %o3			/* Load				*/
	andcc		%o0, 7, %g0				/* IEU1		Group		*/
	bne,a,pn	%icc, 8b				/* CTI				*/

	 add		%o0, 1, %o0				/* IEU0				*/
	or		%g1, %g2, %g1				/* IEU0		Group		*/
	andcc		%o1, 7, %g3				/* IEU1				*/
	be,a,pn		%icc, 1b				/* CTI				*/

	 ldxa		[%o0] ASI_PNF, %o2			/* Load		Group		*/
9:	sllx		%g3, 3, %g5				/* IEU0				*/
	mov		64, %o5					/* IEU1				*/
	sub		%o1, %g3, %o1				/* IEU0		Group		*/

	sub		%o5, %g5, %o5				/* IEU1				*/
	ldxa		[%o1] ASI_PNF, %g6			/* Load		Group		*/
	or		%g1, %g2, %g1				/* IEU0				*/
	sub		%o1, %o0, %o1				/* IEU1				*/

	sllx		%g1, 7, %g2				/* IEU0		Group		*/
	add		%o1, 8, %o1				/* IEU1				*/
								/* %g1 = 0101010101010101
								 * %g2 = 8080808080800880
								 * %g5 = number of bits to shift left
								 * %o5 = number of bits to shift right */
10:	sllx		%g6, %g5, %o3				/* IEU0		Group		*/
	ldxa		[%o1 + %o0] ASI_PNF, %g6		/* Load				*/

11:	srlx		%g6, %o5, %o4				/* IEU0		Group		*/
	ldxa		[%o0] ASI_PNF, %o2			/* Load				*/
	or		%o3, %o4, %o3				/* IEU1				*/
	add		%o0, 8, %o0				/* IEU0		Group		*/

	subcc		%o2, %o3, %g0				/* IEU1				*/
#ifdef EIGHTBIT_NOT_RARE
	sub		%o2, %g1, %g3				/* IEU0		Group		*/
	bne,pn		%xcc, 13b				/* CTI				*/
	 andn		%g3, %o2, %g4				/* IEU0		Group		*/

	andcc		%g4, %g2, %g0				/* IEU1		Group		*/
	be,pt		%xcc, 10b				/* CTI				*/
	 srlx		%g4, 32, %g4				/* IEU0				*/
	andcc		%g4, %g2, %g0				/* IEU1		Group		*/
#else
	bne,pn		%xcc, 13b				/* CTI				*/
	 sub		%o2, %g1, %g3				/* IEU0		Group		*/
	andcc		%g3, %g2, %g0				/* IEU1		Group		*/

	be,pt		%xcc, 10b				/* CTI				*/
	 srlx		%g3, 32, %g3				/* IEU0				*/
	andcc		%g3, %g2, %g0				/* IEU1		Group		*/
#endif
	be,pt		%xcc, 12f				/* CTI				*/

	 srlx		%o2, 56, %g3				/* IEU0				*/
	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/
	 srlx		%o2, 48, %g3				/* IEU0				*/

	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/
	 srlx		%o2, 40, %g3				/* IEU0				*/
	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/

	be,pn		%icc, 4b				/* CTI				*/
	 srlx		%o2, 32, %g3				/* IEU0				*/
	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/

12:	 srlx		%o2, 24, %g3				/* IEU0				*/
	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/
	 srlx		%o2, 16, %g3				/* IEU0				*/

	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/
	 srlx		%o2, 8, %g3				/* IEU0				*/
	andcc		%g3, 0xff, %g0				/* IEU1		Group		*/

	be,pn		%icc, 4b				/* CTI				*/
	 andcc		%o2, 0xff, %g0				/* IEU1		Group		*/
	be,pn		%icc, 4b				/* CTI				*/
	 sllx		%g6, %g5, %o3				/* IEU0				*/

	ba,pt		%xcc, 11b				/* CTI		Group		*/
	 ldxa		[%o1 + %o0] ASI_PNF, %g6		/* Load				*/
END(strcmp)
libc_hidden_def(strcmp)
