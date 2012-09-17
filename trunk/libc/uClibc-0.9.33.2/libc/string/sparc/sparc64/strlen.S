/* Determine the length of a string.  For SPARC v9.
   Copyright (C) 1998, 1999, 2003 Free Software Foundation, Inc.
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
ENTRY(strlen)
	sethi		%hi(0x01010101), %g1		/* IEU0		Group		*/
	ldub		[%o0], %o3			/* Load				*/
	or		%g1, %lo(0x01010101), %g1	/* IEU0		Group		*/
	mov		%o0, %o1			/* IEU1				*/

	sllx		%g1, 32, %g4			/* IEU0		Group 		*/
	andcc		%o0, 7, %g0			/* IEU1				*/
	or		%g1, %g4, %g1			/* IEU0		Group		*/
	brz,pn		%o3, 13f			/* CTI+IEU1			*/

	 sllx		%g1, 7, %g4			/* IEU0		Group		*/
	bne,a,pn	%icc, 15f			/* CTI				*/
	 add		%o0, 1, %o0			/* IEU1				*/
							/* %g1 = 0x0101010101010101	*
							 * %g4 = 0x8080808080808080	*
							 * %o0 = string pointer		*
							 * %o1 = start of string	*/
1:	ldx		[%o0], %o3			/* Load		Group		*/

	add		%o0, 8, %o0			/* IEU1				*/
2:	sub		%o3, %g1, %o2			/* IEU0		Group		*/
#ifdef EIGHTBIT_NOT_RARE
	andn		%o2, %o3, %o5			/* IEU0		Group		*/
	ldxa		[%o0] ASI_PNF, %o3		/* Load				*/
	andcc		%o5, %g4, %g0			/* IEU1		Group		*/
#else
	ldxa		[%o0] ASI_PNF, %o3		/* Load				*/
	andcc		%o2, %g4, %g0			/* IEU1		Group		*/
#endif

	be,pt		%xcc, 2b			/* CTI				*/
	 add		%o0, 8, %o0			/* IEU0				*/
 	addcc		%o2, %g1, %g5			/* IEU1		Group		*/
#ifdef EIGHTBIT_NOT_RARE
	srlx		%o5, 32, %o5			/* IEU0				*/

3:	andcc		%o5, %g4, %g0			/* IEU1		Group		*/
#else
	srlx		%o2, 32, %o2			/* IEU0				*/

3:	andcc		%o2, %g4, %g0			/* IEU1		Group		*/
#endif
	be,pn		%xcc, 4f			/* CTI				*/
	 srlx		%g5, 56, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/

	be,pn		%icc, 12f			/* CTI				*/
	 srlx		%g5, 48, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 11f			/* CTI				*/

	 srlx		%g5, 40, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 10f			/* CTI				*/
	 srlx		%g5, 32, %o2			/* IEU0				*/

	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 9f			/* CTI				*/
4:	 srlx		%g5, 24, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/

	be,pn		%icc, 8f			/* CTI				*/
	 srlx		%g5, 16, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 7f			/* CTI				*/

	 srlx		%g5, 8, %o2			/* IEU0				*/
	andcc		%o2, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 6f			/* CTI				*/
	 sub		%o3, %g1, %o2			/* IEU0				*/

	andcc		%g5, 0xff, %g0			/* IEU1		Group		*/
	be,pn		%icc, 5f			/* CTI				*/
	 ldxa		[%o0] ASI_PNF, %o3		/* Load				*/
	andcc		%o2, %g4, %g0			/* IEU1		Group		*/

	be,pt		%xcc, 2b			/* CTI				*/
	 add		%o0, 8, %o0			/* IEU0				*/
	addcc		%o2, %g1, %g5			/* IEU1		Group		*/
	ba,pt		%xcc, 3b			/* CTI				*/

	 srlx		%o2, 32, %o2			/* IEU0				*/
5:	add		%o0, -9, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/

6:	add		%o0, -10, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/
7:	add		%o0, -11, %o0			/* IEU0		Group		*/

	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/
8:	add		%o0, -12, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/

	 sub		%o0, %o1, %o0			/* IEU0				*/
9:	add		%o0, -13, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/

10:	add		%o0, -14, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/
11:	add		%o0, -15, %o0			/* IEU0		Group		*/

	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/
12:	add		%o0, -16, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/

	 sub		%o0, %o1, %o0			/* IEU0				*/
13:	retl						/* CTI+IEU1	Group		*/
	 mov		0, %o0				/* IEU0				*/
	nop

15:	ldub		[%o0], %o3			/* Load		Group		*/
16:	andcc		%o0, 7, %g0			/* IEU1				*/
	be,pn		%icc, 1b			/* CTI				*/
	 nop						/* IEU0		Group		*/

	add		%o0, 1, %o0			/* IEU1				*/
	andcc		%o3, 0xff, %g0			/* IEU1		Group		*/
	bne,a,pt	%icc, 16b			/* CTI				*/
	 lduba		[%o0] ASI_PNF, %o3		/* Load				*/

	add		%o0, -1, %o0			/* IEU0		Group		*/
	retl						/* CTI+IEU1	Group		*/
	 sub		%o0, %o1, %o0			/* IEU0				*/
END(strlen)
libc_hidden_def(strlen)
