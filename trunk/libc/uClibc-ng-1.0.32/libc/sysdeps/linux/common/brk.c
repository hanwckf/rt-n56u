/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

#define __NR___syscall_brk __NR_brk
static __always_inline _syscall1(void *, __syscall_brk, void *, end)

/* This must be initialized data because commons can't have aliases.  */
void * __curbrk attribute_hidden = 0;

int brk(void *addr)
{
	void *newbrk = __syscall_brk(addr);

	__curbrk = newbrk;

	if (newbrk < addr) {
		__set_errno (ENOMEM);
		return -1;
	}

	return 0;
}
libc_hidden_def(brk)
