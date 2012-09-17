/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>

#include <asm/unistd.h>

#ifndef __NR_vfork
#define __NR_vfork __NR_fork /* uClinux-2.0 only has fork which is vfork */
#endif

#define IMM #

	.text
	.align 2
	.globl errno
	.globl	__vfork
	.hidden	__vfork
	.type	__vfork,@function

__vfork:
	movl	%sp@+, %a1               /* save the return address for later */
	movl	IMM __NR_vfork,%d0
	trap	#0
	movl	IMM -4097, %d1
	cmpl 	%d0, %d1
	bcs     fix_errno
	jmp		%a1@                     /* don't return,  just jmp directly */
fix_errno:
	negl	%d0
#ifndef __PIC__					/* needs handling as the other archs */
	movl	errno, %a0
#else
	movl	errno@GOT(%a5), %a0
#endif
	movl	%d0, %a0@
	movl	IMM -1, %d0
	jmp		%a1@                     /* don't return,  just jmp directly */

.size __vfork,.-__vfork
weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
