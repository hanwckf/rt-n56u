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

#include <features.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>


;----------------------------------------
;         Name: __longjmp
;  Description: Restore the current context
;               as saved by a previous nr_setjmp
;        Input: %o0: jmp_buf (ptr to) array to restore context from
;               %o1: integer to return
;       Output: %o0 = 0 the first time we're called, or
;               whatever longjmp returns later
; Side Effects: uses %g0, %g1 & %g2
;    CWP Depth: 0
;

	.align 2
	.global	__longjmp
__longjmp:
	;
	; The way we'll do this is by executing
	; RESTORE instructions until the old
	; return address matches. Then we'll
	; jump to where setjmp was called from.
	;
	; Since we're moving the window pointer
	; all over the place, we'll naturally
	; only use the %g registers.
	;

	mov	%g0,%o0		; %g0 -> jmp_buf
	mov	%g1,%o1		; %g1 = return value
	pfx	jmpbuf_callersret
	ld	%g2,[%g0]	; %g2 = old return address
__longjmp_loop:
	cmp	%g2,%i7		; Are we there yet?
	skps	cc_ne
	 br	__longjmp_done
	 nop			; (delay slot)

	br	__longjmp_loop
	restore			; (delay slot)
	;
	; One might put in a watchdog counter here, to
	; prevent a runaway stack crawl... but what would that
	; accomplish? What error can we throw? To whom?
	;

__longjmp_done:
	pfx	jmpbuf_l0	; Restore local register l0
	ld	%l0,[%g0]
	pfx	jmpbuf_l1	; Restore local register l1
	ld	%l1,[%g0]
	pfx	jmpbuf_l2	; Restore local register l2
	ld	%l2,[%g0]
	pfx	jmpbuf_l3	; Restore local register l3
	ld	%l3,[%g0]
	pfx	jmpbuf_l4	; Restore local register l4
	ld	%l4,[%g0]
	pfx	jmpbuf_l5	; Restore local register l5
	ld	%l5,[%g0]
	pfx	jmpbuf_l6	; Restore local register l6
	ld	%l6,[%g0]
	pfx	jmpbuf_l7	; Restore local register l7
	ld	%l7,[%g0]
	pfx	jmpbuf_i0	; Restore input register i0
	ld	%i0,[%g0]
	pfx	jmpbuf_i1	; Restore input register i1
	ld	%i1,[%g0]
	pfx	jmpbuf_i2	; Restore input register i2
	ld	%i2,[%g0]
	pfx	jmpbuf_i3	; Restore input register i3
	ld	%i3,[%g0]
	pfx	jmpbuf_i4	; Restore input register i4
	ld	%i4,[%g0]
	pfx	jmpbuf_i5	; Restore input register i5
	ld	%i5,[%g0]
	pfx	jmpbuf_jmpret
	ld	%o7,[%g0]	; set fake return address
	jmp	%o7		; and kinda return there.
	mov	%o0,%g1		; (delay slot) return value

libc_hidden_def(__longjmp)
