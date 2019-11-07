/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

; int pthread_spin_lock(pthread_spinlock_t *lock)
; 1 - locked, 0 - unlocked

ENTRY(pthread_spin_lock)
	mov_s	r1, 1
	nop_s
1:
	ex	r1, [r0]
	breq	r1, 1, 1b

	j.d	[blink]
	mov	r0, 0
END(pthread_spin_lock)
