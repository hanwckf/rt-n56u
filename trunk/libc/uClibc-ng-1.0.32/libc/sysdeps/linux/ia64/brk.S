/* brk system call for Linux/ia64
   Copyright (C) 1999,2000,2001,2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Stephane Eranian <eranian@hpl.hp.com> and
	      Jes Sorensen, <Jes.Sorensen@cern.ch>, April 1999.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "sysdep.h"

#include <asm/unistd.h>
#include <asm/errno.h>

	.global __curbrk
	.hidden __curbrk
	.type __curbrk,@object
	.size __curbrk,8
	.data
	.align	8
__curbrk:
	data8	0

weak_alias (__curbrk, ___brk_addr)

LEAF(brk)
	.regstk 1, 0, 0, 0
	DO_CALL(__NR_brk)
	cmp.ltu	p6, p0 = ret0, in0
	addl r9 = @ltoff(__curbrk), gp
	;;
	ld8 r9 = [r9]
(p6) 	mov ret0 = ENOMEM
(p6)	br.cond.spnt.few __syscall_error
	;;
	st8 [r9] = ret0
	mov ret0 = 0
	ret
END(brk)
libc_hidden_def(brk)
