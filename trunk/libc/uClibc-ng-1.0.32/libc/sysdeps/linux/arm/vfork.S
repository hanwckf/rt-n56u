/*
 * vfork for uClibc
 * Copyright (C) 2000-2006 by Erik Andersen <andersen@uclibc.org>
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <features.h>
#include <bits/arm_asm.h>
#include <bits/arm_bx.h>

#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

#ifdef __NR_fork
.text
.global	__vfork
.hidden	__vfork
.type	__vfork,%function
.align 4

#if defined(__thumb__) && !defined(__thumb2__)
.thumb_func
__vfork:
#ifdef __NR_vfork
	DO_CALL (vfork)
	ldr		r1, =0xfffff000
	cmp		r0, r1
	bcs		1f
	bx		lr
1:

	/* Check if vfork even exists.  */
	ldr		r1, =-ENOSYS
	cmp		r0, r1
	bne		__error

	/* If we don't have vfork, use fork.  */
	DO_CALL (fork)
	ldr		r1, =0xfffff000
	cmp		r0, r1

	/* Syscall worked.  Return to child/parent */
	bcs		1f
	bx		lr
1:

__error:
	push	{r3, lr}
	bl	__syscall_error
	POP_RET
.pool

#endif

#else
__vfork:

#ifdef __NR_vfork
	DO_CALL (vfork)
	cmn	r0, #4096
	IT(t, cc)
	BXC(cc, lr)

	/* Check if vfork even exists.  */
	ldr     r1, =-ENOSYS
	teq     r0, r1
	bne     __error
#endif

	/* If we don't have vfork, use fork.  */
	DO_CALL (fork)
	cmn     r0, #4096

	/* Syscall worked.  Return to child/parent */
	IT(t, cc)
	BXC(cc, lr)

__error:
	b	__syscall_error
#endif

.size __vfork,.-__vfork

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
#endif
