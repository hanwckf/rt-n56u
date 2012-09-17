/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <sys/syscall.h>

#ifndef __NR_vfork
/* No vfork so use fork instead */
# define __NR_vfork __NR_fork
#endif

.text
.global __vfork
.hidden __vfork
.type   __vfork,%function

__vfork:
	popl %ecx

#ifdef SAVE_PID
	SAVE_PID
#endif

	movl $__NR_vfork,%eax
	int $0x80
	pushl %ecx

#ifdef RESTORE_PID
	RESTORE_PID
#endif

	cmpl $-4095,%eax
	jae __syscall_error
	ret

.size __vfork,.-__vfork

weak_alias(__vfork,vfork)
libc_hidden_weak(vfork)
