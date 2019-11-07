!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

#include <asm/unistd.h>

#define _ERRNO_H
#include <bits/errno.h>
#include <sys/syscall.h>

#ifdef __NR_vfork
#define __VFORK_NR __NR_vfork
#else
#define __VFORK_NR __NR_fork
#endif

/* Clone the calling process, but without copying the whole address space.
   The calling process is suspended until the new process exits or is
   replaced by a call to `execve'.  Return -1 for errors, 0 to the new process,
   and the process ID of the new process to the old process.  */

	.balign 4
	.global	___vfork
	.hidden	___vfork
	.type	___vfork, @function
___vfork:

	MOV	D1Ar1, #0x4111	/* CLONE_VM | CLONE_VFORK | SIGCHLD */
	MOV	D0Ar2, #0
	MOV	D1Ar3, #0
	MOV	D0Ar4, #0
	MOV	D1Ar5, #0
	MOV	D0Ar6, #0
	MOV	D1Re0, #__NR_clone
	SWITCH  #0x440001

	MOVT	D1Re0, #HI(-4096)
	ADD	D1Re0, D1Re0, #LO(-4096)
	CMP	D1Re0, D0Re0
	BCS	error

	/* Syscall worked. Return to child/parent */
	MOV	PC, D1RtP

error:
	MOV	D1Ar1, D0Re0
#ifdef __PIC__
	B	___syscall_error@PLT
#else
	B	___syscall_error
#endif
	.size ___vfork,.-___vfork

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
