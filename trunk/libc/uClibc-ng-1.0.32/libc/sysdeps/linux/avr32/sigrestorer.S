/*
 * Copyright (C) 2004 Atmel Corporation
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License.  See the file "COPYING.LIB" in the main directory of this
 * archive for more details.
 */
#include <sys/syscall.h>

	.global	__default_rt_sa_restorer
	.type	__default_rt_sa_restorer,"function"
	.align	1
__default_rt_sa_restorer:
	mov	r8, __NR_rt_sigreturn
	scall
