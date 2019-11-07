/*
 * exit syscall for uClibc
 *
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <bits/kernel-features.h>

#ifdef __UCLIBC_ABORT_INSTRUCTION__
# define ABORT_INSTRUCTION __asm__(__UCLIBC_ABORT_INSTRUCTION__)
#else
# warning "no abort instruction defined for this arch"
#endif

/* have to check for kernel 2.5.35 too, since NR was earlier present */
#if defined __NR_exit_group && __LINUX_KERNEL_VERSION >= 0x020600 \
	&& defined __UCLIBC_HAS_THREADS__
# undef __NR_exit
# define __NR_exit __NR_exit_group
#endif

void _exit(int status)
{
	/* The loop is added only to keep gcc happy. */
	while(1)
	{
		INLINE_SYSCALL(exit, 1, status);
#ifdef ABORT_INSTRUCTION
		ABORT_INSTRUCTION;
#endif
	}
}
libc_hidden_def(_exit)
#ifdef __USE_ISOC99
weak_alias(_exit,_Exit)
#endif
