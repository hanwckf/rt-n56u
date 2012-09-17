/*
 * Copyright (C) 2004-2007 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>


void *__curbrk attribute_hidden = 0;

int brk (void *addr)
{
	void *newbrk;

	newbrk = (void *)INLINE_SYSCALL(brk, 1, addr);

	__curbrk = newbrk;

	if (newbrk < addr) {
		__set_errno (ENOMEM);
		return -1;
	}

	return 0;
}
libc_hidden_def(brk)
