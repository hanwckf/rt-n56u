/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>
#include <sysdep.h>

/* No legacy syscall ABI means NR_vfork is not available at all, use clone */
#define _SIGNAL_H
#include <bits/signum.h>       /* For SIGCHLD */

#define CLONE_VM		0x00000100
#define CLONE_VFORK		0x00004000
#define CLONE_FLAGS_FOR_VFORK	(CLONE_VM|CLONE_VFORK|SIGCHLD)

ENTRY(__vfork)
	mov	r0, CLONE_FLAGS_FOR_VFORK
	mov_s	r1, sp
	mov	r8, __NR_clone
	ARC_TRAP_INSN

	cmp	r0, 0
	jge	[blink]	; pid >=0 return, else detour via tailcall to errno

	b   __syscall_error
END(__vfork)

weak_alias(__vfork,vfork)
libc_hidden_def(vfork)
