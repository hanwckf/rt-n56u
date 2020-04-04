/* consider this code LGPL - davidm */
/*
 * Copyright (C) 2000-2006 Erik Andersen <andersen@uclibc.org>
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */

#include <unistd.h>
#include <sys/syscall.h>
#include <errno.h>


/* This must be initialized data because commons can't have aliases.  */
void * __curbrk = 0;

int brk (void *addr)
{
    void *newbrk;

	__asm__ __volatile__ ("movel %2,%/d1\n\t"
			  "moveq %1,%/d0\n\t"
			  "trap  #0\n\t"
			  "movel %/d0,%0"
		:"=g" (newbrk)
		:"i" (__NR_brk),"g" (addr) : "%d0", "%d1");

    __curbrk = newbrk;

    if (newbrk < addr)
    {
	__set_errno (ENOMEM);
	return -1;
    }

    return 0;
}
libc_hidden_def(brk)
