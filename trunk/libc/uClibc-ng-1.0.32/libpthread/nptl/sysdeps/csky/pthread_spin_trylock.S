/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#define _ERRNO_H 1
#include <bits/errno.h>
#include <sysdep.h>

.text
ENTRY (pthread_spin_trylock)
	mov	a2, a0
	movi	a0, 0
	movi	a1, 1
	trap	2		// trap 2 use to cmpxchg
	cmpnei	a0, 0
	bf	1f
	movi	a0, EBUSY
1:
	jmp	r15
END (pthread_spin_trylock)
