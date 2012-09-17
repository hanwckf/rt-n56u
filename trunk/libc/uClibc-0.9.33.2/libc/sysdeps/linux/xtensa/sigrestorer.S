/*
 * Copyright (C) 2008 Tensilica Inc.
 *
 * Licensed under the LGPL v2.1, see the file COPYING.LIB in this tarball.
 */ 

#include <sys/syscall.h>

#if __NR_rt_sigreturn > 255
# error value of __NR_rt_sigreturn is too big!
#endif

	.text
	.align	4
	.global	__default_sa_restorer
	.type	__default_sa_restorer, @function
__default_sa_restorer:
	movi	a2, __NR_rt_sigreturn
	syscall
