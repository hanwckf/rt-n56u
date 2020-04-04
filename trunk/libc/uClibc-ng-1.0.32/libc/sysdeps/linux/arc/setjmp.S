/*
 * Copyright (C) 2013 Synopsys, Inc. (www.synopsys.com)
 *
 * Licensed under the LGPL v2.1 or later, see the file COPYING.LIB in this tarball.
 */

#include <sysdep.h>

;@ r0 = jump buffer into which regs will be saved
;@ r1 = do we need to save signals

ENTRY(__sigsetjmp)

	st_s r13, [r0]
	st_s r14, [r0,4]
	st   r15, [r0,8]
	st   r16, [r0,12]
	st   r17, [r0,16]
	st   r18, [r0,20]
	st   r19, [r0,24]
	st   r20, [r0,28]
	st   r21, [r0,32]
	st   r22, [r0,36]
	st   r23, [r0,40]
	st   r24, [r0,44]
	st   r25, [r0,48]
	st   fp,  [r0,52]
	st   sp,  [r0,56]

	; make a note of where longjmp will return to.
	; that will be right next to this setjmp call-site which will be
	; contained in blink, since "C" caller of this routine will do
	; a branch-n-link

	st   blink, [r0,60]
	b    __sigjmp_save

END(__sigsetjmp)
libc_hidden_def(__sigsetjmp)
