/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

#define _ERRNO_H 1
#include <bits/errno.h>

; int pthread_spin_trylock(pthread_spinlock_t *lock)
; Return EBUSY is already locked, 0 if not

ENTRY(pthread_spin_trylock)
	mov_s	r1, 1
	ex	r1, [r0]
	tst	r1, r1
	mov.z	r0, 0
	j.d	[blink]
	mov.nz	r0, EBUSY

;	slightly faster, but difficult to follow
;	mov_s	r1, r0
;	mov_s	r0, 1
;	ex	r0, [r1]
;	tst	r0, r0
;	j.d	[blink]
;	mov.nz	r0, EBUSY

END(pthread_spin_trylock)
