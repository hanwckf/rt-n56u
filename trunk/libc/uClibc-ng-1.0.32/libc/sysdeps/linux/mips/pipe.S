/* pipe system call for Linux/MIPS */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

/*see uClibc's sh/pipe.c and glibc-2.2.4's mips/pipe.S */

#include <features.h>
#include <sys/asm.h>
#include <asm/unistd.h>
#include <sys/regdef.h>

	.globl	pipe
	.ent	pipe, 0
	.type pipe,@function
pipe:
#ifdef __PIC__
	SETUP_GP
#endif
	li	v0,__NR_pipe
	syscall
	bnez	a3, 1f
	sw	v0, 0(a0)
	sw	v1, 4(a0)
	li	v0, 0
	j	ra
1:
	/* uClibc change -- start */
	move		a0,v0		/* Pass return val to C function. */
	/* uClibc change -- stop */

#ifdef __PIC__
	SETUP_GP64(v0, pipe)
	PTR_LA	t9, __syscall_error
	RESTORE_GP64
	jr	t9
#else
	j	__syscall_error
#endif
1:
	.end	pipe
	.size	pipe,.-pipe
libc_hidden_def(pipe)
