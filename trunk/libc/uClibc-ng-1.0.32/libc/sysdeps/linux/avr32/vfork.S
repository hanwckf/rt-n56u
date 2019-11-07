/*
 * Copyright (C) 2005 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */

/*
 * Clone the process without copying the address space.  The
 * calling process is suspended until the child either exits
 * or calls execve.
 *
 * This all means that we cannot rely on the stack to store
 * away registers, since they will be overwritten by the child
 * as soon as it makes another function call (e.g. execve()).
 * Fortunately, the Linux kernel preserves LR across system calls.
 */

#include <sys/syscall.h>

	.global	__vfork
	.hidden __vfork
	.type	__vfork,@function
	.align	1
__vfork:
	mov	r8, __NR_vfork
	scall
	cp.w	r12, -4096
	retls	r12

	/* vfork failed, so we may use the stack freely */
	pushm	r4-r7,lr
#ifdef __PIC__
	lddpc	r6, .L_GOT
	rsub	r4, r12, 0
.L_RGOT:
	rsub	r6, pc
	mcall	r6[__errno_location@got]
#else
	rsub	r4, r12, 0
	mcall	.L__errno_location
#endif
	st.w	r12[0], r4
	popm	r4-r7,pc,r12=-1

	.align	2
#ifdef __PIC__
.L_GOT:
	.long	.L_RGOT - _GLOBAL_OFFSET_TABLE_
#else
.L__errno_location:
	.long	__errno_location
#endif
	.size	__vfork, . - __vfork

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
