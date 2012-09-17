/* Copyright (C) 2003, 2004 Free Software Foudnation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Howells <dhowells@redhat.com>, 2003.

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

#include <asm/unistd.h>
#define _ERRNO_H       1
#include <bits/errno.h>

       .text
       .globl	__vfork
       .hidden	__vfork
       .type	__vfork,@function
/* int vfork(void) */
__vfork:
	setlos.p #__NR_vfork, gr7
	setlos	#-4096, gr4
	tra	gr0, gr0

	cmp	gr8, gr4, icc0
	blslr	icc0, #2

	sethi.p	#gotofffuncdeschi(__syscall_error), gr14
	setlo	#gotofffuncdesclo(__syscall_error), gr14
	ldd	@(gr14, gr15), gr14
	jmpl	@(gr14, gr0)

       .size   __vfork,.-__vfork

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
