/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>
#include "NM_Macros.S"

;----------------------------------------
;         Name: __sigsetjmp
;  Description: Save the current context so
;               a nr_longjmp works later.
;        Input: %o0: jmp_buf: (ptr to) array to store context in
;       Output: %o0 = 0 the first time we're called, or
;               whatever longjmp returns later
; Side Effects: Uses %g0
;    CWP Depth: 0
;

	.align	2
	.global	_setjmp
	
_setjmp:
	br	__sigsetjmp
	 movi	%o1,0		; (Delay slot) Set signal mask to zero

	.align	2
	.global	setjmp
	
setjmp:
	MOVIP	%o1,1		; Set signal mask to 1 to save mask

	.align	2
	.global	__sigsetjmp

__sigsetjmp:
	pfx	jmpbuf_callersret ; present return address
	st	[%o0],%i7	
	pfx	jmpbuf_jmpret	; where the longjmp will later execute from
	st	[%o0],%o7	
	pfx	jmpbuf_sp	; Save stack pointer
	st	[%o0],%o6
	pfx	jmpbuf_l0	; Save local register l0
	st	[%o0],%l0
	pfx	jmpbuf_l1	; Save local register l1
	st	[%o0],%l1
	pfx	jmpbuf_l2	; Save local register l2
	st	[%o0],%l2
	pfx	jmpbuf_l3	; Save local register l3
	st	[%o0],%l3
	pfx	jmpbuf_l4	; Save local register l4
	st	[%o0],%l4
	pfx	jmpbuf_l5	; Save local register l5
	st	[%o0],%l5
	pfx	jmpbuf_l6	; Save local register l6
	st	[%o0],%l6
	pfx	jmpbuf_l7	; Save local register l7
	st	[%o0],%l7
	pfx	jmpbuf_i0	; Save input register i0
	st	[%o0],%i0
	pfx	jmpbuf_i1	; Save input register i1
	st	[%o0],%i1
	pfx	jmpbuf_i2	; Save input register i2
	st	[%o0],%i2
	pfx	jmpbuf_i3	; Save input register i3
	st	[%o0],%i3
	pfx	jmpbuf_i4	; Save input register i4
	st	[%o0],%i4
	pfx	jmpbuf_i5	; Save input register i5
	st	[%o0],%i5
	pfx	%hi(__sigjmp_save@h)	  ; Load up %g0 with address
	movi	%g0,%lo(__sigjmp_save@h)
	pfx	%xhi(__sigjmp_save@h)
	movhi	%g0,%xlo(__sigjmp_save@h)
	jmp	%g0
	 nop			; (delay slot)










