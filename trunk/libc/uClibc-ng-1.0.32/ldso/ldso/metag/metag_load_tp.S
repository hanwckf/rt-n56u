!    Copyright (C) 2013 Imagination Technologies Ltd.

!    Licensed under LGPL v2.1 or later, see the file COPYING.LIB in this tarball.

#include <features.h>

#ifdef __UCLIBC_HAS_THREADS_NATIVE__

#include <sysdep.h>

	.text
	.global ___metag_load_tp
	.type   ___metag_load_tp,@function

___metag_load_tp:
	MOVT	D1Ar1,#HI(0x6ffff000)
	JUMP	D1Ar1,#LO(0x6ffff000)
	.size   ___metag_load_tp,.-___metag_load_tp

#endif /* __UCLIBC_HAS_THREADS_NATIVE__ */
