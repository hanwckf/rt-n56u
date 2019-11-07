/* BSD `setjmp' entry point to `sigsetjmp (..., 1)'.  ARC version.
 *
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

/* This just does a tail-call to `__sigsetjmp (ARG, 1)'.
   We cannot do it in C because it must be a tail-call, so frame-unwinding
   in setjmp doesn't clobber the state restored by longjmp.  */

#include <sysdep.h>

;@ r0 = jump buffer into which regs will be saved

ENTRY(setjmp)
	b.d	__sigsetjmp
	mov	r1, 1		; save signals
END(setjmp)
libc_hidden_def(setjmp)
