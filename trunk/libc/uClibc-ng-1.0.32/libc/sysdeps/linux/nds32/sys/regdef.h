/*
 * Copyright (C) 2016 Andes Technology, Inc.
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/* Copyright (C) 1997, 1998, 2002, 2003, 2004 Free Software Foundation, Inc.
   Contributed by Ralf Baechle <ralf@gnu.org>.

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

#ifndef _SYS_REGDEF_H
#define _SYS_REGDEF_H

/*
 * Symbolic register names for 32 bit ABI
 */
#define o0	r0	/* arguments r0 ~ r5 */
#define o1	r1
#define o2	r2
#define o3	r3
#define o4	r4
#define o5	r5
#define o6	r6
#define o7	r7

#define h0	r0	/* arguments r0 ~ r5 */
#define h1	r1
#define h2	r2
#define h3	r3
#define h4	r4
#define h5	r5
#define h6	r6
#define h7	r7
#define h8	r8
#define h9	r9
#define h10	r10
#define h11	r11
#define h12	r16
#define h13	r17
#define h14	r18
#define h15	r19

#define a0	r0	/* arguments r0 ~ r5 */
#define a1	r1
#define a2	r2
#define a3	r3
#define a4	r4
#define a5	r5
#define s0	r6
#define s1	r7
#define s2	r8
#define s3	r9
#define s4	r10
#define s5	r11
#define s6	r12
#define s7	r13
#define s8	r14
#define ta	r15
#define t0	r16
#define t1	r17
#define t2	r18
#define t3	r19
#define t4	r20
#define t5	r21
#define t6	r22
#define t7	r23
#define t8	r24
#define t9	r25
#define p0	r26
#define p1	r27
#define r28	fp
#define s9	r28
#define r29	gp
#define r30	ra
#define r31	sp

#endif /* _SYS_REGDEF_H */
