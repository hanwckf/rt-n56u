!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

	.text
	.global	_pthread_spin_lock
	.type	_pthread_spin_lock,function
_pthread_spin_lock:
1:	LNKGETD	D0Ar2, [D1Ar1]
	CMP	D0Ar2, #0
	ADD	D0Ar2, D0Ar2, #1
	LNKSETDEQ [D1Ar1], D0Ar2
	BNE	1b
	DEFR	D0Ar2, TXSTAT
	ANDT	D0Ar2, D0Ar2, #HI(0x3f000000)
	CMPT	D0Ar2, #HI(0x02000000)
	BNZ	1b
	MOV	D0Re0, #0
	MOV	PC, D1RtP
        .size   _pthread_spin_lock,.-_pthread_spin_lock
