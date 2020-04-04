/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>

ENTRY(__longjmp)
        ldw	sp, (a0, 0)
	ldw	lr, (a0, 4)

	ldw	l0, (a0, 8)
	ldw	l1, (a0, 12)
	ldw	l2, (a0, 16)
	ldw	l3, (a0, 20)
	ldw	l4, (a0, 24)
	ldw	l5, (a0, 28)

#ifdef __CSKYABIV2__
	ldw	l6, (a0, 32)
	ldw	l7, (a0, 36)
	ldw	l8, (a0, 40)
	ldw	l9, (a0, 44)

	ldw	r26, (a0, 48)
	ldw	r27, (a0, 52)
	ldw	gb,  (a0, 56)
	ldw	r29, (a0, 60)
	ldw	r30, (a0, 64)
	ldw	tls, (a0, 68)
#else
	ldw	gb, (a0, 32)
#endif

	mov    a0, a1
	cmpnei a1, 0
	bt     1f
	movi   a0, 1
1:
	rts

END(__longjmp)
libc_hidden_def(__longjmp)

