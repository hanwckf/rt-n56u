!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

	.text
	.global	_pthread_spin_unlock
	.type	_pthread_spin_unlock,function
_pthread_spin_unlock:
	MOV	D0Re0, #0
	SETD	[D1Ar1], D0Re0
	MOV	PC, D1RtP
        .size   _pthread_spin_unlock,.-_pthread_spin_unlock

        /* The implementation of pthread_spin_init is identical.  */
        .global  _pthread_spin_init
_pthread_spin_init = _pthread_spin_unlock
