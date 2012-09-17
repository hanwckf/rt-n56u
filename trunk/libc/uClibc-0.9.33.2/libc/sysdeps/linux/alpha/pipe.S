/* Copyright (C) 1993, 1995, 1997, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger (davidm@cs.arizona.edu).

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

#include <features.h>
#include <sys/syscall.h>
#include <sys/regdef.h>
#include <asm/pal.h>

/* __pipe is a special syscall since it returns two values.  */

.globl pipe
.align 4
.ent pipe, 0
pipe:
	.frame sp, 0, ra
	ldgp gp,0(pv)
	.prologue 1
	lda v0, __NR_pipe
	call_pal PAL_callsys
	bne a3, $syscall_error

	stl	r0, 0(a0)
	stl	r1, 4(a0)
	mov	zero, v0
	ret

$syscall_error:
	jmp	zero,__syscall_error

.end pipe

libc_hidden_def (pipe)
