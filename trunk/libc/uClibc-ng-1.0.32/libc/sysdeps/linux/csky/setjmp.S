/*
 * Copyright (C) 2017 Hangzhou C-SKY Microsystems co.,ltd.
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB
 * in this tarball.
 */

#include <sysdep.h>

ENTRY(__sigsetjmp)
	stw     sp, (a0, 0)
	stw     lr, (a0, 4)

	stw     l0, (a0, 8)
	stw     l1, (a0, 12)
	stw     l2, (a0, 16)
	stw     l3, (a0, 20)
	stw     l4, (a0, 24)
	stw     l5, (a0, 28)

#ifdef __CSKYABIV2__
	stw	l6, (a0, 32)
        stw	l7, (a0, 36)
        stw	l8, (a0, 40)
        stw	l9, (a0, 44)

        stw	r26, (a0, 48)
        stw	r27, (a0, 52)
        stw	gb,  (a0, 56)
        stw	r29, (a0, 60)
        stw	r30, (a0, 64)
        stw	tls, (a0, 68)
#else
	stw     gb, (a0, 32)
#endif

	subi	sp, 8
	stw	lr, (sp, 0)
	stw     gb, (sp, 4)
	__GET_GB
	__JSR(__sigjmp_save)
	ldw     gb, (sp, 4)
	ldw	lr, (sp, 0)
	addi	sp, 8
	rts
END(__sigsetjmp)

/* 
 * Support bsd-setjmp and bsd-_setjmp with tail-call method.
 * Use br to keep the lr-reg, we must return to the Caller not setjmp.
 * And we will rts by __sigsetjmp.
 */
ENTRY(setjmp)
	movi	a1, 1
	br	__sigsetjmp
END(setjmp)

ENTRY(_setjmp)
	movi	a1, 0
	br	__sigsetjmp
END(_setjmp)

