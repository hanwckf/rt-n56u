/*
 * Copyright (C) 2013 Imagination Technologies Ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <errno.h>
#include <unistd.h>
#include <sys/syscall.h>

libc_hidden_proto(brk)

/* This must be initialized data because commons can't have aliases.  */
void * __curbrk attribute_hidden = 0;

int brk (void *addr)
{
    void *newbrk;

    __asm__ __volatile__ ("MOV D1Re0,%2\n\t"
			  "MOV D1Ar1,%1\n\t"
			  "SWITCH #0x440001\n\t"
			  "MOV %0,D0Re0"
			  : "=r" (newbrk)
			  : "r" (addr), "K" (__NR_brk)
			  : "D0Re0", "D1Re0", "D1Ar1");

    __curbrk = newbrk;

    if (newbrk < addr)
    {
	__set_errno (ENOMEM);
	return -1;
    }

    return 0;
}
libc_hidden_def(brk)
