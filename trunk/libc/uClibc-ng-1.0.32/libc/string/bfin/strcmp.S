/* strcmp.S
 * Copyright (C) 2003-2007 Analog Devices Inc., All Rights Reserved.
 *
 * This file is subject to the terms and conditions of the GNU Library General
 * Public License. See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 *
 * Non-LGPL License also available as part of VisualDSP++
 * http://www.analog.com/processors/resources/crosscore/visualDspDevSoftware.html
 */

#include <sysdep.h>

/* Fast strcmp() for Blackfin.
 * When both strings are aligned, this processes four characters at
 * a time. Uses a hw loop with "very big" count to loop "forever",
 * until difference or a terminating zero is found.
 * Once the end-case word has been identified, breaks out of the
 * loop to check more carefully (same as the unaligned case).
 */

.text

.align 2

.weak _strcmp
ENTRY(_strcmp)
	[--sp] = (R7:4);
	p1 = r0;
	p2 = r1;

	p0 = -1;	/* (need for loop counter init) */

	  /* check if byte aligned */
	r0 = r0 | r1;	/* check both pointers at same time */
	r0 <<= 30;	/* dump all but last 2 bits */
	cc = az;	/* are they zero? */
	if !cc jump .Lunaligned;	/* no; use unaligned code. */
			/* fall-thru for aligned case.. */

	  /* note that r0 is zero from the previous... */
	  /*           p0 set to -1 */

	LSETUP (.Lbeginloop, .Lendloop) lc0=p0;
	  /* pick up first words */
	r1 = [p1++];
	r2 = [p2++];
	  /* make up mask:  0FF0FF */
	r7 = 0xFF;
	r7.h = 0xFF;
		/* loop : 9 cycles to check 4 characters */
	cc = r1 == r2;
.Lbeginloop:
	if !cc jump .Lnotequal4;	/* compare failure, exit loop */

	  /* starting with   44332211 */
	  /* see if char 3 or char 1 is 0 */
	r3 = r1 & r7;		/* form 00330011 */
	  /* add to zero, and (r2 is free, reload) */
	r6 = r3 +|+ r0 || r2 = [p2++] || nop;
	cc = az;	/* true if either is zero */
	r3 = r1 ^ r3;	        /* form 44002200 (4321^0301 => 4020) */
				/* (trick, saves having another mask) */
	/* add to zero,  and  (r1 is free, reload) */
	r6 = r3 +|+ r0 || r1 = [p1++] || nop;
	cc |= az;	/* true if either is zero */
	if cc jump .Lzero4;	/* leave if a zero somewhere */
.Lendloop:
	cc = r1 == r2;

 /* loop exits */
.Lnotequal4:		/* compare failure on 4-char compare */
			/* address pointers are one word ahead; */
			/* faster to use zero4 exit code */
	p1 += 4;
	p2 += 4;

.Lzero4:			/* one of the bytes in word 1 is zero */
			/* but we've already fetched the next word; so */
			/* backup two to look at failing word again */
	p1 += -8;
	p2 += -8;



		/* here when pointers are unaligned: checks one */
		/* character at a time.  Also use at the end of */
		/* the word-check algorithm to figure out what happened */
.Lunaligned:
	  /*	R0 is non-zero from before. */
	  /*           p0 set to -1 */

	r0 = 0 (Z);
	r1 = B[p1++] (Z);
	r2 = B[p2++] (Z);
	LSETUP (.Lbeginloop1, .Lendloop1) lc0=p0;

.Lbeginloop1:
	cc = r1;	/* first char must be non-zero */
	/* chars must be the same */
	r3 = r2 - r1 (NS) || r1 = B[p1++] (Z) || nop;
	cc &= az;
	r3 = r0 - r2;	/* second char must be non-zero */
	cc &= an;
	if !cc jump .Lexitloop1;
.Lendloop1:
	r2 = B[p2++] (Z);

.Lexitloop1: /* here means we found a zero or a difference. */
	   /* we have r2(N), p2(N), r1(N+1), p1(N+2) */
	r1=B[p1+ -2] (Z);
	r0 = r1 - r2;
	(r7:4) = [sp++];
	rts;
.size _strcmp,.-_strcmp

libc_hidden_def (strcmp)

#ifndef __UCLIBC_HAS_LOCALE__
weak_alias (strcmp,strcoll)
libc_hidden_def (strcoll)
#endif
