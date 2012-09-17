/* memcpy.S: optimised assembly memcpy
 *
 * Copyright (C) 2003, 2004 Red Hat, Inc. All Rights Reserved.
 * Written by David Howells (dhowells@redhat.com)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


        .text
        .p2align	4

###############################################################################
#
# void *memcpy(void *to, const char *from, size_t count)
#
# - NOTE: must not use any stack. exception detection performs function return
#         to caller's fixup routine, aborting the remainder of the copy
#
###############################################################################
        .globl		memcpy
        .type		memcpy,@function
memcpy:
	or.p		gr8,gr9,gr4
	orcc		gr10,gr0,gr0,icc3
	or.p		gr10,gr4,gr4
	beqlr		icc3,#0

	# optimise based on best common alignment for to, from & count
	andicc.p	gr4,#0x1f,gr0,icc0
	setlos		#8,gr11
	andicc.p	gr4,#0x0f,gr0,icc1
	beq		icc0,#0,memcpy_32
	andicc.p	gr4,#0x07,gr0,icc0
	beq		icc1,#0,memcpy_16
	andicc.p	gr4,#0x03,gr0,icc1
	beq		icc0,#0,memcpy_8
	andicc.p	gr4,#0x01,gr0,icc0
	beq		icc1,#0,memcpy_4
	setlos.p	#1,gr11
	beq		icc0,#0,memcpy_2

	# do byte by byte copy
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	ldubu.p		@(gr9,gr11),gr4
	subicc		gr10,#1,gr10,icc0
	stbu.p		gr4,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	# do halfword by halfword copy
memcpy_2:
	setlos		#2,gr11
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	lduhu.p		@(gr9,gr11),gr4
	subicc		gr10,#2,gr10,icc0
	sthu.p		gr4,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	# do word by word copy
memcpy_4:
	setlos		#4,gr11
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	ldu.p		@(gr9,gr11),gr4
	subicc		gr10,#4,gr10,icc0
	stu.p		gr4,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	# do double-word by double-word copy
memcpy_8:
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	lddu.p		@(gr9,gr11),gr4
	subicc		gr10,#8,gr10,icc0
	stdu.p		gr4,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	# do quad-word by quad-word copy
memcpy_16:
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	lddu		@(gr9,gr11),gr4
	lddu.p		@(gr9,gr11),gr6
	subicc		gr10,#16,gr10,icc0
	stdu		gr4,@(gr3,gr11)
	stdu.p		gr6,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	# do eight-word by eight-word copy
memcpy_32:
	sub.p		gr8,gr11,gr3
	sub		gr9,gr11,gr9
0:	lddu		@(gr9,gr11),gr4
	lddu		@(gr9,gr11),gr6
	lddu		@(gr9,gr11),gr12
	lddu.p		@(gr9,gr11),gr14
	subicc		gr10,#32,gr10,icc0
	stdu		gr4,@(gr3,gr11)
	stdu		gr6,@(gr3,gr11)
	stdu		gr12,@(gr3,gr11)
	stdu.p		gr14,@(gr3,gr11)
	bne		icc0,#2,0b
	bralr

	.size		memcpy, .-memcpy
