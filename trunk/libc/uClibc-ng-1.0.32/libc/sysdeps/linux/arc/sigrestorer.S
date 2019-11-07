/*
 * Copyright (C) 2017 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

/*
 * Default sigretrun stub if user doesn't specify SA_RESTORER.
 */

.section .text
.align 4
.global __default_rt_sa_restorer
.type __default_rt_sa_restorer, @function

; This "nop" is needed here, because debugger would assume that this function
; called the signal handler, therefore it cannot start on the "mov"
; instruction, at which execution will jump to.
	nop
__default_rt_sa_restorer:
	mov r8, __NR_rt_sigreturn
	ARC_TRAP_INSN

